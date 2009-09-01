/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "segment.h"

#include <KDebug>

#include <QtCore/QTimer>

Segment::Segment (const KUrl &src, KIO::fileoffset_t offset, KIO::fileoffset_t bytes, int segmentNum, QObject *parent)
  : QObject(parent),
    m_status(Stopped),
    m_offset(offset),
    m_bytes(bytes),
    m_segmentNum(segmentNum),
    m_bytesWritten(0),
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
        m_getJob->kill(KJob::Quietly);
    }
}

bool Segment::createTransfer ( const KUrl &src )
{
    kDebug(5001) << " -- " << src;
    if ( m_getJob )
        return false;

    m_getJob = KIO::get(src, KIO::Reload, KIO::HideProgressInfo);
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
        createTransfer(m_url);
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
    if( !m_bytes )
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
            kDebug(5001) << "Segment" << m_segmentNum << "broken, using" << m_url;
            setStatus(Timeout);
            emit brokenSegment(this, m_segmentNum);
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
    if (static_cast<uint>(m_buffer.size()) >= m_bytes)
    {
        kDebug(5001) << "Segment::slotData() buffer full. stoping transfer...";
        m_buffer.truncate( m_bytes );
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
        m_bytes -= m_buffer.size();
        m_offset += m_buffer.size();
        m_bytesWritten += m_buffer.size();
        m_buffer.clear();
        kDebug(5001) << "Segment::writeBuffer() updating segment record of job:" << m_getJob << "--" << m_bytes << "bytes left";
    }
    if (!m_bytes)
    {
        kDebug(5001) << "Closing transfer ...";
        if (m_getJob)
        {
            m_getJob->kill(KJob::Quietly);
            m_getJob = 0;
        }
        emit finishedSegment(this, m_segmentNum);
    }
    return worked;
}

void Segment::setStatus(Status stat, bool doEmit)
{
    m_status = stat;
    if (doEmit)
        emit statusChanged(this);
}

#include "segment.moc"
