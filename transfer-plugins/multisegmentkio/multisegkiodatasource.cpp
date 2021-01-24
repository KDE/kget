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

#include "kget_debug.h"
#include <QDebug>

MultiSegKioDataSource::MultiSegKioDataSource(const QUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent),
    m_size(0),
    m_canResume(false),
    m_started(false)
{
    qCDebug(KGET_DEBUG) << "Create MultiSegKioDataSource for" << m_sourceUrl << this;
    setCapabilities(capabilities() | Transfer::Cap_FindFilesize);
}

MultiSegKioDataSource::~MultiSegKioDataSource()
{
    qCDebug(KGET_DEBUG) << this;
}

void MultiSegKioDataSource::start()
{
    qCDebug(KGET_DEBUG) << this;

    m_started = true;
    foreach (Segment *segment, m_segments)
    {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::stop()
{
    qCDebug(KGET_DEBUG) << this << m_segments.count() << "segments stopped.";

    m_started = false;
    foreach (Segment *segment, m_segments)
    {
        if (segment->findingFileSize()) {
            qCDebug(KGET_DEBUG) << "Removing findingFileSize segment" << this;
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
    auto *segment = new Segment(m_sourceUrl, segmentSize, segmentRange, this);
    m_segments.append(segment);

    connect(segment, &Segment::canResume, this, &MultiSegKioDataSource::slotCanResume);
    connect(segment, SIGNAL(totalSize(KIO::filesize_t,QPair<int,int>)), this, SLOT(slotTotalSize(KIO::filesize_t,QPair<int,int>)));
    connect(segment, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)), this, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)));
    connect(segment, &Segment::finishedSegment, this, &MultiSegKioDataSource::slotFinishedSegment);
    connect(segment, &Segment::error, this, &MultiSegKioDataSource::slotError);
    connect(segment, &Segment::finishedDownload, this, &MultiSegKioDataSource::slotFinishedDownload);
    connect(segment, &Segment::urlChanged, this, &MultiSegKioDataSource::slotUrlChanged);

    if (m_started) {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::slotUrlChanged(const QUrl &url)
{
    if (m_sourceUrl != url) {
        Q_EMIT urlChanged(m_sourceUrl, url);
        m_sourceUrl = url;
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
    Q_EMIT speed(m_speed);
}

void MultiSegKioDataSource::slotFinishedSegment(Segment *segment, int segmentNum, bool connectionFinished)
{
    if (connectionFinished) {
        m_segments.removeAll(segment);
        segment->deleteLater();
    }
    Q_EMIT finishedSegment(this, segmentNum, connectionFinished);
}

void MultiSegKioDataSource::setSupposedSize(KIO::filesize_t supposedSize)
{
    m_supposedSize = supposedSize;

    //check if the size is correct
    slotTotalSize(m_size);
}

void MultiSegKioDataSource::slotTotalSize(KIO::filesize_t size, const QPair<int, int> &range)
{
    qCDebug(KGET_DEBUG) << "Size found for" << m_sourceUrl << size << "bytes";

    m_size = size;

    //findFileSize was called
    if ((range.first != -1) && (range.second != -1)) {
        Q_EMIT foundFileSize(this, size, range);
    }

    //the filesize is not what it should be, maybe using a wrong mirror
    if (m_size && m_supposedSize && (m_size != m_supposedSize))
    {
        qCDebug(KGET_DEBUG) << "Size does not match for" << m_sourceUrl << this;
        Q_EMIT broken(this, WrongDownloadSize);
    }
}

void MultiSegKioDataSource::slotCanResume()
{
    qCDebug(KGET_DEBUG) << this;

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
    Segment *seg = nullptr;
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
    qCDebug(KGET_DEBUG) << "Error" << errorText << "segment" << segment;

    const QPair<KIO::fileoffset_t, KIO::fileoffset_t> size = segment->segmentSize();
    const QPair<int, int> range = segment->assignedSegments();
    m_segments.removeAll(segment);
    segment->deleteLater();

    Q_EMIT log(errorText, logLevel);
    if (m_segments.isEmpty()) {
        qCDebug(KGET_DEBUG) << this << "has broken segments.";
        Q_EMIT brokenSegments(this, range);
    } else {
        //decrease the number of maximum parallel downloads, maybe the server does not support so many connections
        if (m_parallelSegments > 1) {
            --m_parallelSegments;
        }
        qCDebug(KGET_DEBUG) << this << "reducing connections to" << m_parallelSegments << "and freeing range of segments" << range;
        if (!tryMerge(size, range)) {
            Q_EMIT freeSegments(this, range);
        }
    }
}

void MultiSegKioDataSource::slotFinishedDownload(KIO::filesize_t size)
{
    stop();
    Q_EMIT finishedDownload(this, size);
}

void MultiSegKioDataSource::slotRestartBrokenSegment()
{
    qCDebug(KGET_DEBUG) << this;
    start();
}



