/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "segmentfactory.h"
#include "multisegkiosettings.h"

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
    kDebug(5001) << "Segment::createTransfer() -- " << src << endl;
    if ( m_getJob )
        return false;
    m_getJob = KIO::get(src, false, false);
    m_getJob->internalSuspend();
    m_getJob->addMetaData( "errorPage", "false" );
    m_getJob->addMetaData( "AllowCompressedPage", "false" );
    if ( m_segData.offset )
    {
        m_getJob->addMetaData( "resume", KIO::number(m_segData.offset) );
    }
    connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
                 SLOT( slotData(KIO::Job *, const QByteArray&)));
    connect( m_getJob, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));

    return true;
}

bool Segment::startTransfer ()
{
    kDebug(5001) << "Segment::startTransfer()"<< endl;
    if( m_getJob && m_status != Running )
    {
        setStatus( Running, false );
        m_getJob->internalResume();
        return true;
    }
    return false;
}

bool Segment::stopTransfer ()
{
    kDebug(5001) << "Segment::stopTransfer()" << endl;
    if( m_getJob && m_status == Running )
    {
        setStatus( Stopped, false );
        m_getJob->internalSuspend();
        if ( !m_buffer.isEmpty() )
        {
            kDebug(5001) << "Looping until write the buffer ..." << endl;
            while(writeBuffer());
        }
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
    kDebug(5001) << "Segment::slotResult() job: " << job << endl;
    m_getJob = 0;
    if ( !m_buffer.isEmpty() )
    {
        kDebug(5001) << "Looping until write the buffer ..." << endl;
        while(writeBuffer());
    }
    if( !m_segData.bytes )
    {
        setStatus(Finished);
        deleteLater();
        return;
    }
    if( m_status == Running )
    {
        kDebug(5001) << "Segment::slotResult() Conection broken " << job << " --restarting--" << endl;
        setStatus(Timeout);
    }
}

void Segment::slotData(KIO::Job *, const QByteArray& _data)
{
//     kDebug(5001) << "Segment::slotData()" << endl;
    m_buffer.append(_data);
    if ( m_buffer.size() > m_segData.bytes )
    {
//         kDebug(5001) << "Segment::slotData() buffer full. stoping transfer..." << endl;
        m_buffer.truncate( m_segData.bytes );
        m_getJob->internalSuspend();
        m_getJob->kill( KJob::EmitResult );
    }
    if ( m_buffer.size() )
        writeBuffer();
}

bool Segment::writeBuffer()
{
//     kDebug(5001) << "Segment::writeBuffer() sending: " << m_buffer.size() << " from job: "<< m_getJob << endl;
    bool rest;
    emit data( this, m_buffer, rest);
    if ( rest )
    {
        m_segData.bytes -= m_buffer.size();
        m_segData.offset += m_buffer.size();
        m_bytesWritten += m_buffer.size();
        m_buffer = QByteArray();
//         kDebug(5001) << "Segment::writeBuffer() updating segment record of job: " << m_getJob << " -- " << m_segData.bytes <<" bytes left"<< endl;
    }
    if (!m_segData.bytes)
    {
        kDebug(5001) << "Segment::writeBuffer() closing transfer ..." << endl;
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
    kDebug(5001) << "SegmentFactory::SegmentFactory()" << endl;
    it_Urls = m_Urls.begin();
}

SegmentFactory::~SegmentFactory()
{
    kDebug(5001) << "SegmentFactory::destructor()" << endl;
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
    kDebug(5001) << "SegmentFactory::startTransfer()" << endl;
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
    kDebug(5001) << "SegmentFactory::stopTransfer()" << endl;
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
    kDebug(5001) << "SegmentFactory::getSegmentsData" << endl;
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
    kDebug(5001) << "SegmentFactory::splitSegment() " << Seg << endl;
    QList<Segment *> Segments;

    KIO::TransferJob *Job = Seg->job();
    if(Job)
    {
        Job->internalSuspend();
        kDebug(5001) << "job Suspended..." << endl;
    }

    KIO::filesize_t bytes = Seg->data().bytes;
    KIO::filesize_t offset = Seg->data().offset;

    int min = bytes/(MultiSegKioSettings::splitSize()*1024);

    if( min < n )
    {
        n = min;
    }

    if( n == 0 )
    {
        kDebug(5001) << "Segment can't be splited." << endl;
        if(Job)
        {
            Job->internalResume();
            kDebug(5001) << "Resuming Job..." << endl;
        }
        return Segments;
    }

    KIO::filesize_t segment = bytes/n;

    kDebug(5001) << "spliting: " << Seg->data().bytes <<" in "<< n << "  and got: " << segment << endl;

    KIO::fileoffset_t rest_size = segment + ( bytes%n );
    Seg->setBytes( segment );
    kDebug(5001) << "Now the segment has: " << Seg->data().bytes <<" bytes."<< endl;

    if(Job)
    {
        Job->internalResume();
        kDebug(5001) << "Resuming Job..." << endl;
    }

    SegData data;
    for(int i = 1; i < n; i++)
    {
        if(i == n - 1)
        {
            data.offset = i*segment + offset;
            data.bytes = rest_size;
            Segments << createSegment(data, nextUrl());
            kDebug(5001) << "Segment created at offset: "<< data.offset <<" with "<< data.bytes << " bytes." << endl;
            continue;
        }
        data.offset = i*segment + offset;
        data.bytes = segment;
        Segments << createSegment(data, nextUrl());
        kDebug(5001) << "Segment created at offset: "<< data.offset <<" with "<< data.bytes << " bytes." << endl;
    }

    return Segments;
}

Segment *SegmentFactory::createSegment( SegData data, const KUrl &src )
{
    kDebug(5001) << "SegmentFactory::createSegment()" << endl;
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
    kDebug(5001) << "SegmentFactory::deleteSegment() " << m_Segments.size() << " segments left." << endl;
}

void SegmentFactory::slotStatusChanged( Segment *seg)
{
    kDebug(5001) << "SegmentFactory::slotStatusChanged() " << seg->status() << endl;
    switch (seg->status())
    {
    case Segment::Timeout :
        kDebug(5001) << "Restarting Segment in 5 seg... " << endl;
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
    }
}

void SegmentFactory::slotSegmentTimeOut()
{
    kDebug(5001) << "SegmentFactory::slotSegmentTimeOut() " <<  m_TimeOutSegments.size() << endl;
    if(m_TimeOutSegments.isEmpty())
        return;
    m_TimeOutSegments.takeFirst()->restartTransfer( nextUrl() );
}

Segment *SegmentFactory::takeLongest()
{
    kDebug(5001) << "SegmentFactory::takeLongest()" << endl;

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
        kDebug(5001) << "the longest segment has: " << longest->data().bytes << endl;

    return longest;
}

const KUrl SegmentFactory::nextUrl()
{
    kDebug(5001) << "SegmentFactory::nextUrl() " << endl;
    if ( it_Urls == m_Urls.end() )
    {
        it_Urls = m_Urls.begin();
    }
    KUrl url(*it_Urls);
    it_Urls++;
    return url;
}

#include "segmentfactory.moc"
