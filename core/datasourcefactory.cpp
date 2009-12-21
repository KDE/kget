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
#include "kiodownload.h"
#include "settings.h"

#include "core/kget.h"
#include "core/verifier.h"

#include <math.h>

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QVarLengthArray>
#include <QtXml/QDomText>

#include <KIO/DeleteJob>
#include <KIO/FileJob>
#include <KIO/NetAccess>
#include <KLocale>
#include <KMessageBox>

#include <KDebug>


#include <kde_file.h>

const int SPEEDTIMER = 1000;//1 second...

DataSourceFactory::DataSourceFactory(const KUrl &dest, KIO::filesize_t size, KIO::fileoffset_t segSize,
                                     QObject *parent)
  : QObject(parent),
    m_dest(dest),
    m_size(size),
    m_downloadedSize(0),
    m_segSize(segSize),
    m_speed(0),
    m_tempOffset(0),
    m_startedChunks(0),
    m_finishedChunks(0),
    m_putJob(0),
    m_doDownload(true),
    m_open(false),
    m_blocked(false),
    m_startTried(false),
    m_assignTried(false),
    m_movingFile(false),
    m_finished(false),
    m_maxMirrorsUsed(3),
    m_speedTimer(0),
    m_tempDownload(0),
    m_status(Job::Stopped),
    m_statusBeforeMove(m_status),
    m_verifier(0),
    m_signature(0)
{
    kDebug(5001) << "Initialize DataSourceFactory: Dest: " + m_dest.url() + "Size: " + QString::number(m_size) + "SegSize: " + QString::number(m_segSize);

    m_prevDownloadedSizes.append(0);
}

DataSourceFactory::DataSourceFactory(QObject *parent)
  : QObject(parent),
    m_size(0),
    m_downloadedSize(0),
    m_segSize(0),
    m_speed(0),
    m_tempOffset(0),
    m_startedChunks(0),
    m_finishedChunks(0),
    m_putJob(0),
    m_doDownload(true),
    m_open(false),
    m_blocked(false),
    m_startTried(false),
    m_assignTried(false),
    m_movingFile(false),
    m_finished(false),
    m_maxMirrorsUsed(3),
    m_speedTimer(0),
    m_tempDownload(0),
    m_status(Job::Stopped),
    m_statusBeforeMove(m_status),
    m_verifier(0),
    m_signature(0)
{
    kDebug(5001) << "Initialize DataSourceFactory only with parrent";

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

    //create all dirs needed
    QDir dir;
    dir.mkpath(m_dest.directory());
}

void DataSourceFactory::deinit()
{
    if (m_dest.isLocalFile())
    {
        KIO::Job *del = KIO::del(m_dest, KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }
}

void DataSourceFactory::findFileSize()
{
    kDebug(5001) << "Find the filesize";
    if (!m_size && !m_dest.isEmpty() && !m_tempDownload)
    {
        foreach (TransferDataSource *source, m_sources)
        {
            const KUrl url = source->sourceUrl();
            QString prot = url.protocol();
            if ((prot == "http" || prot == "https" || prot == "ftp"  || prot == "sftp") &&
                (!url.fileName().endsWith(QLatin1String(".torrent"))) &&
                (!url.fileName().endsWith(QLatin1String(".metalink"))) &&
                (!url.fileName().endsWith(QLatin1String(".meta4"))))
            {
                m_tempDownload = new KioDownload(url, m_dest, this);
                connect(m_tempDownload, SIGNAL(processedSize(KIO::filesize_t)), this, SIGNAL(processedSize(KIO::filesize_t)));
                connect(m_tempDownload, SIGNAL(speed(ulong)), this, SIGNAL(speed(ulong)));
                connect(m_tempDownload, SIGNAL(totalSize(KIO::filesize_t)), this, SLOT(sizeFound(KIO::filesize_t)));
                connect(m_tempDownload, SIGNAL(finished()), this, SLOT(finished()));
                m_tempDownload->start();
                changeStatus(Job::Running);
                break;
            }
        }
    }
}


void DataSourceFactory::sizeFound(KIO::filesize_t size)
{
    m_size = size;
    kDebug(5001) << "Size found: " << m_size;
    emit totalSize(m_size);

    init();

    //use what the tempDownload has downloaded if it is larger than a segment
    m_tempDownload->stop();
    const quint32 segmentsDownloaded = m_tempDownload->processedSize() / m_segSize;
    if (segmentsDownloaded)
    {
        kDebug(5001) << "Segments reused of the tempDownload: " << segmentsDownloaded;

        m_downloadedSize = segmentsDownloaded * m_segSize;
        for (quint32 i = 0; i < segmentsDownloaded; ++i)
        {
            m_startedChunks->set(i, true);
            m_finishedChunks->set(i, true);
        }
    }

    delete m_tempDownload;
    m_tempDownload = 0;

    if (m_startTried)
    {
        start();
    }
}

void DataSourceFactory::finished()
{
    m_downloadedSize = m_tempDownload->processedSize();

    delete m_tempDownload;
    m_tempDownload = 0;

    changeStatus(Job::Finished);

    //set all chunks to true, that is useful for saving
    init();
    m_startedChunks->setAll(true);
    m_finishedChunks->setAll(true);
}

bool DataSourceFactory::checkLocalFile()
{
    QString dest_orig = m_dest.path();
    QByteArray _dest_part(QFile::encodeName(dest_orig));

    KDE_struct_stat buff_part;
    bool bPartExists = (KDE_stat( _dest_part.data(), &buff_part ) != -1);
    if(!bPartExists)
    {
        QByteArray _dest = QFile::encodeName(dest_orig);
        int fd = -1;
        mode_t initialMode = 0666;

        fd = KDE_open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
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
    if (!m_size)
    {
        m_startTried = true;
        findFileSize();
        return;
    }

    init();

    if (m_assignTried) {
        m_assignTried = false;

        //TODO maybe remove this later, if loading works without it
        if (!m_startedChunks->allOn()) {
            foreach(TransferDataSource *source, m_sources) {
                assignSegments(source);
            }
        }
    }

    if (checkLocalFile())
    {
        if (!m_putJob)
        {
            QFile::resize(m_dest.pathOrUrl(), m_size);//TODO should we keep that?
            m_putJob = KIO::open(m_dest, QIODevice::WriteOnly | QIODevice::ReadOnly);
            connect(m_putJob, SIGNAL(open(KIO::Job*)), this, SLOT(open(KIO::Job*)));
            m_startTried = true;
            return;
        }
        if (m_open) {
            m_speedTimer->start();

            foreach (TransferDataSource *source, m_sources) {
                source->start();
            }

            m_startTried = false;
            changeStatus(Job::Running);
        }
    }
}

void DataSourceFactory::open(KIO::Job *job)
{
    Q_UNUSED(job)
    kDebug(5001) << "File opened.";

    if (!m_speedTimer)
    {
        init();
    }

    connect(m_putJob, SIGNAL(position(KIO::Job*,KIO::filesize_t)), this, SLOT(slotOffset(KIO::Job*,KIO::filesize_t)));
    connect(m_putJob, SIGNAL(written(KIO::Job*, KIO::filesize_t)), this, SLOT(slotDataWritten(KIO::Job*,KIO::filesize_t)));
    m_open = true;

    if (m_startTried)
    {
        start();
    }
}

void DataSourceFactory::stop()
{
    kDebug(5001) << "Stopping";
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
    changeStatus(Job::Stopped);
}

void DataSourceFactory::setDoDownload(bool doDownload)
{
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
                    if ((start != -1) && (end != -1))
                    {
                        for (int k = start; k <= end; ++k)
                        {
                            m_startedChunks->set(k, false);
                        }
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
                    kDebug(5001) << "Successfully created a TransferDataSource for " << url.pathOrUrl();

                    //url might have been an unused Mirror, so remove it in any case
                    const int index = m_unusedUrls.indexOf(url);
                    if (index > -1 )
                    {
                        m_unusedUrls.removeAt(index);
                        m_unusedConnections.removeAt(index);
                    }

                    m_sources[url] = source;
                    m_sources[url]->setParalellSegments(numParalellConnections);

                    connect(source, SIGNAL(brokenSegments(TransferDataSource*,QPair<int, int>)), this, SLOT(brokenSegments(TransferDataSource*,QPair<int, int>)));
                    connect(source, SIGNAL(finishedSegment(TransferDataSource*,int,bool)), this, SLOT(finishedSegment(TransferDataSource*,int,bool)));
                    connect(source, SIGNAL(data(KIO::fileoffset_t, const QByteArray&, bool&)), this, SLOT(slotWriteData(KIO::fileoffset_t, const QByteArray&, bool&)));
                    connect(source, SIGNAL(freeSegments(TransferDataSource*,QPair<int,int>)), this, SLOT(slotFreeSegments(TransferDataSource*,QPair<int,int>)));

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
            if ((start != -1) && (end != -1))
            {
                for (int k = start; k <= end; ++k)
                {
                    kDebug(5001) << "Segment" << k << "not assigned anymore.";
                    m_startedChunks->set(k, false);
                }
            }
        }
    }

    //see if mirrors need to be assigned, e.g. the broken segment was the last one
    if (m_status == Job::Running)
    {
        bool assignNeeded = true;
        QHash<KUrl, TransferDataSource*>::const_iterator it;
        QHash<KUrl, TransferDataSource*>::const_iterator itEnd = m_sources.constEnd();
        for (it = m_sources.constBegin(); it != itEnd; ++it)
        {
            if ((*it)->currentSegments())
            {
                //at least one TransferDataSource is still running, so no assign needed
                assignNeeded = false;
                break;
            }
        }

        if (assignNeeded)
        {
            if (m_sources.count())
            {
                kDebug(5001) << "Assigning a TransferDataSource.";
                //simply assign a TransferDataSource
                assignSegments(*m_sources.begin());
            }
            else if (m_unusedUrls.count())
            {
                kDebug(5001) << "Assigning an unused mirror";
                //takes the first unused mirror
                addMirror(m_unusedUrls.takeFirst(), true, m_unusedConnections.takeFirst());
            }
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

void DataSourceFactory::brokenSegments(TransferDataSource *source, const QPair<int, int> &segmentRange)
{
    kDebug(5001) << "Segments" << segmentRange << "broken," << source;
    if (!source || (segmentRange.first < 0) || (segmentRange.second < 0) || (static_cast<quint32>(segmentRange.second) > m_finishedChunks->getNumBits()))
    {
        return;
    }

    removeMirror(source->sourceUrl());
}


void DataSourceFactory::broken(TransferDataSource *source, TransferDataSource::Error error)
{
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
        for (int i = start; i <= end; ++i) {
            kDebug(5001) << "Segment" << i << "not assigned anymore.";
            m_startedChunks->set(i, false);
        }
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

        if (source->canHandleMultipleSegments()) {
            const QPair<int, int> splitResult = target->split();
            newStart = splitResult.first;
            newEnd = splitResult.second;
        } else {
            newStart = target->takeOneSegment();
            newEnd = newStart;
        }
    }
    //still some free segments
    else
    {
        //avoid looking through all bits if not nescessary
        if (m_startedChunks->allOff())
        {
            newStart = 0;

            if (source->canHandleMultipleSegments())
            {
                newEnd = m_startedChunks->getNumBits() - 1;
            }
            else
            {
                newEnd = newStart;
            }
        }
        else
        {
            for (uint i = 0; i < m_startedChunks->getNumBits(); ++i)
            {
                //we only choose chunks that are not started and finished
                if (!m_startedChunks->get(i) && !m_finishedChunks->get(i))
                {
                    //first result is assigned to newStart and newEnd
                    if (newStart == -1)
                    {
                        newStart = i;
                        newEnd = i;

                        if (!source->canHandleMultipleSegments())
                        {
                            break;
                        }
                    }
                    //start already found, now find the end
                    else
                    {
                        newEnd = i;
                    }
                }
                //not continuous
                else if (newStart != -1)
                {
                    break;
                }
            }
        }
    }

    if ((newStart == -1) || (newEnd == -1))
    {
        kDebug(5001) << "No segment can be assigned.";
        return;
    }

    const KIO::fileoffset_t newoff = KIO::fileoffset_t(newStart * m_segSize);
    const KIO::fileoffset_t rest = m_size % m_segSize;

    //if newStart is the last segment of the download and there is a rest, when segSize is rest
    const KIO::fileoffset_t segSize = ((static_cast<uint>(newStart + 1) == m_startedChunks->getNumBits()) && rest) ? rest : m_segSize;

    //the lastSegsize is rest, but only if there is a rest and it is the last segment of the download
    const KIO::fileoffset_t lastSegSize = ((static_cast<uint>(newEnd + 1) == m_startedChunks->getNumBits() && rest) ? rest : m_segSize);

    kDebug(5001) << "Segments assigned:" << newStart << "-" << newEnd << "offset:" << newoff << "segment-size:" << segSize << "rest:" << rest;

    for (int i = newStart; i <= newEnd; ++i)
    {
        m_startedChunks->set(i, true);
    }
    source->addSegments(newoff, qMakePair(segSize, lastSegSize), qMakePair(newStart, newEnd));

    //there should still be segments added to this transfer
    if (source->changeNeeded() > 0) {
        assignSegments(source);
    }
}
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
        emit processedSize((m_downloadedSize));
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
    emit percent(p);
}

void DataSourceFactory::speedChanged()
{
    m_speed = (m_downloadedSize - m_prevDownloadedSizes.first()) / (SPEEDTIMER * m_prevDownloadedSizes.size() / 1000);//downloaded in 1 second

    m_prevDownloadedSizes.append(m_downloadedSize);
    if(m_prevDownloadedSizes.size() > 10)
        m_prevDownloadedSizes.removeFirst();

    emit speed(m_speed);
    emit percent(m_downloadedSize * 100 / m_size);
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

bool DataSourceFactory::setNewDestination(const KUrl &newDestination)
{
    m_newDest = newDestination;
    if (m_newDest.isValid() && (m_newDest != m_dest))
    {
        if (QFile::exists(m_dest.pathOrUrl()))
        {
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
    connect(move, SIGNAL(result(KJob *)), this, SLOT(newDestResult(KJob *)));
    connect(move, SIGNAL(percent(KJob*, unsigned long)), this, SLOT(slotPercent(KJob*, ulong)));

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
    verifier()->brokenPieces();
}

void DataSourceFactory::slotRepair(const QList<QPair<KIO::fileoffset_t, KIO::filesize_t> > &brokenPieces)
{
    if (brokenPieces.isEmpty())
    {
        kDebug(5001) << "Redownload everything";
        m_startedChunks->clear();
        m_finishedChunks->clear();
    }
    else
    {
        kDebug(5001) << "Redownload broken pieces";
        QList<QPair<KIO::fileoffset_t, KIO::filesize_t> >::const_iterator it;
        QList<QPair<KIO::fileoffset_t, KIO::filesize_t> >::const_iterator itEnd = brokenPieces.constEnd();
        for (it = brokenPieces.constBegin(); it != itEnd; ++it)
        {
            const quint32 startSegment = (*it).first / m_segSize;
            const quint32 endSegment = ceil((*it).second / static_cast<double>(m_segSize)) - 1 + startSegment;
            for (quint32 i = startSegment; i <= endSegment; ++i)
            {
                m_startedChunks->set(i, false);
                m_finishedChunks->set(i, false);
            }
        }
    }

    //remove all current mirrors and readd the first unused mirror
    const QList<KUrl> mirrors = m_sources.keys();//TODO only remove the mirrors of the broken segments?! --> for that m_assignedChunks would need to be saved was well
    foreach (const KUrl &mirror, mirrors)
    {
        removeMirror(mirror);
    }
    addMirror(m_unusedUrls.takeFirst(), true, m_unusedConnections.takeFirst());

    m_downloadedSize = m_segSize * m_finishedChunks->numOnBits();
    m_prevDownloadedSizes.clear();
    if (m_downloadedSize) {
        m_prevDownloadedSizes.append(m_downloadedSize);
    }

    changeStatus(Job::Stopped, true);
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

    if (!m_size)
    {
        m_size = e.attribute("size").toULongLong();
    }
    if (!m_segSize)
    {
        m_segSize = e.attribute("segementSize").toLongLong();
    }
    if (!m_downloadedSize)
    {
        m_downloadedSize = e.attribute("processedSize").toULongLong();
    }
    if (e.hasAttribute("doDownload"))
    {
        m_doDownload = QVariant(e.attribute("doDownload")).toBool();
    }
    if (e.hasAttribute("maxMirrorsUsed"))
    {
        bool worked;
        m_maxMirrorsUsed = e.attribute("maxMirrorsUsed").toInt(&worked);
        m_maxMirrorsUsed = (worked ? m_maxMirrorsUsed : 3);
    }

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
            //adapt the downloadedSize to what not has to be downloaded
            //that might differ to what has been download, e.g. if a chunk did not finish before closing Kget
            if (m_segSize)
            {
                m_downloadedSize = m_segSize * m_finishedChunks->numOnBits();
                kDebug(5001) << "Modified download size: " << m_downloadedSize;
            }
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

    changeStatus(static_cast<Job::Status>(e.attribute("status").toInt()), true);
}

void DataSourceFactory::changeStatus(Job::Status status, bool loaded)
{
    m_status = status;
    switch (m_status)
    {
        case Job::Stopped:
            m_speed = 0;
            emit speed(m_speed);
            break;
        case Job::Running:
            break;
        case Job::Moving:
            m_speed = 0;
            emit speed(m_speed);
            break;
        case Job::Finished:
            m_speed = 0;

            if (m_size)
            {
                m_downloadedSize = m_size;
                emit processedSize(m_downloadedSize);
            }
            else if (m_downloadedSize)
            {
                m_size = m_downloadedSize;
                emit totalSize(m_size);
            }

            emit speed(0);
            emit percent(100);

            if (!loaded) {
                if (Settings::checksumAutomaticVerification() && verifier()->isVerifyable()) {
                    verifier()->verify();
                }
                if (Settings::signatureAutomaticVerification() && signature()->isVerifyable()) {
                    signature()->verify();
                }
            }
            break;
        default:
            //TODO handle Delayed and Aborted
            break;
    }

    //load has been called, so change the totalSize, percent and the processedSize if possible
    if (loaded)
    {
        if (m_size)
        {
            emit totalSize(m_size);
            if (m_downloadedSize)
            {
                emit processedSize(m_downloadedSize);
                emit percent((m_downloadedSize * 100) / m_size);
            }
        }

        //Do not emit a status when loading as it would not reflect the current status, but rather the
        //status the program was ended with
        return;
    }

    emit statusChanged(m_status);
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

    if (!m_finishedChunks)
    {
        factory.setAttribute("processedSize", m_downloadedSize);
    }
    factory.setAttribute("size", m_size);
    factory.setAttribute("segementSize", m_segSize);
    factory.setAttribute("status", m_status);
    factory.setAttribute("doDownload", m_doDownload);
    factory.setAttribute("maxMirrorsUsed", m_maxMirrorsUsed);

    verifier()->save(factory);
    signature()->save(factory);

    //set the finished chunks
    if (m_finishedChunks)
    {
        //not m_downloadedSize is stored, but the bytes that do not have to be redownloaded
        factory.setAttribute("processedSize", m_segSize * m_finishedChunks->numOnBits());

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

KIO::filesize_t DataSourceFactory::size() const
{
    return m_size;
}

KIO::filesize_t DataSourceFactory::downloadedSize() const
{
    return m_downloadedSize;
}

ulong DataSourceFactory::currentSpeed() const
{
    return m_speed;
}

KUrl DataSourceFactory::dest() const
{
    return m_dest;
}

int DataSourceFactory::maxMirrorsUsed() const
{
    return m_maxMirrorsUsed;
}

void DataSourceFactory::setMaxMirrorsUsed(int maxMirrorsUsed)
{
    m_maxMirrorsUsed = maxMirrorsUsed;
}

bool DataSourceFactory::doDownload() const
{
    return m_doDownload;
}

Job::Status DataSourceFactory::status() const
{
    return m_status;
}

Verifier *DataSourceFactory::verifier()
{
    if (!m_verifier) {
        m_verifier = new Verifier(m_dest, this);
        connect(m_verifier, SIGNAL(brokenPieces(QList<QPair<KIO::fileoffset_t,KIO::filesize_t> >)), this, SLOT(slotRepair(QList<QPair<KIO::fileoffset_t,KIO::filesize_t> >)));
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

#include "datasourcefactory.moc"
