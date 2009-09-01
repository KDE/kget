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

MultiSegKioDataSource::MultiSegKioDataSource(const KUrl &srcUrl)
  : TransferDataSource(srcUrl, 0),
    m_size(0),
    m_canResume(false),
    m_getInitJob(0),
    m_hasInitJob(false),
    m_started(false)
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

void MultiSegKioDataSource::addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum)
{
    kDebug(5001);

    Segment *segment = new Segment(m_sourceUrl, offset, bytes, segmentNum, this);
    m_segments.append(segment);

    connect(segment, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)), this, SIGNAL(data(KIO::fileoffset_t,QByteArray,bool&)));
    connect(segment, SIGNAL(finishedSegment(Segment*, int)), this, SLOT(slotFinishedSegment(Segment*, int)));
    connect(segment, SIGNAL(brokenSegment(Segment*,int)), this, SLOT(slotBrokenSegment(Segment*,int)));

    if (m_started)
    {
        segment->startTransfer();
    }
}

void MultiSegKioDataSource::slotSpeed(ulong downloadSpeed)
{
    m_speed = downloadSpeed;
    emit speed(m_speed);
}

void MultiSegKioDataSource::slotBrokenSegment(Segment *segment, int segmentNum)
{
    m_segments.removeAll(segment);
    delete segment;
    emit brokenSegment(this, segmentNum);
}

void MultiSegKioDataSource::slotFinishedSegment(int segmentNum)
{
    emit finishedSegment(this, segmentNum);
}

void MultiSegKioDataSource::slotFinishedSegment(Segment *segment, int segmentNum)
{
    m_segments.removeAll(segment);
    delete segment;
    emit finishedSegment(this, segmentNum);
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

    m_size = size;

    //the filesize is not what it should be, maybe using a wrong mirror
    if (m_size && m_supposedSize && (m_size != m_supposedSize))
    {
        emit broken(this, WrongDownloadSize);
    }

    if (m_canResume)
    {
        killInitJob();
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

#include "multisegkiodatasource.moc"
