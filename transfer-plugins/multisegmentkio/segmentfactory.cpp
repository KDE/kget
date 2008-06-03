/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "segmentfactory.h"
#include "multisegkiosettings.h"
#include "../../settings.h"

#include <QtCore/QTimer>

SegData::SegData ()
{
    offset = 0;
    bytes = (KIO::filesize_t) 0;
}

Segment::Segment (QObject* parent)
  :QObject(parent)
{
    m_status = Stopped;
    m_bytesWritten = 0;
    m_getJob = 0;
}

bool Segment::createTransfer ( const KUrl &src )
{
    kDebug(5001) << " -- " << src;
    if ( m_getJob )
        return false;
    m_getJob = KIO::get(src, KIO::NoReload, KIO::HideProgressInfo);
    m_getJob->suspend();
    m_getJob->addMetaData( "errorPage", "false" );
    m_getJob->addMetaData( "AllowCompressedPage", "false" );
    if ( m_segData.offset )
    {
        m_getJob->addMetaData( "resume", KIO::number(m_segData.offset) );
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

bool Segment::startTransfer ()
{
    kDebug(5001);
    if( m_getJob && m_status != Running )
    {
        setStatus( Running, false );
        m_getJob->resume();
        return true;
    }
    return false;
}

bool Segment::stopTransfer ()
{
    kDebug(5001);
    if( m_getJob && m_status == Running )
    {
        setStatus( Stopped, false );
        m_getJob->suspend();
        if ( !m_buffer.isEmpty() )
        {
            writeBuffer();
        }
        if (m_getJob)
            m_getJob->kill( KJob::EmitResult );
        return true;
    }
    return false;
}

bool Segment::restartTransfer ( const KUrl &url )
{
    bool rest;
    rest = createTransfer( url );
    rest |= startTransfer();
    return rest;
}

void Segment::slotResult( KJob *job )
{
    kDebug(5001) << "job: " << job;
    m_getJob = 0;
    if ( !m_buffer.isEmpty() )
    {
        kDebug(5001) << "Looping until write the buffer ...";
        while(writeBuffer()) ;
    }
    if( !m_segData.bytes )
    {
        setStatus(Finished);
        deleteLater();
        return;
    }
    if( m_status == Running )
    {
        kDebug(5001) << "Conection broken " << job << " --restarting--";
        setStatus(Timeout);
    }
}

void Segment::slotData(KIO::Job *, const QByteArray& _data)
{
//     kDebug(5001) << "Segment::slotData()";
    m_buffer.append(_data);
    if ( (uint)m_buffer.size() > m_segData.bytes )
    {
//         kDebug(5001) << "Segment::slotData() buffer full. stoping transfer...";
        m_buffer.truncate( m_segData.bytes );
        m_getJob->suspend();
        m_getJob->kill( KJob::EmitResult );
        writeBuffer();
    }
    else
    { 
    /* 
     write to the local file only if the buffer has more than 8kbytes
     this hack try to avoid too much cpu usage. it seems to be due KIO::Filejob
     so remove it when it works property
    */
    if ( m_buffer.size() > 8*1024)
        writeBuffer();
    }
}

bool Segment::writeBuffer()
{
//     kDebug(5001) << "Segment::writeBuffer() sending: " << m_buffer.size() << " from job: "<< m_getJob;
    bool rest;
    emit data( this, m_buffer, rest);
    if ( rest )
    {
        m_segData.bytes -= m_buffer.size();
        m_segData.offset += m_buffer.size();
        m_bytesWritten += m_buffer.size();
        m_buffer = QByteArray();
//         kDebug(5001) << "Segment::writeBuffer() updating segment record of job: " << m_getJob << " -- " << m_segData.bytes <<" bytes left";
    }
    if (!m_segData.bytes)
    {
        kDebug(5001) << "Closing transfer ...";
        if( m_getJob )
            m_getJob->kill( KJob::EmitResult );
        emit updateSegmentData();
    }
    return rest;
}

void Segment::setStatus(Status stat, bool doEmit)
{
    m_status = stat;
    if (doEmit)
        emit statusChanged(this);
}

SegmentFactory::SegmentFactory(uint n, const QList<KUrl> Urls)
   : m_segments( n ), m_Urls( Urls ), m_split(true)

{
    kDebug(5001);
    it_Urls = m_Urls.begin();
}

SegmentFactory::SegmentFactory()
{
    kDebug(5001);
}

SegmentFactory::~SegmentFactory()
{
    kDebug(5001);
    QList<Segment *>::iterator it = m_Segments.begin();
    QList<Segment *>::iterator itEnd = m_Segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        if( (*it)->status() == Segment::Running )
            (*it)->stopTransfer();
        (*it)->deleteLater();
    }
}

bool SegmentFactory::startTransfer()
{
    kDebug(5001);
    bool rest = false;
    QList<Segment *>::iterator it = m_Segments.begin();
    QList<Segment *>::iterator itEnd = m_Segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        rest |= (*it)->startTransfer();
    }
    return rest;
}

bool SegmentFactory::stopTransfer()
{
    kDebug(5001);
    bool rest = false;
    QList<Segment *>::iterator it = m_Segments.begin();
    QList<Segment *>::iterator itEnd = m_Segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        rest |= (*it)->stopTransfer();
    }
    return rest;
}

QList<SegData> SegmentFactory::SegmentsData()
{
    kDebug(5001);
    QList<SegData> tdata;
    QList<Segment *>::iterator it = m_Segments.begin();
    QList<Segment *>::iterator itEnd = m_Segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        if( (*it)->data().bytes )
            tdata << (*it)->data();
    }
    return tdata;
}

QList<Segment *> SegmentFactory::splitSegment( Segment *Seg, int n)
{
    kDebug(5001) << "Spliting " << Seg << "in " << n;
    QList<Segment *> Segments;

    KIO::TransferJob *Job = Seg->job();
    if(Job)
    {
        Job->suspend();
        kDebug(5001) << "job Suspended...";
    }

    KIO::filesize_t bytes = Seg->data().bytes;
    KIO::filesize_t offset = Seg->data().offset;

    uint splitSize = 50;
    if( MultiSegKioSettings::splitSize() )
    {
        splitSize = MultiSegKioSettings::splitSize();
    }
    int min = bytes/(splitSize*1024);
	
    if( min < n )
    {
        n = min;
    }

    if( n == 0 )
    {
        kDebug(5001) << "Segment can't be splited.";
        if(Job)
        {
            Job->resume();
            kDebug(5001) << "Resuming Job...";
        }
        return Segments;
    }

    KIO::filesize_t segment = bytes/n;

    kDebug(5001) << "spliting: " << Seg->data().bytes <<" in "<< n << "  and got: " << segment;

    KIO::fileoffset_t rest_size = segment + ( bytes%n );
    Seg->setBytes( segment );
    kDebug(5001) << "Now the segment has: " << Seg->data().bytes <<" bytes.";

    if(Job)
    {
        Job->resume();
        kDebug(5001) << "Resuming Job...";
    }

    SegData data;
    for(int i = 1; i < n; i++)
    {
        if(i == n - 1)
        {
            data.offset = i*segment + offset;
            data.bytes = rest_size;
            Segments << createSegment(data, nextUrl());
            kDebug(5001) << "Segment created at offset: "<< data.offset <<" with "<< data.bytes << " bytes.";
            continue;
        }
        data.offset = i*segment + offset;
        data.bytes = segment;
        Segments << createSegment(data, nextUrl());
        kDebug(5001) << "Segment created at offset: "<< data.offset <<" with "<< data.bytes << " bytes.";
    }

    return Segments;
}

Segment *SegmentFactory::createSegment( SegData data, const KUrl &src )
{
    kDebug(5001);
    Segment *seg = new Segment(this);
    connect( seg, SIGNAL(statusChanged( Segment *)),
                  SLOT(slotStatusChanged( Segment *)));
    seg->setData(data);
    seg->createTransfer( src );
    m_Segments.append(seg);
    emit createdSegment(seg);
    return seg;
}

void SegmentFactory::deleteSegment(Segment *seg)
{
    m_Segments.removeAll(seg);
    kDebug(5001) << m_Segments.size() << " segments left.";
}

void SegmentFactory::slotStatusChanged( Segment *seg)
{
    kDebug(5001) << seg->status();
    switch (seg->status())
    {
    case Segment::Timeout :
        kDebug(5001) << "Restarting Segment in 5 seg... ";
        m_TimeOutSegments << seg;
        QTimer::singleShot(5000, this, SLOT(slotSegmentTimeOut()));
    break;
    case Segment::Finished :
        deleteSegment(seg);
        if( !m_Segments.isEmpty() )
        {
            Segment* longSeg = takeLongest();
            if( longSeg == 0)
                break;
            QList<Segment*> segl = splitSegment( longSeg, 2);
            if( !segl.isEmpty() )
                segl.takeFirst()->startTransfer();
        }
    break;
    default:
    break;
    }
}

void SegmentFactory::slotSegmentTimeOut()
{
    kDebug(5001) <<  m_TimeOutSegments.size();
    if(m_TimeOutSegments.isEmpty())
        return;
    m_TimeOutSegments.takeFirst()->restartTransfer( nextUrl() );
}

Segment *SegmentFactory::takeLongest()
{
    kDebug(5001);

    Segment *longest = 0;
    KIO::filesize_t bytes = MultiSegKioSettings::splitSize()*1024;

    QList<Segment*>::const_iterator it = m_Segments.begin();
    QList<Segment*>::const_iterator itEnd = m_Segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        if((*it)->data().bytes > bytes)
        {
            longest = (*it);
            bytes = (*it)->data().bytes;
        }
    }

    if(longest)
        kDebug(5001) << "the longest segment has: " << longest->data().bytes;

    return longest;
}

const KUrl SegmentFactory::nextUrl()
{
    kDebug(5001);
    if ( it_Urls == m_Urls.end() )
    {
        it_Urls = m_Urls.begin();
    }
    KUrl url(*it_Urls);
    it_Urls++;
    return url;
}

#include "segmentfactory.moc"
