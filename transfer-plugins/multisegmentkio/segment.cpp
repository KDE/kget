/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "segment.h"

#include <math.h>

#include <KDebug>

#include <QtCore/QTimer>

Segment::Segment(const KUrl &src, const KIO::fileoffset_t offset, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange, QObject *parent)
  : QObject(parent),
    m_status(Stopped),
    m_offset(offset),
    m_segSize(segmentSize),
    m_curentSegSize(m_segSize.first),
    m_curentSegment(segmentRange.first),
    m_endSegment(segmentRange.second),
    m_bytesWritten(0),
    m_totalBytesLeft(m_segSize.first * (m_endSegment - m_curentSegment) + m_segSize.second),
    m_getJob(0),
    m_canResume(true),
    m_url(src),
    m_restarted(0)
{
}

Segment::~Segment()
{
    if (m_getJob)
    {
        kDebug(5001) << "Closing transfer ...";
        m_getJob->kill(KJob::Quietly);
    }
}

bool Segment::createTransfer()
{
    kDebug(5001) << " -- " << m_url;
    if ( m_getJob )
        return false;

    m_getJob = KIO::get(m_url, KIO::Reload, KIO::HideProgressInfo);
    m_getJob->suspend();
    m_getJob->addMetaData( "errorPage", "false" );
    m_getJob->addMetaData( "AllowCompressedPage", "false" );
    if (m_offset)
    {
        m_canResume = false;
        m_getJob->addMetaData( "resume", KIO::number(m_offset) );
        connect(m_getJob, SIGNAL(canResume(KIO::Job *, KIO::filesize_t)),
                 SLOT( slotCanResume(KIO::Job *, KIO::filesize_t)));
    }
    #if 0 //TODO: we disable that code till it's implemented in kdelibs, also we need to think, which settings we should use
    if(Settings::speedLimit())
    {
                m_getJob->addMetaData( "speed-limit", KIO::number(Settings::transferSpeedLimit() * 1024) );
    }
    #endif
    connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
                 SLOT( slotData(KIO::Job *, const QByteArray&)));
    connect( m_getJob, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));
    return true;
}

void Segment::slotCanResume( KIO::Job* job, KIO::filesize_t offset )
{
    Q_UNUSED(job);
    Q_UNUSED(offset);
    kDebug(5001);
    m_canResume = true;
}


bool Segment::startTransfer ()
{
    kDebug(5001);
    if (!m_getJob)
    {
        createTransfer();
    }
    if( m_getJob && m_status != Running )
    {
        setStatus( Running, false );
        m_getJob->resume();
        return true;
    }
    return false;
}

bool Segment::stopTransfer()
{
    kDebug(5001);

    if(m_getJob)
    {
        setStatus( Stopped, false );

        if (m_getJob)
        {
            m_getJob->kill( KJob::EmitResult );
        }
        return true;
    }
    return false;
}

void Segment::slotResult( KJob *job )
{
    kDebug(5001) << "Job:" << job;

    m_getJob = 0;

    //clear the buffer as the download might be moved around
    if (m_status == Stopped)
    {
        m_buffer.clear();
    }
    if ( !m_buffer.isEmpty() )
    {
        kDebug(5001) << "Looping until write the buffer ...";
//         while(writeBuffer()) ;
    }
    if (!m_totalBytesLeft)
    {
        setStatus(Finished);
        deleteLater();
        return;
    }
    if( m_status == Killed )
    {
        deleteLater();
        return;
    }
    if( m_status == Running )
    {
        ++m_restarted;
        //try it 3 trimes
        if (m_restarted <= 3)
        {
            const int seconds = 2 * m_restarted;
            kDebug(5001) << "Conection broken" << job << "--restarting in" << seconds << "seconds for the" << m_restarted << "time--";
            QTimer::singleShot(seconds * 1000, this, SLOT(startTransfer()));
            setStatus(Timeout);
        }
        else
        {
            kDebug(5001) << "Segment" << m_curentSegment << "broken, using" << m_url;
            setStatus(Timeout);
            emit brokenSegment(this, m_curentSegment);
        }
    }
}

void Segment::slotData(KIO::Job *, const QByteArray& _data)
{
    // Check if the transfer allow resuming...
    if ( m_offset && !m_canResume)
    {
        kDebug(5001) << "the remote site does not allow resuming ...";
        stopTransfer();
        setStatus(Killed, false );
        return;
    }

    m_buffer.append(_data);
    if (static_cast<uint>(m_buffer.size()) >= m_totalBytesLeft)
    {
        kDebug(5001) << "Segment::slotData() buffer full. stoping transfer...";
        m_buffer.truncate(m_totalBytesLeft);
        writeBuffer();
    }
    else
    { 
    /* 
     write to the local file only if the buffer has more than 8kbytes
     this hack try to avoid too much cpu usage. it seems to be due KIO::Filejob
     so remove it when it works property
    */
    if (m_buffer.size() > 16 * 1024)
        writeBuffer();
    }
}

bool Segment::writeBuffer()
{
    kDebug(5001) << "Segment::writeBuffer() sending:" << m_buffer.size() << "from job:" << m_getJob;
    if ( m_buffer.isEmpty() )
    {
        return false;
    }

    bool worked = false;
    emit data(m_offset, m_buffer, worked);

    if (worked)
    {
        m_curentSegSize -= m_buffer.size();
        m_totalBytesLeft -= m_buffer.size();
        m_offset += m_buffer.size();
        m_bytesWritten += m_buffer.size();
        m_buffer.clear();
        kDebug(5001) << "Segment::writeBuffer() updating segment record of job:" << m_getJob << "--" << m_totalBytesLeft << "bytes left";
    }
    //at least one segment has been finished
    if (m_curentSegSize <= 0)
    {
        bool finished = (m_curentSegment == m_endSegment);
        if (finished)
        {
            emit finishedSegment(this, m_curentSegment, finished);
        }
        else
        {
            while (m_curentSegSize <= 0)
            {
                emit finishedSegment(this, m_curentSegment, finished);
                ++m_curentSegment;
                finished = (m_curentSegment == m_endSegment);
                m_curentSegSize += (finished ? m_segSize.second : m_segSize.first);
                if (!m_curentSegSize)
                {
                    break;
                }
            }
        }
    }
    return worked;
}

void Segment::setStatus(Status stat, bool doEmit)
{
    m_status = stat;
    if (doEmit)
        emit statusChanged(this);
}

QPair<int, int> Segment::assignedSegments() const
{
    return QPair<int, int>(m_curentSegment, m_endSegment);
}

int Segment::countUnfinishedSegments() const
{
    return m_endSegment - m_curentSegment;//do not count the current segment//TODO change that maybe?
}

int Segment::takeOneSegment()
{
    if (m_getJob)
    {
        m_getJob->suspend();
    }

    int oneSegment = -1;
    int free = countUnfinishedSegments();
    if (free > 1)
    {
        oneSegment = m_endSegment;
        --m_endSegment;
    }

    kDebug(5001) << "Taken segment" << oneSegment;

    if (m_getJob)
    {
        m_getJob->resume();
    }
    return oneSegment;
}

QPair<int, int> Segment::split()
{
    if (m_getJob)
    {
        m_getJob->suspend();
    }

    QPair<int, int> freed = QPair<int, int>(-1, -1);
    int free = ceil((countUnfinishedSegments() + 1) / static_cast<double>(2));

    if (!free)
    {
        kDebug(5001) << "None freed, start:" << m_curentSegment << "end:" << m_endSegment;

        if (m_getJob)
        {
            m_getJob->resume();
        }
        return freed;
    }
//FIXME last seg, when one connection removed not finished, why???
    const int newEnd = m_endSegment - free;
    freed = QPair<int, int>(newEnd + 1, m_endSegment);
    kDebug(5001) << "Start:" << m_curentSegment << "old end:" << m_endSegment << "new end:" << newEnd << "freed:" << freed;
    m_endSegment = newEnd;
    m_totalBytesLeft -= m_segSize.first * (free - 1) + m_segSize.second;

    //end changed, so in any case the lastSegSize should be the normal segSize
    if (free)
    {
        m_segSize.second = m_segSize.first;
    }

    if (m_getJob)
    {
        m_getJob->resume();
    }
    return freed;
}

#include "segment.moc"
