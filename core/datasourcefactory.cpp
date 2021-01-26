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

#include <QDir>
#include <QTimer>
#include <QVarLengthArray>
#include <QDomText>

#include <KIO/FileJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMountPoint>

#include "kget_debug.h"


#include <qplatformdefs.h>

const int SPEEDTIMER = 1000;//1 second...

DataSourceFactory::DataSourceFactory(QObject *parent, const QUrl &dest, KIO::filesize_t size, KIO::fileoffset_t segSize)
  : QObject(parent),
    m_capabilities(),
    m_dest(dest),
    m_size(size),
    m_downloadedSize(0),
    m_segSize(segSize),
    m_speed(0),
    m_percent(0),
    m_tempOffset(0),
    m_startedChunks(nullptr),
    m_finishedChunks(nullptr),
    m_putJob(nullptr),
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
    m_speedTimer(nullptr),
    m_status(Job::Stopped),
    m_statusBeforeMove(m_status),
    m_verifier(nullptr),
    m_signature(nullptr)
{
    qCDebug(KGET_DEBUG) << "Initialize DataSourceFactory: Dest: " + m_dest.toLocalFile() + "Size: " + QString::number(m_size) + "SegSize: " + QString::number(m_segSize);

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
        connect(m_speedTimer, &QTimer::timeout, this, &DataSourceFactory::speedChanged);
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
    qCDebug(KGET_DEBUG) << "Find the filesize" << this;
    if (!m_size && !m_dest.isEmpty() && !m_sources.isEmpty()) {
        foreach (TransferDataSource *source, m_sources) {
            if (source->capabilities() & Transfer::Cap_FindFilesize) {
                connect(source, &TransferDataSource::foundFileSize, this, &DataSourceFactory::slotFoundFileSize);
                connect(source, &TransferDataSource::finishedDownload, this, &DataSourceFactory::slotFinishedDownload);

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
    qCDebug(KGET_DEBUG) << source << "found size" << m_size << "and is assigned segments" << segmentRange << this;
    Q_EMIT dataSourceFactoryChange(Transfer::Tc_TotalSize);

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

bool DataSourceFactory::checkLocalFile()
{
    QString dest_orig = m_dest.toLocalFile();
    QString _dest_part(dest_orig);

    QT_STATBUF buff_part;
    bool bPartExists = (QT_STAT( _dest_part.toUtf8().constData(), &buff_part ) != -1);
    if(!bPartExists)
    {
        QString _dest = dest_orig;
        int fd = -1;
        mode_t initialMode = 0666;

        fd = QT_OPEN(_dest.toUtf8().constData(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
        if ( fd < 0 )
        {
            qCDebug(KGET_DEBUG) << " error";
            return false;
        }
        else
        {
            close(fd);
        }
    }//TODO: Port to use Qt functions maybe

    qCDebug(KGET_DEBUG) << "success";
    return true;
}

void DataSourceFactory::start()
{
    qCDebug(KGET_DEBUG) << "Start DataSourceFactory";
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
        qCDebug(KGET_DEBUG) << "Removing existing file.";
        m_startTried = true;
        FileDeleter::deleteFile(m_dest, this, SLOT(slotRemovedFile()));
        return;
    }

    m_downloadInitialized = true;

    //create all dirs needed
    QDir dir;
    dir.mkpath(m_dest.adjusted(QUrl::RemoveFilename).toLocalFile());
    if (checkLocalFile()) {
        if (!m_putJob) {
            m_putJob = KIO::open(m_dest, QIODevice::WriteOnly | QIODevice::ReadOnly);
            connect(m_putJob, &KIO::FileJob::open, this, &DataSourceFactory::slotOpen);
            connect(m_putJob, &QObject::destroyed, this, &DataSourceFactory::slotPutJobDestroyed);
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
            qCDebug(KGET_DEBUG) << "Assigning a TransferDataSource.";
            //simply assign a TransferDataSource
            assignSegments(*m_sources.begin());
        } else if (m_unusedUrls.count()) {
            qCDebug(KGET_DEBUG) << "Assigning an unused mirror";
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
            KMountPoint::Ptr mountPoint = KMountPoint::currentMountPoints().findByPath(m_dest.adjusted(QUrl::RemoveFilename).toString());
            if (!mountPoint) {
                if (mountPoint->mountType() == "vfat") {//TODO check what is reported on Windows for vfat
                    stop();
                    KMessageBox::error(nullptr, i18n("Filesize is larger than maximum file size supported by VFAT."), i18n("Error"));
                    return;
                }
            }
        }

        QFile::resize(m_dest.toString(), m_size);//TODO should we keep that?
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
    qCDebug(KGET_DEBUG) << "File has been removed" << this;
    if (m_startTried) {
        m_startTried = false;
        start();
    }
}

void DataSourceFactory::slotOpen(KIO::Job *job)
{
    Q_UNUSED(job)
    qCDebug(KGET_DEBUG) << "File opened" << this;

    if (!m_speedTimer)
    {
        init();
    }

    connect(m_putJob, &KIO::FileJob::position, this, &DataSourceFactory::slotOffset);
    connect(m_putJob, &KIO::FileJob::written, this, &DataSourceFactory::slotDataWritten);
    m_open = true;

    if (m_startTried)
    {
        start();
    }
}

void DataSourceFactory::stop()
{
    qCDebug(KGET_DEBUG) << "Stopping" << this;
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

void DataSourceFactory::addMirror(const QUrl &url, int numParallelConnections)
{
    addMirror(url, true, numParallelConnections, false);
}

void DataSourceFactory::addMirror(const QUrl &url, bool used, int numParallelConnections)
{
    addMirror(url, used, numParallelConnections, true);
}

void DataSourceFactory::addMirror(const QUrl &url, bool used, int numParallelConnections, bool usedDefined)
{
    if (!url.isValid() || url.isEmpty())
    {
        qCDebug(KGET_DEBUG) << "Url is not usable: " << url.toString();
        return;
    }
    if (numParallelConnections <= 0)
    {
        numParallelConnections = 1;
    }
    if (!usedDefined)
    {
        used = true;
    }

    if (used)
    {
        //there is already a TransferDataSource with that url, reuse it and modify numParallelSegments
        if (m_sources.contains(url))
        {
            TransferDataSource *source = m_sources[url];
            source->setParallelSegments(numParallelConnections);
            if (source->changeNeeded() > 0) {
                assignSegments(source);
            } else {
                for (int i = source->changeNeeded(); i < 0; ++i)
                {
                    const QPair<int, int> removed = source->removeConnection();
                    qCDebug(KGET_DEBUG) << "Remove connection with segments" << removed;
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
                    qCDebug(KGET_DEBUG) << "Successfully created a TransferDataSource for " << url.toString() << this;

                    //url might have been an unused Mirror, so remove it in any case
                    const int index = m_unusedUrls.indexOf(url);
                    if (index > -1 )
                    {
                        m_unusedUrls.removeAt(index);
                        m_unusedConnections.removeAt(index);
                    }

                    m_sources[url] = source;
                    m_sources[url]->setParallelSegments(numParallelConnections);
                    if (m_sizeInitiallyDefined) {
                        source->setSupposedSize(m_size);
                    }

                    connect(source, &TransferDataSource::capabilitiesChanged, this, &DataSourceFactory::slotUpdateCapabilities);
                    connect(source, SIGNAL(brokenSegments(TransferDataSource*,QPair<int,int>)), this, SLOT(brokenSegments(TransferDataSource*,QPair<int,int>)));
                    connect(source, &TransferDataSource::broken, this, &DataSourceFactory::broken);
                    connect(source, &TransferDataSource::finishedSegment, this, &DataSourceFactory::finishedSegment);
                    connect(source, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)), this, SLOT(slotWriteData(KIO::fileoffset_t,QByteArray,bool&)));
                    connect(source, SIGNAL(freeSegments(TransferDataSource*,QPair<int,int>)), this, SLOT(slotFreeSegments(TransferDataSource*,QPair<int,int>)));
                    connect(source, &TransferDataSource::log, this, &DataSourceFactory::log);
                    connect(source, &TransferDataSource::urlChanged, this, &DataSourceFactory::slotUrlChanged);

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
                    qCDebug(KGET_DEBUG) << "A TransferDataSource could not be created for " << url.toString();
                }
            }
            else if (usedDefined)
            {
                //increase the number of allowed mirrors as the user wants to use this one!
                ++m_maxMirrorsUsed;
                addMirror(url, used, numParallelConnections, usedDefined);
                return;
            }
            else
            {
                m_unusedUrls.append(url);
                m_unusedConnections.append(numParallelConnections);
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
            m_unusedConnections.append(numParallelConnections);
        }
    }
}

void DataSourceFactory::slotUrlChanged(const QUrl &old, const QUrl &newUrl)
{
    TransferDataSource * ds = m_sources.take(old);
    m_sources[newUrl] = ds;
    Q_EMIT dataSourceFactoryChange(Transfer::Tc_Source | Transfer::Tc_FileName);
}

void DataSourceFactory::removeMirror(const QUrl &url)
{
    qCDebug(KGET_DEBUG) << "Removing mirror: " << url;
    if (m_sources.contains(url))
    {
        TransferDataSource *source = m_sources[url];
        source->stop();
        const QList<QPair<int, int> > assigned = source->assignedSegments();
        m_sources.remove(url);
        m_unusedUrls.append(url);
        m_unusedConnections.append(source->parallelSegments());
        delete source;

        for (int i = 0; i < assigned.count(); ++i)
        {
            const int start = assigned[i].first;
            const int end = assigned[i].second;
            if ((start != -1) && (end != -1)) {
                m_startedChunks->setRange(start, end, false);
                qCDebug(KGET_DEBUG) << "Segmentrange" << start << '-' << end << "not assigned anymore.";
            }
        }
    }

    if ((m_status == Job::Running) && assignNeeded()) {
        //here we only handle the case when there are existing TransferDataSources,
        //the other case is triggered when stopping and then starting again
        if (m_sources.count()) {
            qCDebug(KGET_DEBUG) << "Assigning a TransferDataSource.";
            //simply assign a TransferDataSource
            assignSegments(*m_sources.begin());
        }
    }
}

void DataSourceFactory::setMirrors(const QHash<QUrl, QPair<bool, int> > &mirrors)
{
    //first remove the not set DataSources
    QList<QUrl> oldUrls = m_sources.keys();
    QList<QUrl> newUrls = mirrors.keys();
    
    foreach (const QUrl &url, oldUrls)
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
    QHash<QUrl, QPair<bool, int> >::const_iterator it;
    QHash<QUrl, QPair<bool, int> >::const_iterator itEnd = mirrors.constEnd();
    for (it = mirrors.constBegin(); it != itEnd; ++it)
    {
        addMirror(it.key(), it.value().first, it.value().second, true);
    }
}

QHash<QUrl, QPair<bool, int> > DataSourceFactory::mirrors() const
{
    QHash<QUrl, QPair<bool, int> > mirrors;

    QHash<QUrl, TransferDataSource*>::const_iterator it;
    QHash<QUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
    for (it = m_sources.constBegin(); it != itEnd; ++it) {
        mirrors[it.key()] = QPair<bool, int>(true, (*it)->parallelSegments());
    }

    for (int i = 0; i < m_unusedUrls.count(); ++i) {
        mirrors[m_unusedUrls[i]] = QPair<bool, int>(false, m_unusedConnections[i]);
    }

    return mirrors;
}

bool DataSourceFactory::assignNeeded() const
{
    bool assignNeeded = true;
    QHash<QUrl, TransferDataSource*>::const_iterator it;
    QHash<QUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
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
    qCDebug(KGET_DEBUG) << "Segments" << segmentRange << "broken," << source;
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
    qCDebug(KGET_DEBUG) << source << "is broken with error" << error;
    const QString url = source->sourceUrl().toString();

    removeMirror(source->sourceUrl());

    if (error == TransferDataSource::WrongDownloadSize)
    {
        KMessageBox::error(nullptr, i18nc("A mirror is removed when the file has the wrong download size", "%1 removed as it did report a wrong file size.", url), i18n("Error"));
    }
}

void DataSourceFactory::slotFreeSegments(TransferDataSource *source, QPair<int, int> segmentRange)
{
    qCDebug(KGET_DEBUG) << "Segments freed:" << source << segmentRange;

    const int start = segmentRange.first;
    const int end = segmentRange.second;
    if ((start != -1) && (end != -1)) {
        m_startedChunks->setRange(start, end, false);
        qCDebug(KGET_DEBUG) << "Segmentrange" << start << '-' << end << "not assigned anymore.";
    }

    assignSegments(source);
}

void DataSourceFactory::finishedSegment(TransferDataSource *source, int segmentNumber, bool connectionFinished)
{
    if (!source || (segmentNumber < 0) || (static_cast<quint32>(segmentNumber) > m_finishedChunks->getNumBits()))
    {
        qCDebug(KGET_DEBUG) << "Incorrect data";
        return;
    }

    m_finishedChunks->set(segmentNumber, true);

    if (!connectionFinished)
    {
        qCDebug(KGET_DEBUG) << "Some segments still not finished";
        return;
    }

    m_finished = m_finishedChunks->allOn();
    if (m_finished)
    {
        qDebug() << "All segments have been downloaded.";
        return;
    }

    assignSegments(source);
}

void DataSourceFactory::assignSegments(TransferDataSource *source)
{
    if (!m_startedChunks || !m_finishedChunks)
    {
        qCDebug(KGET_DEBUG) << "Assign tried";
        m_assignTried = true;
        return;
    }
    if (m_finishedChunks->allOn())
    {
        qCDebug(KGET_DEBUG) << "All segments are finished already.";
        return;
    }

    //no more segments needed for that TransferDataSource
    if (source->changeNeeded() <= 0) {
        qCDebug(KGET_DEBUG) << "No change needed for" << source;
        return;
    }

    //find the segments that should be assigned to the transferDataSource
    int newStart = -1;
    int newEnd = -1;

    //a split needed
    if (m_startedChunks->allOn()) {
        int unfinished = 0;
        TransferDataSource *target = nullptr;
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
        qCDebug(KGET_DEBUG) << "No segment can be assigned.";
        return;
    }

    const KIO::fileoffset_t rest = m_size % m_segSize;

    //the lastSegsize is rest, but only if there is a rest and it is the last segment of the download
    const KIO::fileoffset_t lastSegSize = ((static_cast<uint>(newEnd + 1) == m_startedChunks->getNumBits() && rest) ? rest : m_segSize);

    qCDebug(KGET_DEBUG) << "Segments assigned:" << newStart << "-" << newEnd << "segment-size:" << m_segSize << "rest:" << rest;
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

    auto tempSize = static_cast<KIO::filesize_t>(m_tempData.size());
    //the complete data has been written
    if (written == tempSize)//TODO if not same cache it temporarily!
    {
        m_downloadedSize += written;
        Q_EMIT dataSourceFactoryChange(Transfer::Tc_DownloadedSize);
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
    Q_EMIT dataSourceFactoryChange(Transfer::Tc_Percent);
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
    Q_EMIT dataSourceFactoryChange(change);
}

void DataSourceFactory::killPutJob()
{
    if (m_putJob)
    {
        qCDebug(KGET_DEBUG) << "Closing the file";
        m_open = false;
        m_putJob->close();
        m_putJob = nullptr;
    }
}

void DataSourceFactory::slotPutJobDestroyed(QObject *job)
{
    Q_UNUSED(job)

    m_putJob = nullptr;
}

bool DataSourceFactory::setNewDestination(const QUrl &newDestination)
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
        } else if (QFile::exists(m_dest.toString())) {
            //create all dirs needed
            QDir dir;
            dir.mkpath(m_newDest.adjusted(QUrl::RemoveFilename).toString());

            m_statusBeforeMove = m_status;
            stop();
            changeStatus(Job::Moving);
            m_movingFile = true;

            //still a write in progress
            if (m_blocked)
            {
                QTimer::singleShot(1000, this, &DataSourceFactory::startMove);
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
    connect(move, &KJob::result, this, &DataSourceFactory::newDestResult);
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
        qCDebug(KGET_DEBUG) << "Redownload everything";
        m_downloadedSize = 0;
    }
    else
    {
        if (offsets.isEmpty()) {
            m_startedChunks->clear();
            m_finishedChunks->clear();
        }
        qCDebug(KGET_DEBUG) << "Redownload broken pieces";
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
    const QList<QUrl> mirrors = m_sources.keys();//FIXME only remove the mirrors of the broken segments?! --> for that m_assignedChunks would need to be saved was well
    foreach (const QUrl &mirror, mirrors)
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
    Q_EMIT dataSourceFactoryChange(change);
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
        m_dest = QUrl(e.attribute("dest"));
    }

    verifier()->load(e);
    signature()->load(e);

    Transfer::ChangesFlags change = Transfer::Tc_None;

    if (!m_size) {
        m_size = e.attribute("size").toULongLong();
        change |= Transfer::Tc_TotalSize;
    }
    KIO::fileoffset_t tempSegSize = e.attribute("segmentSize").toLongLong();
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
    const int numBytes = chunks.attribute("numBytes").toInt();
    QVarLengthArray<quint8> data(numBytes);

    if (numBytes && (numBytes == chunkList.length()))
    {
        for (int i = 0; i < numBytes; ++i)
        {
            const quint8 value = chunkList.at(i).toElement().text().toInt();
            data[i] = value;
        }

        if (!m_finishedChunks)
        {
            m_finishedChunks = new BitSet(data.data(), numBits);
            qCDebug(KGET_DEBUG) << m_finishedChunks->numOnBits() << " bits on of " << numBits << " bits.";
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
        const QUrl url = QUrl(urlElement.text());
        const int connections = urlElement.attribute("numParallelSegments").toInt();
        addMirror(url, true, connections, true);
    }

    //add the unused urls
    const QDomNodeList unusedUrls = e.firstChildElement("unusedUrls").elementsByTagName("url");
    for (int i = 0; i < unusedUrls.count(); ++i)
    {
        const QDomElement urlElement = unusedUrls.at(i).toElement();
        const QUrl url = QUrl(urlElement.text());
        const int connections = urlElement.attribute("numParallelSegments").toInt();
        addMirror(url, false, connections, true);
    }

    //m_status = static_cast<Job::Status>(e.attribute("status").toInt());

    if (change != Transfer::Tc_None) {
        Q_EMIT dataSourceFactoryChange(change);
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

    Q_EMIT dataSourceFactoryChange(change);
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
    factory.setAttribute("segmentSize", m_segSize);
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
    QHash<QUrl, TransferDataSource*>::const_iterator it;
    QHash<QUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
    for (it = m_sources.constBegin(); it != itEnd; ++it) {
        QDomElement url = doc.createElement("url");
        const QDomText text = doc.createTextNode(it.key().url());
        url.appendChild(text);
        url.setAttribute("numParallelSegments", (*it)->parallelSegments());
        urls.appendChild(url);
        factory.appendChild(urls);
    }

    //set the unused urls
    urls = doc.createElement("unusedUrls");
    for (int i = 0; i < m_unusedUrls.count(); ++i) {
        QDomElement url = doc.createElement("url");
        const QDomText text = doc.createTextNode(m_unusedUrls.at(i).url());
        url.appendChild(text);
        url.setAttribute("numParallelSegments", m_unusedConnections.at(i));
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
    qDebug() << m_size << m_segSize << m_dest.isValid() << !m_sources.isEmpty();
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
    Transfer::Capabilities newCaps = {};

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
        Q_EMIT capabilitiesChanged();
    }
}


