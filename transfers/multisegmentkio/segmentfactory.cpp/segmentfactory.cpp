/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include "segmentfactory.h"

SegData::SegData ()
{
   offset = 0;
   bytes = (KIO::filesize_t)-1;
}

Segment::Segment ()
{
   m_stoped = true;
   m_bytesWritten = 0;
   m_chunkSize =0;
   m_getJob = 0;
}

bool Segment::createTransfer ( KUrl src )
{
   kDebug(5001) << "Segment::startTransfer()" << endl;
   if ( m_getJob )
      return false;
   m_getJob = KIO::get(src, false, false);
   m_getJob->internalSuspend();
//    m_getJob->url(); // to know the url
   if ( m_segData.offset )
   {
      m_getJob->addMetaData( "resume", KIO::number(m_segData.offset) );
   }
   connect( m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)),
                 SLOT( slotData(KIO::Job *, const QByteArray&)));
   connect( m_getJob, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));

   return true;
}

bool Segment::startTransfer ( )
{
   m_stoped = false;
   if( m_getJob )
      m_getJob->internalResume();
}

bool Segment::stopTransfer ( )
{
   m_stoped = true;
   if( m_getJob )
      m_getJob->internalSuspend();
}

void Segment::slotResult( KJob *job )
{
   m_getJob = 0;
   if( m_stoped || !m_segData.bytes )
   {
      emit updateSegmentData();
      deleteLater();
   }
   else
   {
   }
}

void Segment::slotData(KIO::Job *, const QByteArray& _data)
{
   kDebug(5001) << "Segment::slotData()" << endl;
   m_buffer.append(_data);
   if ( m_buffer.size() > m_segData.bytes )
   {
      m_buffer.truncate( m_segData.bytes );
      m_getJob->internalSuspend();
   }
   kDebug(5001) << "Segment::slotData() sending: " << m_buffer.size() << " from job: "<< m_getJob << endl;
   bool rest;
   emit data( this, m_buffer, rest);
   kDebug(5001) << "Segment::slotData() the writer return: " << rest << endl;
   if ( rest )
   {
      m_segData.bytes -= m_buffer.size();
      m_segData.offset += m_buffer.size();
      m_bytesWritten += m_buffer.size();
      m_chunkSize += m_buffer.size();
      m_buffer = QByteArray();
      kDebug(5001) << "Segment::slotData() updating segment record of job: " << m_getJob << " -- " << m_segData.bytes <<" bytes left"<< endl;
      if( m_chunkSize > 50*1024)
      {
         emit updateSegmentData();
         m_chunkSize =0;
      }
      if (!m_segData.bytes)
      {
         kDebug(5001) << "Segment::slotData() closing transfer ..." << endl;
         m_getJob->kill( KJob::EmitResult );
         return;
      }
   }
}

SegmentFactory::SegmentFactory(uint n, const QList<KUrl> Urls, QList<SegData> SegmentsData)
   : m_segments( n ), m_Urls( Urls )
{
   kDebug(5001) << "SegmentFactory::SegmentFactory()" << endl;
   it_Urls = m_Urls.begin();
   if ( !SegmentsData.isEmpty() )
   {
      QList<SegData>::const_iterator it = SegmentsData.begin();
      QList<SegData>::const_iterator itEnd = SegmentsData.end();
      for ( ; it!=itEnd ; ++it )
      {
         m_Segments << createSegment( (*it), nextUrl() );
      }
   }
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
      if( !(*it)->job() )
         m_Segments.erase(it);
   }
   return tdata;
}

QList<Segment *> SegmentFactory::splitSegment( Segment *Seg, int n)
{
   kDebug(5001) << "SegmentFactory::splitSegment() " << n << endl;
   QList<Segment *> Segments;

   KIO::filesize_t bytes = Seg->data().bytes;
   uint min = bytes/MIN_SIZE;

   if( min < n )
   {
      n = min;
   }

   if( n <= 1 )
      return Segments;

   KIO::filesize_t segment = bytes/n;
   KIO::fileoffset_t rest_size = segment + ( bytes%n );
   Seg->setBytes( segment );
   SegData data;
   for( uint i = 1; i < n; i++)
      {
         if(i == n - 1)
         {
            data.offset = i*segment + Seg->BytesWritten();
            data.bytes = rest_size;
            Segments << createSegment(data, nextUrl());
            continue;
         }
         data.offset = i*segment;
         data.bytes = segment;
         Segments << createSegment(data, nextUrl());
      }
   return Segments;
}

Segment *SegmentFactory::createSegment( SegData data, KUrl src )
{
   kDebug(5001) << "SegmentFactory::createSegment()" << endl;
   Segment *seg = new Segment();
   seg->setData(data);
   seg->createTransfer( src );
   m_Segments.append(seg);
   return seg;
}

const KUrl SegmentFactory::nextUrl()
{
   kDebug(5001) << "SegmentFactory::nextUrl() " << m_Urls.size() << endl;
   if ( it_Urls == m_Urls.end() )
   {
      it_Urls = m_Urls.begin();
   }
   KUrl url(*it_Urls);
   it_Urls++;
   return url;
}
#include "segmentfactory.moc"
