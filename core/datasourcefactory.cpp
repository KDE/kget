/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "datasourcefactory.h"
#include "bitset.h"
#include "settings.h"

#include "core/filedeleter.h"
#include "core/kget.h"
#include "core/signature.h"
#include "core/verifier.h"

#include <cmath>

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QVarLengthArray>
#include <QtXml/QDomText>

#include <KIO/FileJob>
#include <KLocale>
#include <KMessageBox>
#include <kmountpoint.h>

#include <KDebug>


#include <kde_file.h>

const int SPEEDTIMER = 1000;//1 second...

DataSourceFactory::DataSourceFactory(QObject *parent, const KUrl &dest, KIO::filesize_t size, KIO::fileoffset_t segSize)
  : QObject(parent),
    m_capabilities(0),
    m_dest(dest),
    m_size(size),
    m_downloadedSize(0),
    m_segSize(segSize),
    m_speed(0),
    m_percent(0),
    m_tempOffset(0),
    m_startedChunks(0),
    m_finishedChunks(0),
    m_putJob(0),
    m_doDownload(true),
    m_open(false),
    m_blocked(false),
    m_startTried(false),
    m_findFilesizeTried(false),
    m_assignTried(false),
    m_movingFile(false),
    m_finished(false),
    m_downloadInitialized(false),
    m_sizeInitiallyDefined(m_size),
    m_sizeFoundOnFinish(false),
    m_maxMirrorsUsed(3),
    m_speedTimer(0),
    m_status(Job::Stopped),
    m_statusBeforeMove(m_status),
    m_verifier(0),
    m_signature(0)
{
    kDebug(5001) << "Initialize DataSourceFactory: Dest: " + m_dest.url() + "Size: " + QString::number(m_size) + "SegSize: " + QString::number(m_segSize);

    m_prevDownloadedSizes.append(0);
}

DataSourceFactory::~DataSourceFactory()
{
    killPutJob();
    delete m_startedChunks;
    delete m_finishedChunks;
}

void DataSourceFactory::init()
{

    if (!m_doDownload)
    {
        return;
    }

    if (!m_speedTimer)
    {
        m_speedTimer = new QTimer(this);
        m_speedTimer->setInterval(SPEEDTIMER);
        connect(m_speedTimer, SIGNAL(timeout()), this, SLOT(speedChanged()));
    }

    if (m_segSize && m_size)
    {
        const int hasRemainder = (m_size % m_segSize == 0) ? 0 : 1;
        const int bitSetSize = (m_size / m_segSize) + hasRemainder;//round up if needed
        if (!m_startedChunks && bitSetSize)
        {
            m_startedChunks = new BitSet(bitSetSize);
        }
        if (!m_finishedChunks && bitSetSize)
        {
            m_finishedChunks = new BitSet(bitSetSize);
        }
    }
}

void DataSourceFactory::deinit()
{
    if (m_downloadInitialized && QFile::exists(m_dest.toLocalFile())) {
        FileDeleter::deleteFile(m_dest);
    }
}

void DataSourceFactory::findFileSize()
{
    kDebug(5001) << "Find the filesize" << this;
    if (!m_size && !m_dest.isEmpty() && !m_sources.isEmpty()) {
        foreach (TransferDataSource *source, m_sources) {
            if (source->capabilities() & Transfer::Cap_FindFilesize) {
                connect(source, SIGNAL(foundFileSize(TransferDataSource*,KIO::filesize_t,QPair<int,int>)), this, SLOT(slotFoundFileSize(TransferDataSource*,KIO::filesize_t,QPair<int,int>)));
                connect(source, SIGNAL(finishedDownload(TransferDataSource*,KIO::filesize_t)), this, SLOT(slotFinishedDownload(TransferDataSource*,KIO::filesize_t)));

                m_speedTimer->start();
                source->findFileSize(m_segSize);
                changeStatus(Job::Running);
                slotUpdateCapabilities();
                return;
            }
        }
    }
}

void DataSourceFactory::slotFoundFileSize(TransferDataSource *source, KIO::filesize_t fileSize, const QPair<int, int> &segmentRange)
{
    m_size = fileSize;
    kDebug(5001) << source << "found size" << m_size << "and is assigned segments" << segmentRange << this;
    emit dataSourceFactoryChange(Transfer::Tc_TotalSize);

    init();

    if ((segmentRange.first != -1) && (segmentRange.second != -1)) {
        m_startedChunks->setRange(segmentRange.first, segmentRange.second, true);
    }

    if (m_startTried) {
        start();
    }
}

void DataSourceFactory::slotFinishedDownload(TransferDataSource *source, KIO::filesize_t size)
{
    Q_UNUSED(source)
    Q_UNUSED(size)

    m_speedTimer->stop();
    m_finished = true;
}
//FIXME is this even needed
bool DataSourceFactory::checkLocalFile()
{
    QString dest_orig = m_dest.toLocalFile();
    QString _dest_part(dest_orig);

    KDE_struct_stat buff_part;
    bool bPartExists = (KDE::stat( _dest_part, &buff_part ) != -1);
    if(!bPartExists)
    {
        QString _dest = dest_orig;
        int fd = -1;
        mode_t initialMode = 0666;

        fd = KDE::open(_dest, O_CREAT | O_TRUNC | O_WRONLY, initialMode);
        if ( fd < 0 )
        {
            kDebug(5001) << " error";
            return false;
        }
        else
        {
            close(fd);
        }
    }

    kDebug(5001) << "success";
    return true;
}

void DataSourceFactory::start()
{
    kDebug(5001) << "Start DataSourceFactory";
    if (m_movingFile || (m_status == Job::Finished))
    {
        return;
    }
    if (!m_doDownload)
    {
        m_startTried = true;
        return;
    }

    //the file already exists, even though DataSourceFactory has not been initialized remove it
    //to avoid problems like over methods not finished removing it because of a redownload
    if (!m_downloadInitialized && QFile::exists(m_dest.toLocalFile())) {
        kDebug(5001) << "Removing existing file.";
        m_startTried = true;
        FileDeleter::deleteFile(m_dest, this, SLOT(slotRemovedFile()));
        return;
    }

    m_downloadInitialized = true;

    //create all dirs needed
    QDir dir;
    dir.mkpath(m_dest.directory());
    if (checkLocalFile()) {
        if (!m_putJob) {
            m_putJob = KIO::open(m_dest, QIODevice::WriteOnly | QIODevice::ReadOnly);
            connect(m_putJob, SIGNAL(open(KIO::Job*)), this, SLOT(open(KIO::Job*)));
            connect(m_putJob, SIGNAL(destroyed(QObject*)), this, SLOT(slotPutJobDestroyed(QObject*)));
            m_startTried = true;
            return;
        }
    } else {
        //could not create file, maybe device not mounted so abort
        m_startTried = true;
        changeStatus(Job::Aborted);
        return;
    }

    init();

    if (!m_open) {
        m_startTried = true;
        return;
    }

    if (!m_size) {
        if (!m_findFilesizeTried && m_sources.count()) {
            m_findFilesizeTried = true;
            findFileSize();
        }
        m_startTried = true;
        return;
    }

    if (assignNeeded()) {
        if (m_sources.count()) {
            kDebug(5001) << "Assigning a TransferDataSource.";
            //simply assign a TransferDataSource
            assignSegments(*m_sources.begin());
        } else if (m_unusedUrls.count()) {
            kDebug(5001) << "Assigning an unused mirror";
            //takes the first unused mirror
            addMirror(m_unusedUrls.takeFirst(), true, m_unusedConnections.takeFirst());
        }
    }

    if (m_assignTried) {
        m_assignTried = false;

        foreach(TransferDataSource *source, m_sources) {
            assignSegments(source);
        }
    }

    if (m_open) {
        //check if the filesystem supports a file of m_size
        const static KIO::filesize_t maxFatSize = 4294967295;
        if (m_size > maxFatSize) {
            KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByPath(m_dest.directory());
            if (!mountPoint.isNull()) {
                if (mountPoint->mountType() == "vfat") {//TODO check what is reported on Windows for vfat
                    stop();
                    KMessageBox::error(0, i18n("Filesize is larger than maximum file size supported by VFAT."), i18n("Error"));
                    return;
                }
            }
        }

        QFile::resize(m_dest.pathOrUrl(), m_size);//TODO should we keep that?
        m_speedTimer->start();

        foreach (TransferDataSource *source, m_sources) {
            source->start();
        }

        m_startTried = false;
        changeStatus(Job::Running);
    }
    slotUpdateCapabilities();
}

void DataSourceFactory::slotRemovedFile()
{
    kDebug(5001) << "File has been removed" << this;
    if (m_startTried) {
        m_startTried = false;
        start();
    }
}

void DataSourceFactory::open(KIO::Job *job)
{
    Q_UNUSED(job)
    kDebug(5001) << "File opened" << this;

    if (!m_speedTimer)
    {
        init();
    }

    connect(m_putJob, SIGNAL(position(KIO::Job*,KIO::filesize_t)), this, SLOT(slotOffset(KIO::Job*,KIO::filesize_t)));
    connect(m_putJob, SIGNAL(written(KIO::Job*,KIO::filesize_t)), this, SLOT(slotDataWritten(KIO::Job*,KIO::filesize_t)));
    m_open = true;

    if (m_startTried)
    {
        start();
    }
}

void DataSourceFactory::stop()
{
    kDebug(5001) << "Stopping" << this;
    if (m_movingFile || (m_status == Job::Finished))
    {
        return;
    }

    if (m_speedTimer)
    {
        m_speedTimer->stop();
    }

    foreach (TransferDataSource *source, m_sources) {
        source->stop();
    }
    m_startTried = false;
    m_findFilesizeTried = false;
    changeStatus(Job::Stopped);

    slotUpdateCapabilities();
}

void DataSourceFactory::setDoDownload(bool doDownload)
{
    if (m_doDownload == doDownload) {
        return;
    }

    m_doDownload = doDownload;
    if (m_doDownload)
    {
        if (m_startTried)
        {
            start();
        }
    }
    else
    {
        if (m_status == Job::Running)
        {
            stop();
        }
    }
}

void DataSourceFactory::addMirror(const KUrl &url, int numParalellConnections)
{
    addMirror(url, true, numParalellConnections, false);
}

void DataSourceFactory::addMirror(const KUrl &url, bool used, int numParalellConnections)
{
    addMirror(url, used, numParalellConnections, true);
}

void DataSourceFactory::addMirror(const KUrl &url, bool used, int numParalellConnections, bool usedDefined)
{
    if (!url.isValid() || url.isEmpty())
    {
        kDebug(5001) << "Url is not useable: " << url.pathOrUrl();
        return;
    }
    if (numParalellConnections <= 0)
    {
        numParalellConnections = 1;
    }
    if (!usedDefined)
    {
        used = true;
    }

    if (used)
    {
        //there is already a TransferDataSource with that url, reuse it and modify numParalellSegments
        if (m_sources.contains(url))
        {
            TransferDataSource *source = m_sources[url];
            source->setParalellSegments(numParalellConnections);
            if (source->changeNeeded() > 0) {
                assignSegments(source);
            } else {
                for (int i = source->changeNeeded(); i < 0; ++i)
                {
                    const QPair<int, int> removed = source->removeConnection();
                    kDebug(5001) << "Remove connection with segments" << removed;
                    const int start = removed.first;
                    const int end = removed.second;
                    if ((start != -1) && (end != -1)) {
                        m_startedChunks->setRange(start, end, false);
                    }
                }
            }
        }
        else
        {
            if (m_sources.count() < m_maxMirrorsUsed)
            {
                TransferDataSource *source = KGet::createTransferDataSource(url, QDomElement(), this);
                if (source)
                {
                    kDebug(5001) << "Successfully created a TransferDataSource for " << url.pathOrUrl() << this;

                    //url might have been an unused Mirror, so remove it in any case
                    const int index = m_unusedUrls.indexOf(url);
                    if (index > -1 )
                    {
                        m_unusedUrls.removeAt(index);
                        m_unusedConnections.removeAt(index);
                    }

                    m_sources[url] = source;
                    m_sources[url]->setParalellSegments(numParalellConnections);
                    if (m_sizeInitiallyDefined) {
                        source->setSupposedSize(m_size);
                    }

                    connect(source, SIGNAL(capabilitiesChanged()), this, SLOT(slotUpdateCapabilities()));
                    connect(source, SIGNAL(brokenSegments(TransferDataSource*,QPair<int,int>)), this, SLOT(brokenSegments(TransferDataSource*,QPair<int,int>)));
                    connect(source, SIGNAL(broken(TransferDataSource*,TransferDataSource::Error)), this, SLOT(broken(TransferDataSource*,TransferDataSource::Error)));
                    connect(source, SIGNAL(finishedSegment(TransferDataSource*,int,bool)), this, SLOT(finishedSegment(TransferDataSource*,int,bool)));
                    connect(source, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)), this, SLOT(slotWriteData(KIO::fileoffset_t,QByteArray,bool&)));
                    connect(source, SIGNAL(freeSegments(TransferDataSource*,QPair<int,int>)), this, SLOT(slotFreeSegments(TransferDataSource*,QPair<int,int>)));
                    connect(source, SIGNAL(log(QString,Transfer::LogLevel)), this, SIGNAL(log(QString,Transfer::LogLevel)));

                    slotUpdateCapabilities();

                    assignSegments(source);

                    //the job is already running, so also start the TransferDataSource
                    if (!m_assignTried && !m_startTried && m_putJob && m_open && (m_status == Job::Running))
                    {
                        source->start();
                    }
                }
                else
                {
                    kDebug(5001) << "A TransferDataSource could not be created for " << url.pathOrUrl();
                }
            }
            else if (usedDefined)
            {
                //increase the number of alowed mirrors as the user wants to use this one!
                ++m_maxMirrorsUsed;
                addMirror(url, used, numParalellConnections, usedDefined);
                return;
            }
            else
            {
                m_unusedUrls.append(url);
                m_unusedConnections.append(numParalellConnections);
            }
        }
    }
    else
    {
        if (m_sources.contains(url))
        {
            removeMirror(url);
        }
        else
        {
            m_unusedUrls.append(url);
            m_unusedConnections.append(numParalellConnections);
        }
    }
}

void DataSourceFactory::removeMirror(const KUrl &url)
{
    kDebug(5001) << "Removing mirror: " << url;
    if (m_sources.contains(url))
    {
        TransferDataSource *source = m_sources[url];
        source->stop();
        const QList<QPair<int, int> > assigned = source->assignedSegments();
        m_sources.remove(url);
        m_unusedUrls.append(url);
        m_unusedConnections.append(source->paralellSegments());
        delete source;

        for (int i = 0; i < assigned.count(); ++i)
        {
            const int start = assigned[i].first;
            const int end = assigned[i].second;
            if ((start != -1) && (end != -1)) {
                m_startedChunks->setRange(start, end, false);
                kDebug(5001) << "Segmentrange" << start << '-' << end << "not assigned anymore.";
            }
        }
    }

    if ((m_status == Job::Running) && assignNeeded()) {
        //here we only handle the case when there are existing TransferDataSources,
        //the other case is triggered when stopping and then starting again
        if (m_sources.count()) {
            kDebug(5001) << "Assigning a TransferDataSource.";
            //simply assign a TransferDataSource
            assignSegments(*m_sources.begin());
        }
    }
}

void DataSourceFactory::setMirrors(const QHash<KUrl, QPair<bool, int> > &mirrors)
{
    //first remove the not set DataSources
    QList<KUrl> oldUrls = m_sources.keys();
    QList<KUrl> newUrls = mirrors.keys();
    foreach (const KUrl &url, oldUrls)
    {
        if (!newUrls.contains(url))
        {
            removeMirror(url);
        }
    }

    //remove all unused Mirrors and simply readd them below
    m_unusedUrls.clear();
    m_unusedConnections.clear();

    //second modify the existing DataSources and add the new ones
    QHash<KUrl, QPair<bool, int> >::const_iterator it;
    QHash<KUrl, QPair<bool, int> >::const_iterator itEnd = mirrors.constEnd();
    for (it = mirrors.constBegin(); it != itEnd; ++it)
    {
        addMirror(it.key(), it.value().first, it.value().second, true);
    }
}

QHash<KUrl, QPair<bool, int> > DataSourceFactory::mirrors() const
{
    QHash<KUrl, QPair<bool, int> > mirrors;

    QHash<KUrl, TransferDataSource*>::const_iterator it;
    QHash<KUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
    for (it = m_sources.constBegin(); it != itEnd; ++it) {
        mirrors[it.key()] = QPair<bool, int>(true, (*it)->paralellSegments());
    }

    for (int i = 0; i < m_unusedUrls.count(); ++i) {
        mirrors[m_unusedUrls[i]] = QPair<bool, int>(false, m_unusedConnections[i]);
    }

    return mirrors;
}

bool DataSourceFactory::assignNeeded() const
{
    bool assignNeeded = true;
    QHash<KUrl, TransferDataSource*>::const_iterator it;
    QHash<KUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
    for (it = m_sources.constBegin(); it != itEnd; ++it) {
        if ((*it)->currentSegments()) {
            //at least one TransferDataSource is still running, so no assign needed
            assignNeeded = false;
            break;
        }
    }
    return assignNeeded;
}

void DataSourceFactory::brokenSegments(TransferDataSource *source, const QPair<int, int> &segmentRange)
{
    kDebug(5001) << "Segments" << segmentRange << "broken," << source;
    if (!source || !m_startedChunks || !m_finishedChunks || (segmentRange.first < 0) || (segmentRange.second < 0) || (static_cast<quint32>(segmentRange.second) > m_finishedChunks->getNumBits()))
    {
        return;
    }

    const int start = segmentRange.first;
    const int end = segmentRange.second;
    if ((start != -1) && (end != -1)) {
        m_startedChunks->setRange(start, end, false);
    }

    removeMirror(source->sourceUrl());
}


void DataSourceFactory::broken(TransferDataSource *source, TransferDataSource::Error error)
{
    kDebug(5001) << source << "is broken with error" << error;
    const QString url = source->sourceUrl().pathOrUrl();

    removeMirror(source->sourceUrl());

    if (error == TransferDataSource::WrongDownloadSize)
    {
        KMessageBox::error(0, i18nc("A mirror is removed when the file has the wrong download size", "%1 removed as it did report a wrong file size.", url), i18n("Error"));
    }
}

void DataSourceFactory::slotFreeSegments(TransferDataSource *source, QPair<int, int> segmentRange)
{
    kDebug(5001) << "Segments freed:" << source << segmentRange;

    const int start = segmentRange.first;
    const int end = segmentRange.second;
    if ((start != -1) && (end != -1)) {
        m_startedChunks->setRange(start, end, false);
        kDebug(5001) << "Segmentrange" << start << '-' << end << "not assigned anymore.";
    }

    assignSegments(source);
}

void DataSourceFactory::finishedSegment(TransferDataSource *source, int segmentNumber, bool connectionFinished)
{
    if (!source || (segmentNumber < 0) || (static_cast<quint32>(segmentNumber) > m_finishedChunks->getNumBits()))
    {
        kDebug(5001) << "Incorrect data";
        return;
    }

    m_finishedChunks->set(segmentNumber, true);

    if (!connectionFinished)
    {
        kDebug(5001) << "Some segments still not finished";
        return;
    }

    m_finished = m_finishedChunks->allOn();
    if (m_finished)
    {
        kDebug() << "All segments have been downloaded.";
        return;
    }

    assignSegments(source);
}

void DataSourceFactory::assignSegments(TransferDataSource *source)
{
    if (!m_startedChunks || !m_finishedChunks)
    {
        kDebug(5001) << "Assign tried";
        m_assignTried = true;
        return;
    }
    if (m_finishedChunks->allOn())
    {
        kDebug(5001) << "All segments are finished already.";
        return;
    }

    //no more segments needed for that TransferDataSource
    if (source->changeNeeded() <= 0) {
        kDebug(5001) << "No change needed for" << source;
        return;
    }

    //find the segments that should be assigned to the transferDataSource
    int newStart = -1;
    int newEnd = -1;

    //a split needed
    if (m_startedChunks->allOn()) {
        int unfinished = 0;
        TransferDataSource *target = 0;
        foreach (TransferDataSource *source, m_sources) {
            int temp = source->countUnfinishedSegments();
            if (temp > unfinished) {
                unfinished = temp;
                target = source;
            }
        }
        if (!unfinished || !target) {
            return;
        }

        const QPair<int, int> splitResult = target->split();
        newStart = splitResult.first;
        newEnd = splitResult.second;
    } else {
        m_startedChunks->getContinuousRange(&newStart, &newEnd, false);
    }

    if ((newStart == -1) || (newEnd == -1))
    {
        kDebug(5001) << "No segment can be assigned.";
        return;
    }

    const KIO::fileoffset_t rest = m_size % m_segSize;

    //the lastSegsize is rest, but only if there is a rest and it is the last segment of the download
    const KIO::fileoffset_t lastSegSize = ((static_cast<uint>(newEnd + 1) == m_startedChunks->getNumBits() && rest) ? rest : m_segSize);

    kDebug(5001) << "Segments assigned:" << newStart << "-" << newEnd << "segment-size:" << m_segSize << "rest:" << rest;
    m_startedChunks->setRange(newStart, newEnd, true);
    source->addSegments(qMakePair(m_segSize, lastSegSize), qMakePair(newStart, newEnd));

    //there should still be segments added to this transfer
    if (source->changeNeeded() > 0) {
        assignSegments(source);
    }
}

//TODO implement checks if the correct offsets etc. are used + error recovering e.g. when something else
//touches the file
void DataSourceFactory::slotWriteData(KIO::fileoffset_t offset, const QByteArray &data, bool &worked)
{
    worked = !m_blocked && !m_movingFile && m_open;
    if (m_blocked || m_movingFile || !m_open)
    {
        return;
    }

    m_blocked = true;
    m_tempOffset = offset;
    m_tempData = data;
    m_putJob->seek(offset);
}

void DataSourceFactory::slotOffset(KIO::Job *job , KIO::filesize_t offset)
{
    Q_UNUSED(job)
    Q_UNUSED(offset)

    m_putJob->write(m_tempData);
}

void DataSourceFactory::slotDataWritten(KIO::Job *job, KIO::filesize_t written)
{
    Q_UNUSED(job)

    KIO::filesize_t tempSize = static_cast<KIO::filesize_t>(m_tempData.size());
    //the complete data has been written
    if (written == tempSize)//TODO if not same cache it temporarly!
    {
        m_downloadedSize += written;
        emit dataSourceFactoryChange(Transfer::Tc_DownloadedSize);
//             m_tempCache.clear();
    }

    if (m_finished)
    {
        m_speedTimer->stop();
        killPutJob();
        changeStatus(Job::Finished);
    }
    m_tempData.clear();
    m_blocked = false;
}

void DataSourceFactory::slotPercent(KJob* job, ulong p)
{
    Q_UNUSED(job)
    m_percent = p;
    emit dataSourceFactoryChange(Transfer::Tc_Percent);
}

void DataSourceFactory::speedChanged()
{
    m_speed = (m_downloadedSize - m_prevDownloadedSizes.first()) / (SPEEDTIMER * m_prevDownloadedSizes.size() / 1000);//downloaded in 1 second

    m_prevDownloadedSizes.append(m_downloadedSize);
    if(m_prevDownloadedSizes.size() > 10)
        m_prevDownloadedSizes.removeFirst();

    ulong percent = (m_size ? (m_downloadedSize * 100 / m_size) : 0);
    const bool percentChanged = (percent != m_percent);
    m_percent = percent;

    Transfer::ChangesFlags change = (percentChanged ? (Transfer::Tc_DownloadSpeed | Transfer::Tc_Percent) : Transfer::Tc_DownloadSpeed);
    emit dataSourceFactoryChange(change);
}

void DataSourceFactory::killPutJob()
{
    if (m_putJob)
    {
        kDebug(5001) << "Closing the file";
        m_open = false;
        m_putJob->close();
        m_putJob = 0;
    }
}

void DataSourceFactory::slotPutJobDestroyed(QObject *job)
{
    Q_UNUSED(job)

    m_putJob = 0;
}

bool DataSourceFactory::setNewDestination(const KUrl &newDestination)
{
    m_newDest = newDestination;
    if (m_newDest.isValid() && (m_newDest != m_dest))
    {
        //No files created yet, simply change the urls
        if (!m_downloadInitialized) {
            m_dest = m_newDest;
            if (m_verifier) {
                verifier()->setDestination(m_dest);
            }
            if (m_signature) {
                signature()->setDestination(m_dest);
            }

            return true;
        } else if (QFile::exists(m_dest.pathOrUrl())) {
            //create all dirs needed
            QDir dir;
            dir.mkpath(m_newDest.directory());

            m_statusBeforeMove = m_status;
            stop();
            changeStatus(Job::Moving);
            m_movingFile = true;

            //still a write in progress
            if (m_blocked)
            {
                QTimer::singleShot(1000, this, SLOT(startMove()));
            }
            else
            {
                startMove();
            }
            return true;
        }
    }
    return false;
}

void DataSourceFactory::startMove()
{
    killPutJob();

    KIO::Job *move = KIO::file_move(m_dest, m_newDest, -1, KIO::HideProgressInfo);
    connect(move, SIGNAL(result(KJob*)), this, SLOT(newDestResult(KJob*)));
    connect(move, SIGNAL(percent(KJob*,ulong)), this, SLOT(slotPercent(KJob*,ulong)));

    m_dest = m_newDest;
    verifier()->setDestination(m_dest);
    signature()->setDestination(m_dest);
}

void DataSourceFactory::newDestResult(KJob *job)
{
    Q_UNUSED(job)//TODO handle errors etc.?

    m_movingFile = false;
    changeStatus(m_statusBeforeMove);
    if (m_status == Job::Running)
    {
        start();
    }
}

void DataSourceFactory::repair()
{
    if (verifier()->status() != Verifier::NotVerified)
    {
        return;
    }

    m_finished = false;

    connect(verifier(), SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)), this, SLOT(slotRepair(QList<KIO::fileoffset_t>,KIO::filesize_t)));

    verifier()->brokenPieces();
}

void DataSourceFactory::slotRepair(const QList<KIO::fileoffset_t> &offsets, KIO::filesize_t length)
{
    disconnect(verifier(), SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)), this, SLOT(slotRepair(QList<KIO::fileoffset_t>,KIO::filesize_t)));

    if (!m_startedChunks || !m_finishedChunks)
    {
        kDebug(5001) << "Redownload everything";
        m_downloadedSize = 0;
    }
    else
    {
        if (offsets.isEmpty()) {
            m_startedChunks->clear();
            m_finishedChunks->clear();
        }
        kDebug(5001) << "Redownload broken pieces";
        for (int i = 0; i < offsets.count(); ++i) {
            const int start = offsets[i] / m_segSize;
            const int end = std::ceil(length / static_cast<double>(m_segSize)) - 1 + start;
            m_startedChunks->setRange(start, end, false);
            m_finishedChunks->setRange(start, end, false);
        }

        m_downloadedSize = m_segSize * m_finishedChunks->numOnBits();
    }
    m_prevDownloadedSizes.clear();
    m_prevDownloadedSizes.append(m_downloadedSize);

    //remove all current mirrors and readd the first unused mirror
    const QList<KUrl> mirrors = m_sources.keys();//FIXME only remove the mirrors of the broken segments?! --> for that m_assignedChunks would need to be saved was well
    foreach (const KUrl &mirror, mirrors)
    {
        removeMirror(mirror);
    }
    addMirror(m_unusedUrls.takeFirst(), true, m_unusedConnections.takeFirst());

    m_speed = 0;

    Transfer::ChangesFlags change = (Transfer::Tc_DownloadSpeed | Transfer::Tc_DownloadedSize);
    if (m_size) {
        change |= Transfer::Tc_Percent;
        m_percent = (m_downloadedSize * 100 / m_size);
    }
    emit dataSourceFactoryChange(change);
    m_status = Job::Stopped;

    start();
}

void DataSourceFactory::load(const QDomElement *element)
{
    if (!element)
    {
        return;
    }

    QDomElement e = element->firstChildElement("factory");
    if (e.isNull())
    {
        e = element->firstChildElement("factories").firstChildElement("factory");
    }

    //only try to load values if they haven't been set
    if (m_dest.isEmpty())
    {
        m_dest = KUrl(e.attribute("dest"));
    }

    verifier()->load(e);
    signature()->load(e);

    Transfer::ChangesFlags change = Transfer::Tc_None;

    if (!m_size) {
        m_size = e.attribute("size").toULongLong();
        change |= Transfer::Tc_TotalSize;
    }
    KIO::fileoffset_t tempSegSize = e.attribute("segementSize").toLongLong();
    if (tempSegSize)
    {
        m_segSize = tempSegSize;
    }
    if (!m_downloadedSize) {
        m_downloadedSize = e.attribute("processedSize").toULongLong();
        change |= Transfer::Tc_DownloadedSize;
        if (m_size) {
            m_percent = (m_downloadedSize * 100 / m_size);
            change |= Transfer::Tc_Percent;
        }
    }
    if (e.hasAttribute("doDownload"))
    {
        m_doDownload = QVariant(e.attribute("doDownload")).toBool();
    }
    if (e.hasAttribute("downloadInitialized")) {
        m_downloadInitialized = QVariant(e.attribute("downloadInitialized")).toBool();
    }
    if (e.hasAttribute("maxMirrorsUsed"))
    {
        bool worked;
        m_maxMirrorsUsed = e.attribute("maxMirrorsUsed").toInt(&worked);
        m_maxMirrorsUsed = (worked ? m_maxMirrorsUsed : 3);
    }
    m_sizeInitiallyDefined = QVariant(e.attribute("sizeInitiallyDefined", "false")).toBool();
    m_sizeFoundOnFinish = QVariant(e.attribute("sizeFoundOnFinish", "false")).toBool();

    //load the finishedChunks
    const QDomElement chunks = e.firstChildElement("chunks");
    const QDomNodeList chunkList = chunks.elementsByTagName("chunk");

    const quint32 numBits = chunks.attribute("numBits").toInt();
    const quint32 numBytes = chunks.attribute("numBytes").toInt();
    QVarLengthArray<quint8> data(numBytes);

    if (numBytes && (numBytes == chunkList.length()))
    {
        for (quint32 i = 0; i < numBytes; ++i)
        {
            const quint8 value = chunkList.at(i).toElement().text().toInt();
            data[i] = value;
        }

        if (!m_finishedChunks)
        {
            m_finishedChunks = new BitSet(data.data(), numBits);
            kDebug(5001) << m_finishedChunks->numOnBits() << " bits on of " << numBits << " bits.";
        }

        //set the finished chunks to started
        if (!m_startedChunks)
        {
            m_startedChunks = new BitSet(data.data(), numBits);
        }

    }
    m_prevDownloadedSizes.clear();
    m_prevDownloadedSizes.append(m_downloadedSize);

    init();

    //add the used urls
    const QDomNodeList urls = e.firstChildElement("urls").elementsByTagName("url");
    for (int i = 0; i < urls.count(); ++i)
    {
        const QDomElement urlElement = urls.at(i).toElement();
        const KUrl url = KUrl(urlElement.text());
        const int connections = urlElement.attribute("numParalellSegments").toInt();
        addMirror(url, true, connections, true);
    }

    //add the unused urls
    const QDomNodeList unusedUrls = e.firstChildElement("unusedUrls").elementsByTagName("url");
    for (int i = 0; i < unusedUrls.count(); ++i)
    {
        const QDomElement urlElement = unusedUrls.at(i).toElement();
        const KUrl url = KUrl(urlElement.text());
        const int connections = urlElement.attribute("numParalellSegments").toInt();
        addMirror(url, false, connections, true);
    }

    m_status = static_cast<Job::Status>(e.attribute("status").toInt());

    if (change != Transfer::Tc_None) {
        emit dataSourceFactoryChange(change);
    }
}

void DataSourceFactory::changeStatus(Job::Status status)
{
    Transfer::ChangesFlags change = Transfer::Tc_Status;
    m_status = status;

    switch (m_status)
    {
        case Job::Aborted:
        case Job::Moving:
        case Job::Stopped:
            m_speed = 0;
            change |= Transfer::Tc_DownloadSpeed;
            break;
        case Job::Running:
            break;
        case Job::Finished:
            m_speed = 0;
            m_percent = 100;

            if (m_size) {
                m_downloadedSize = m_size;
                change |= Transfer::Tc_DownloadedSize;
            } else if (m_downloadedSize) {
                m_sizeFoundOnFinish = true;
                m_size = m_downloadedSize;
                change |= Transfer::Tc_TotalSize;
            }

            change |= Transfer::Tc_DownloadSpeed | Transfer::Tc_Percent;

            if (Settings::checksumAutomaticVerification() && verifier()->isVerifyable()) {
                verifier()->verify();
            }
            if (Settings::signatureAutomaticVerification() && signature()->isVerifyable()) {
                signature()->verify();
            }

            slotUpdateCapabilities();
            break;
        default:
            //TODO handle Delayed
            break;
    }

    emit dataSourceFactoryChange(change);
}

void DataSourceFactory::save(const QDomElement &element)
{
    if (element.isNull())
    {
        return;
    }

    QDomElement e = element;
    QDomDocument doc(e.ownerDocument());


    QDomElement factories = e.firstChildElement("factories");
    if (factories.isNull())
    {
        factories = doc.createElement("factories");
    }

    QDomElement factory = doc.createElement("factory");
    factory.setAttribute("dest", m_dest.url());

    if (!m_finishedChunks || m_sizeFoundOnFinish) {
        factory.setAttribute("processedSize", m_downloadedSize);
    }
    factory.setAttribute("size", m_size);
    factory.setAttribute("segementSize", m_segSize);
    factory.setAttribute("status", m_status);
    factory.setAttribute("doDownload", m_doDownload);
    factory.setAttribute("downloadInitialized", m_downloadInitialized);
    factory.setAttribute("maxMirrorsUsed", m_maxMirrorsUsed);
    factory.setAttribute("sizeInitiallyDefined", m_sizeInitiallyDefined);
    factory.setAttribute("sizeFoundOnFinish", m_sizeFoundOnFinish);

    verifier()->save(factory);
    signature()->save(factory);

    //set the finished chunks, but only if chunks were actually used
    if (m_finishedChunks && !m_sizeFoundOnFinish) {
        const KIO::fileoffset_t rest = m_size % m_segSize;
        //the lastSegsize is rest, but only if there is a rest and it is the last segment of the download
        const KIO::fileoffset_t lastSegSize = (rest ? rest : m_segSize);

        //not m_downloadedSize is stored, but the bytes that do not have to be redownloaded
        const bool lastOn = m_finishedChunks->get(m_finishedChunks->getNumBits() - 1);
        factory.setAttribute("processedSize", m_segSize * (m_finishedChunks->numOnBits() - lastOn) + lastOn * lastSegSize);

        QDomElement chunks = doc.createElement("chunks");
        chunks.setAttribute("numBits", m_finishedChunks->getNumBits());
        chunks.setAttribute("numBytes", m_finishedChunks->getNumBytes());

        const quint8 *data = m_finishedChunks->getData();
        for (quint32 i = 0; i < m_finishedChunks->getNumBytes(); ++i)
        {
            QDomElement chunk = doc.createElement("chunk");
            QDomText num = doc.createTextNode(QString::number(data[i]));
            chunk.setAttribute("number", i);
            chunk.appendChild(num);
            chunks.appendChild(chunk);
        }
        factory.appendChild(chunks);
    }

    //set the used urls
    QDomElement urls = doc.createElement("urls");
    QHash<KUrl, TransferDataSource*>::const_iterator it;
    QHash<KUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
    for (it = m_sources.constBegin(); it != itEnd; ++it) {
        QDomElement url = doc.createElement("url");
        const QDomText text = doc.createTextNode(it.key().url());
        url.appendChild(text);
        url.setAttribute("numParalellSegments", (*it)->paralellSegments());
        urls.appendChild(url);
        factory.appendChild(urls);
    }

    //set the unused urls
    urls = doc.createElement("unusedUrls");
    for (int i = 0; i < m_unusedUrls.count(); ++i) {
        QDomElement url = doc.createElement("url");
        const QDomText text = doc.createTextNode(m_unusedUrls.at(i).url());
        url.appendChild(text);
        url.setAttribute("numParalellSegments", m_unusedConnections.at(i));
        urls.appendChild(url);
        factory.appendChild(urls);
    }

    factories.appendChild(factory);
    e.appendChild(factories);
}


bool DataSourceFactory::isValid() const
{
    //FIXME
    return true;
    bool valid = (m_size && m_segSize && m_dest.isValid() && !m_sources.isEmpty());

    //if the download is finished the it is valid no matter what is set or not
    if (m_status == Job::Finished)
    {
        valid = true;
    }
    kDebug() << m_size << m_segSize << m_dest.isValid() << !m_sources.isEmpty();
    return valid;
}

Verifier *DataSourceFactory::verifier()
{
    if (!m_verifier) {
        m_verifier = new Verifier(m_dest, this);
    }

    return m_verifier;
}

Signature *DataSourceFactory::signature()
{
    if (!m_signature) {
        m_signature = new Signature(m_dest, this);
    }

    return m_signature;
}

void DataSourceFactory::slotUpdateCapabilities()
{
    const Transfer::Capabilities oldCaps = capabilities();
    Transfer::Capabilities newCaps = 0;

    if ((status() == Job::Finished) || (status() == Job::Stopped)) {
        newCaps |= Transfer::Cap_Moving | Transfer::Cap_Renaming;
    } else {
        foreach (TransferDataSource *source, m_sources) {
            if (!source->assignedSegments().isEmpty()) {
                if (newCaps) {
                    newCaps &= source->capabilities();
                } else {
                    newCaps = source->capabilities();
                }
            }
        }
    }

    if (newCaps & Transfer::Cap_Resuming) {
        newCaps |= Transfer::Cap_Moving | Transfer::Cap_Renaming;
    }

    newCaps |= Transfer::Cap_MultipleMirrors;

    if (oldCaps != newCaps) {
        m_capabilities = newCaps;
        emit capabilitiesChanged();
    }
}

#include "datasourcefactory.moc"
