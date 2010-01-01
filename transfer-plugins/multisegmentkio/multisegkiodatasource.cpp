/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "multisegkiodatasource.h"
#include "segment.h"

#include <KDebug>

MultiSegKioDataSource::MultiSegKioDataSource(const KUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent),
    m_size(0),
    m_canResume(false),
    m_getInitJob(0),
    m_hasInitJob(false),
    m_started(false),
    m_restarted(0)
{
    kDebug(5001);
}

MultiSegKioDataSource::~MultiSegKioDataSource()
{
    kDebug(5001);

    killInitJob();
}

void MultiSegKioDataSource::start()
{
    kDebug(5001);

    if (!m_hasInitJob && !m_getInitJob)
    {
        m_hasInitJob = true;
        //check what size this server reports and if it is resumeable, so we do not
        //have to do that for every Segment
        m_getInitJob = KIO::get(m_sourceUrl, KIO::Reload, KIO::HideProgressInfo);
        m_getInitJob->suspend();
        m_getInitJob->addMetaData("errorPage", "false");
        m_getInitJob->addMetaData("AllowCompressedPage", "false");
        m_getInitJob->addMetaData("resume", KIO::number(1) );
        connect(m_getInitJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)), SLOT(slotCanResume(KIO::Job *, KIO::filesize_t)));
        connect(m_getInitJob, SIGNAL(data(KIO::Job *, const QByteArray&)), this, SLOT(slotInitData(KIO::Job *, const QByteArray&)));
        connect(m_getInitJob, SIGNAL(totalSize(KJob *, qulonglong)), this, SLOT(slotTotalSize(KJob*, qulonglong)));
        connect(m_getInitJob, SIGNAL(result(KJob *)), this, SLOT(slotInitResult(KJob *)));
    }

    m_started = true;
    foreach (Segment *segment, m_segments)
    {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::stop()
{
    kDebug(5001);

    m_started = false;
    foreach (Segment *segment, m_segments)
    {
        segment->stopTransfer();
    }
}

bool MultiSegKioDataSource::canHandleMultipleSegments() const
{
    return true;
}

QList<QPair<int, int> > MultiSegKioDataSource::assignedSegments() const
{
    QList<QPair<int, int> > assigned;
    foreach (Segment *segment, m_segments)
    {
        assigned.append(segment->assignedSegments());
    }

    return assigned;
}

void MultiSegKioDataSource::addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum)
{
    kDebug(5001);
    addSegments(offset, qMakePair(bytes, bytes), qMakePair(segmentNum, segmentNum));
}

void MultiSegKioDataSource::addSegments(const KIO::fileoffset_t offset, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    Segment *segment = new Segment(m_sourceUrl, offset, segmentSize, segmentRange, this);
    m_segments.append(segment);

    connect(segment, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)), this, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)));
    connect(segment, SIGNAL(finishedSegment(Segment*, int, bool)), this, SLOT(slotFinishedSegment(Segment*, int, bool)));
    connect(segment, SIGNAL(brokenSegments(Segment*,QPair<int,int>)), this, SLOT(slotBrokenSegments(Segment*,QPair<int,int>)));
    connect(segment, SIGNAL(error(Segment*,int)), SLOT(slotError(Segment*,int)));

    if (m_started) {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::slotSpeed(ulong downloadSpeed)
{
    m_speed = downloadSpeed;
    emit speed(m_speed);
}

void MultiSegKioDataSource::slotBrokenSegments(Segment *segment, const QPair<int,int> &segmentRange)
{
    Q_UNUSED(segment)

    m_segments.removeAll(segment);
    delete segment;

    emit brokenSegments(this, segmentRange);
}

void MultiSegKioDataSource::slotFinishedSegment(Segment *segment, int segmentNum, bool connectionFinished)
{
    if (connectionFinished) {
        m_segments.removeAll(segment);
        delete segment;
    }
    emit finishedSegment(this, segmentNum, connectionFinished);
}

void MultiSegKioDataSource::setSupposedSize(KIO::filesize_t supposedSize)
{
    m_supposedSize = supposedSize;

    //check if the size is correct
    slotTotalSize(0, m_size);
}

void MultiSegKioDataSource::slotTotalSize(KJob *job, qulonglong size)
{
    Q_UNUSED(job)

    kDebug(5001) << "Size found for" << m_sourceUrl << size << "bytes";

    m_size = size;

    if (m_canResume)
    {
        killInitJob();
    }

    //the filesize is not what it should be, maybe using a wrong mirror
    if (m_size && m_supposedSize && (m_size != m_supposedSize))
    {
        kDebug(5001) << "Size does not match for" << m_sourceUrl;
        emit broken(this, WrongDownloadSize);
    }
}

void MultiSegKioDataSource::slotInitData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)
    static KIO::filesize_t downloaded = 0;
    downloaded += static_cast<KIO::filesize_t>(data.size());
    if ((downloaded > 100 * 1024))
    {
        killInitJob();
    }
}

void MultiSegKioDataSource::slotCanResume(KIO::Job *job, KIO::filesize_t offset)
{
    Q_UNUSED(job)
    Q_UNUSED(offset)

    kDebug(5001);

    m_canResume = true;
    if (m_size)
    {
       killInitJob();
    }
}

void MultiSegKioDataSource::slotInitResult(KJob *job)
{
    kDebug(5001) << "slotInitResult  (" << job->error() << ")";
    m_getInitJob = 0;
}

void MultiSegKioDataSource::killInitJob()
{
    if (m_getInitJob)
    {
        m_getInitJob->kill(KJob::Quietly);
        m_getInitJob = 0;
    }
}

int MultiSegKioDataSource::currentSegments() const
{
    return m_segments.count();
}

Segment *MultiSegKioDataSource::mostUnfinishedSegments(int *unfin) const
{
    int unfinished = 0;
    Segment *seg = 0;
    foreach (Segment *segment, m_segments)
    {
        if (segment->countUnfinishedSegments() > unfinished)
        {
            unfinished = segment->countUnfinishedSegments();
            seg = segment;
        }
    }

    if (unfin)
    {
        *unfin = unfinished;
    }

    return seg;
}

int MultiSegKioDataSource::countUnfinishedSegments() const
{
    int unfinished = 0;
    mostUnfinishedSegments(&unfinished);

    return unfinished;
}

int MultiSegKioDataSource::takeOneSegment()
{
    int unassigned = -1;
    Segment *seg = mostUnfinishedSegments();
    if (seg) {
        unassigned = seg->takeOneSegment();
    }

    return unassigned;
}

QPair<int, int> MultiSegKioDataSource::split()
{
    QPair<int, int> unassigned = qMakePair(-1, -1);
    Segment *seg = mostUnfinishedSegments();
    if (seg) {
        unassigned = seg->split();
    }

    return unassigned;
}

QPair<int, int> MultiSegKioDataSource::removeConnection()
{
    QPair<int, int> unassigned = qMakePair(-1, -1);
    Segment *seg = mostUnfinishedSegments();
    if (seg) {
        unassigned = seg->assignedSegments();
        m_segments.removeAll(seg);
        delete seg;
    }

    return unassigned;
}


void MultiSegKioDataSource::slotError(Segment *segment, int KIOError)
{
    Q_UNUSED(KIOError)

    kDebug(5001) << "Error" << KIOError << "segment" << segment;

    const KIO::fileoffset_t offset = segment->offset();
    const QPair<KIO::fileoffset_t, KIO::fileoffset_t> segmentSize = segment->segmentSize();
    const QPair<int, int> range = segment->assignedSegments();

    m_segments.removeAll(segment);
    delete segment;

    if (m_segments.isEmpty()) {
        //empty, retry it three times
        if (m_restarted < 3) {
            ++m_restarted;
            addSegments(offset, segmentSize, range);
        } else {
            emit brokenSegments(this, range);
        }
    } else {
        //decrease the number of maximum paralell downloads, maybe the server does not support so many connections
        if (m_paralellSegments > 1) {
            --m_paralellSegments;
        }
        emit freeSegments(this, range);
    }
}


#include "multisegkiodatasource.moc"
