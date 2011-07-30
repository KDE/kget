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
#include "core/transfer.h"

#include <KDebug>

MultiSegKioDataSource::MultiSegKioDataSource(const KUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent),
    m_size(0),
    m_canResume(false),
    m_started(false)
{
    kDebug(5001) << "Create MultiSegKioDataSource for" << m_sourceUrl << this;
    setCapabilities(capabilities() | Transfer::Cap_FindFilesize);
}

MultiSegKioDataSource::~MultiSegKioDataSource()
{
    kDebug(5001) << this;
}

void MultiSegKioDataSource::start()
{
    kDebug(5001) << this;

    m_started = true;
    foreach (Segment *segment, m_segments)
    {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::stop()
{
    kDebug(5001) << this << m_segments.count() << "segments stopped.";

    m_started = false;
    foreach (Segment *segment, m_segments)
    {
        if (segment->findingFileSize()) {
            kDebug(5001) << "Removing findingFileSize segment" << this;
            m_segments.removeAll(segment);
            segment->deleteLater();
        } else {
            segment->stopTransfer();
        }
    }
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

void MultiSegKioDataSource::addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    Segment *segment = new Segment(m_sourceUrl, segmentSize, segmentRange, this);
    m_segments.append(segment);

    connect(segment, SIGNAL(canResume()), this, SLOT(slotCanResume()));
    connect(segment, SIGNAL(totalSize(KIO::filesize_t,QPair<int,int>)), this, SLOT(slotTotalSize(KIO::filesize_t,QPair<int,int>)));
    connect(segment, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)), this, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)));
    connect(segment, SIGNAL(finishedSegment(Segment*,int,bool)), this, SLOT(slotFinishedSegment(Segment*,int,bool)));
    connect(segment, SIGNAL(error(Segment*,QString,Transfer::LogLevel)), this, SLOT(slotError(Segment*,QString,Transfer::LogLevel)));
    connect(segment, SIGNAL(finishedDownload(KIO::filesize_t)), this, SLOT(slotFinishedDownload(KIO::filesize_t)));

    if (m_started) {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::findFileSize(KIO::fileoffset_t segmentSize)
{
    addSegments(qMakePair(segmentSize, segmentSize), qMakePair(-1, -1));
    Segment *segment = m_segments.last();
    segment->startTransfer();
}

void MultiSegKioDataSource::slotSpeed(ulong downloadSpeed)
{
    m_speed = downloadSpeed;
    emit speed(m_speed);
}

void MultiSegKioDataSource::slotFinishedSegment(Segment *segment, int segmentNum, bool connectionFinished)
{
    if (connectionFinished) {
        m_segments.removeAll(segment);
        segment->deleteLater();
    }
    emit finishedSegment(this, segmentNum, connectionFinished);
}

void MultiSegKioDataSource::setSupposedSize(KIO::filesize_t supposedSize)
{
    m_supposedSize = supposedSize;

    //check if the size is correct
    slotTotalSize(m_size);
}

void MultiSegKioDataSource::slotTotalSize(KIO::filesize_t size, const QPair<int, int> &range)
{
    kDebug(5001) << "Size found for" << m_sourceUrl << size << "bytes";

    m_size = size;

    //findFileSize was called
    if ((range.first != -1) && (range.second != -1)) {
        emit foundFileSize(this, size, range);
    }

    //the filesize is not what it should be, maybe using a wrong mirror
    if (m_size && m_supposedSize && (m_size != m_supposedSize))
    {
        kDebug(5001) << "Size does not match for" << m_sourceUrl << this;
        emit broken(this, WrongDownloadSize);
    }
}

void MultiSegKioDataSource::slotCanResume()
{
    kDebug(5001) << this;

    if (!m_canResume) {
        m_canResume = true;
        setCapabilities(capabilities() | Transfer::Cap_Resuming);
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
        seg->deleteLater();
    }

    return unassigned;
}

bool MultiSegKioDataSource::tryMerge(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    foreach (Segment *segment, m_segments) {
        if (segment->merge(segmentSize, segmentRange)) {
            return true;
        }
    }

    return false;
}

void MultiSegKioDataSource::slotError(Segment *segment, const QString &errorText, Transfer::LogLevel logLevel)
{
    kDebug(5001) << "Error" << errorText << "segment" << segment;

    const QPair<KIO::fileoffset_t, KIO::fileoffset_t> size = segment->segmentSize();
    const QPair<int, int> range = segment->assignedSegments();
    m_segments.removeAll(segment);
    segment->deleteLater();

    emit log(errorText, logLevel);
    if (m_segments.isEmpty()) {
        kDebug(5001) << this << "has broken segments.";
        emit brokenSegments(this, range);
    } else {
        //decrease the number of maximum paralell downloads, maybe the server does not support so many connections
        if (m_paralellSegments > 1) {
            --m_paralellSegments;
        }
        kDebug(5001) << this << "reducing connections to" << m_paralellSegments << "and freeing range of semgents" << range;
        if (!tryMerge(size, range)) {
            emit freeSegments(this, range);
        }
    }
}

void MultiSegKioDataSource::slotFinishedDownload(KIO::filesize_t size)
{
    stop();
    emit finishedDownload(this, size);
}

void MultiSegKioDataSource::slotRestartBrokenSegment()
{
    kDebug(5001) << this;
    start();
}


#include "multisegkiodatasource.moc"
