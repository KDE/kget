/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kio/observer.h>
#include <kio/slave.h>
#include <kio/global.h>

#include <klocale.h>
#include <kde_file.h>

#include <qtimer.h>
#include <QDateTime>
#include <QFile>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

#include "MultiSegKio.h"

using namespace KIO;

/**
**/
MultiSegmentCopyJob::MultiSegmentCopyJob( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, uint segments)
    : Job(showProgressInfo), m_src(src), m_dest(dest), m_permissions(permissions), m_ProcessedSize(0), m_segments(segments)
{
   if (showProgressInfo)
      Observer::self()->slotCopying( this, src, dest );

   kDebug(7007) << "MultiSegmentCopyJob::MultiSegmentCopyJob()" << endl;
   blockWrite = false;
   bcopycompleted = false;
   bstartsaved = false;
   m_getJob = 0;
   m_putJob = 0;
   QTimer::singleShot(0, this, SLOT(slotStart()));
}

MultiSegmentCopyJob::MultiSegmentCopyJob(
                           const KUrl& src,
                           const KUrl& dest,
                           int permissions, bool showProgressInfo,
                           qulonglong ProcessedSize,
                           KIO::filesize_t totalSize,
                           QList<struct MultiSegData> segments)

    : Job(showProgressInfo), m_src(src), m_dest(dest), m_permissions(permissions), m_ProcessedSize(ProcessedSize),
    m_totalSize(totalSize)
{
   if (showProgressInfo)
      Observer::self()->slotCopying( this, src, dest );

   kDebug(7007) << "MultiSegmentCopyJob::MultiSegmentCopyJob()" << endl;
   blockWrite = false;
   bcopycompleted = false;
   bstartsaved = true;
   m_getJob = 0;
   m_putJob = 0;
   QTimer::singleShot(0, this, SLOT(slotStart()));
   addGetJobManagers(segments);
}

void MultiSegmentCopyJob::addGetJobManagers( QList<struct MultiSegData> segments)
{
   QList <struct MultiSegData>::const_iterator it = segments.begin();
   QList <struct MultiSegData>::const_iterator itEnd = segments.end();
   for ( ; it!=itEnd ; ++it )
   {
      slotaddGetJobManager(*it);
   }
}

void MultiSegmentCopyJob::slotaddGetJobManager(struct MultiSegData segment)
{
   kDebug(7007) << "MultiSegmentCopyJob::slotaddGetJobManager()" << endl
    << "src: " << segment.src << endl
    << "offset: " << segment.offset << endl
    << "bytes: " << segment.bytes << endl;

   GetJobManager *jobManager = new GetJobManager();
   FileJob *job = KIO::open(segment.src, 1);
   jobManager->job = job;
   jobManager->data.src = segment.src;
   jobManager->data.offset = segment.offset;
   jobManager->data.bytes = segment.bytes;

   connect( job, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));

   connect( this, SIGNAL(canWrite()), jobManager, SLOT(slotCanWrite()));

   connect( jobManager, SIGNAL(hasData( GetJobManager * )), SLOT(slotDataReq( GetJobManager *)));
   connect( jobManager, SIGNAL(segmentData(struct MultiSegData)), SLOT(slotaddGetJobManager(struct MultiSegData)));

   connect(job, SIGNAL(open(KIO::Job *)), jobManager, SLOT(slotOpen(KIO::Job *)));
   connect( job, SIGNAL(data( KIO::Job *, const QByteArray & )), jobManager, SLOT(slotData( KIO::Job *, const QByteArray &)));
   connect( job, SIGNAL(result(KJob *)), jobManager, SLOT(slotResult( KJob *)));

   m_jobs.append(jobManager);
   addSubjob( job );
}

void MultiSegmentCopyJob::addFisrtGetJobManager( FileJob* job, struct MultiSegData segment)
{
   GetJobManager *jobManager = new GetJobManager();
   jobManager->job = job;
   jobManager->data.src = segment.src;
   jobManager->data.offset = segment.offset;
   jobManager->data.bytes = segment.bytes;

   connect( this, SIGNAL(canWrite()), jobManager, SLOT(slotCanWrite()));

   connect( jobManager, SIGNAL(hasData( GetJobManager * )), SLOT(slotDataReq( GetJobManager *)));
   connect( jobManager, SIGNAL(segmentData(struct MultiSegData)), SLOT(slotaddGetJobManager(struct MultiSegData)));

   connect( job, SIGNAL(data( KIO::Job *, const QByteArray & )), jobManager, SLOT(slotData( KIO::Job *, const QByteArray &)));
   connect( job, SIGNAL(result(KJob *)), jobManager, SLOT(slotResult( KJob *)));

   m_jobs.append(jobManager);
}

QList<struct MultiSegData> MultiSegmentCopyJob::getSegmentsData()
{
    kDebug(7007) << "MultiSegmentCopyJob::getSegmentsData" << endl;
    QList<struct MultiSegData> tdata;
    QList<GetJobManager*>::const_iterator it = m_jobs.begin();
    QList<GetJobManager*>::const_iterator itEnd = m_jobs.end();
    for ( ; it!=itEnd ; ++it )
    {
        tdata << (*it)->data;
    }
    return tdata;
}

void MultiSegmentCopyJob::slotStart()
{
   kDebug(7007) << "MultiSegmentCopyJob::slotStart()" << endl;
   if( !checkLocalFile() )
      emitResult();

   kDebug(7007) << "MultiSegmentCopyJob::slotStart() opening: " << m_dest_part << endl;
   m_putJob = KIO::open(m_dest_part, 3);
   connect( m_putJob, SIGNAL(open(KIO::Job *)), SLOT(slotOpen(KIO::Job *)));
   connect(m_putJob, SIGNAL(close(KIO::Job *)), SLOT(slotClose(KIO::Job *)));
   connect( m_putJob, SIGNAL(written(KIO::Job * ,KIO::filesize_t )), SLOT(slotWritten( KIO::Job * ,KIO::filesize_t )));
   connect( m_putJob, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));
   addSubjob( m_putJob );
}

void MultiSegmentCopyJob::slotOpen( KIO::Job * job)
{
   if( job == m_putJob )
   {
      kDebug(7007) << "MultiSegmentCopyJob::slotOpen() putjob" << endl;
      if(bstartsaved)
         return;
      FileJob* getjob = KIO::open(m_src, 1);
      connect(getjob, SIGNAL(open(KIO::Job *)), SLOT(slotOpen(KIO::Job *)));
      connect( getjob, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));
      connect( getjob, SIGNAL(totalSize(KJob *, qulonglong)), 
                this, SLOT(slotTotalSize(KJob *, qulonglong)));

      addSubjob( getjob );
   }
   else
   {
      FileJob* getjob = static_cast<FileJob*>(job);
      m_totalSize = getjob->size();
      if (m_totalSize == -1)
      {
      kDebug(7007) << "MultiSegmentCopyJob::slotOpen() getjob: wrong file size. exiting...." << endl;
      getjob->close();
      m_putJob->close();
      return;
      }
      kDebug(7007) << "MultiSegmentCopyJob::slotOpen() getjob: " << getjob->size() << endl;
      emit totalSize(this, m_totalSize);
      uint min = m_totalSize/MIN_SIZE;
      if(min < m_segments)
      {
         m_segments = min;
      }
      if(min == 0)
      {
         m_segments = 1;
      }

      KIO::filesize_t segment = m_totalSize/m_segments;
      KIO::fileoffset_t rest_size = segment + (m_totalSize%m_segments);

      struct MultiSegData segmentInfo;

      segmentInfo.src = m_src;
      segmentInfo.offset = 0;
      segmentInfo.bytes = segment;
      addFisrtGetJobManager( getjob, segmentInfo);
      getjob->read(segment);

      for( uint i = 1; i < m_segments; i++)
      {
         if(i == m_segments - 1)
         {
            segmentInfo.src = m_src;
            segmentInfo.offset = i*segment;
            segmentInfo.bytes = rest_size;
            slotaddGetJobManager( segmentInfo );
            continue;
         }
         segmentInfo.src = m_src;
         segmentInfo.offset = i*segment;
         segmentInfo.bytes = segment;
         slotaddGetJobManager( segmentInfo );
      }
      emit updateSegmentsData();
   }

}

void MultiSegmentCopyJob::slotClose( KIO::Job * )
{
   kDebug(7007) << "MultiSegmentCopyJob::slotClose() putjob" << endl;
   if( bcopycompleted )
   {
      QString dest_orig = m_dest.path();
      QString dest_part = m_dest_part.path();
      QFile::rename ( dest_part, dest_orig );
   }
   emit updateSegmentsData();
}

void MultiSegmentCopyJob::slotDataReq( GetJobManager *jobManager)
{
   kDebug(7007) << "MultiSegmentCopyJob::slotDataReq() " << jobManager->job << endl;
   if ( blockWrite )
   {
   kDebug(7007) << "MultiSegmentCopyJob::slotDataReq() the putjob is busy trying... next time " << jobManager->job << endl;
      return;
   }
   kDebug(7007) << "MultiSegmentCopyJob::slotDataReq() putjob writing from: " << jobManager->job << endl;
   blockWrite = true;
   m_getJob = jobManager;
   m_putJob->seek( jobManager->data.offset );
   m_putJob->write( jobManager->getData() );
}

void MultiSegmentCopyJob::slotWritten( KIO::Job * ,KIO::filesize_t bytesWritten)
{
   kDebug(7007) << "MultiSegmentCopyJob::slotWritten() " << bytesWritten << " from: " << m_getJob->job << endl;
   m_ProcessedSize += bytesWritten;
   emit processedSize(this, m_ProcessedSize);
   emitPercent( m_ProcessedSize, m_totalSize );
   if( blockWrite && m_getJob )
   {
      m_getJob->data.offset = m_getJob->data.offset + bytesWritten;
      m_getJob->data.bytes = m_getJob->data.bytes - bytesWritten;
      kDebug(7007) << "MultiSegmentCopyJob::slotWritten() bytes left: " << m_getJob->data.bytes << endl;
      if(!m_getJob->data.bytes)
      {
         m_getJob->deleteLater();
         m_jobs.removeAll(m_getJob);
         kDebug(7007) << "Segments left: " << m_jobs.size() << endl;
      }
      m_getJob = 0;
      blockWrite = false;
      emit canWrite();
   }
   if(m_jobs.isEmpty())
   {
      bcopycompleted = true;
      kDebug(7007) << "Download completed.. closing putjob " << endl;
      m_putJob->close();
   }
   emit updateSegmentsData();
}

void MultiSegmentCopyJob::slotResult( KJob *job )
{
   kDebug(7007) << "MultiSegmentCopyJob::slotResult()" << job <<endl;
   if( job->error() )
   {
      setError( job->error() );
      setErrorText( job->errorText() );
      emitResult();
      return;
   }
   if (job == m_putJob )
   {
      kDebug(7007) << "MultiSegmentCopyJob: m_putJob finished " << endl;
      m_putJob = 0;
      removeSubjob(job);
   }
   else //is a get job
   {
      kDebug(7007) << "MultiSegmentCopyJob: getJob finished " << endl;
      removeSubjob(job);
   }

   if ( !hasSubjobs() )
   {
      kDebug(7007) << "MultiSegmentCopyJob: finished " << endl;
      emitResult();
   }
}

void MultiSegmentCopyJob::slotTotalSize( KJob *job, qulonglong size )
{
   kDebug(7007) << "MultiSegmentCopyJob::slotTotalSize() " << size << endl;
//    emit totalSize(this, size);
}

void MultiSegmentCopyJob::slotPercent( KJob *job, unsigned long pct )
{
}

void MultiSegmentCopyJob::slotSpeed( KIO::Job* job, unsigned long bytes_per_second )
{
   if(job == m_putJob)
     emit speed( this, bytes_per_second );
}

bool MultiSegmentCopyJob::checkLocalFile()
{
   QString dest_orig = m_dest.path();
   QString dest_part( dest_orig );
   dest_part += QLatin1String(".part");
   QByteArray _dest_part( QFile::encodeName(dest_part));

   KDE_struct_stat buff_part;
   bool bPartExists = (KDE_stat( _dest_part.data(), &buff_part ) != -1);
   if(!bPartExists)
   {
      QByteArray _dest = QFile::encodeName(dest_part);
      int fd = -1;
      mode_t initialMode;
      if (m_permissions != -1)
         initialMode = m_permissions | S_IWUSR | S_IRUSR;
      else
         initialMode = 0666;

      fd = KDE_open(_dest.data(), O_CREAT | O_TRUNC | O_WRONLY, initialMode);
      if ( fd < 0 )
      {
          kDebug(7007) << "MultiSegmentCopyJob::checkLocalFile() error" << endl;
/*          if ( errno == EACCES )
            error( ERR_WRITE_ACCESS_DENIED, dest_part );
          else
            error( ERR_CANNOT_OPEN_FOR_WRITING, dest_part );*/
          return false;
      }
      else
      {
         close(fd);
      }
   }
   m_dest_part = m_dest;
   m_dest_part.setPath(dest_part);
   kDebug(7007) << "MultiSegmentCopyJob::checkLocalFile() success" << endl;
   return true;
}

GetJobManager::GetJobManager()
{
   job = 0;
   data.offset = 0;
   data.bytes = 0;
   restarting = false;
}

QByteArray GetJobManager::getData()
{
   QByteArray _data;
   while (!chunks.isEmpty())
      _data.append( chunks.dequeue() );
   kDebug(7007) << "GetJobManager::getData() " << _data.size() << endl;
   return _data;
}

void GetJobManager::slotOpen(KIO::Job *)
{
   kDebug(7007) << "GetJobManager::slotOpen() getjob: " << job << " offset: " << data.offset <<" bytes: "<< data.bytes << endl;
   job->seek( data.offset);
   job->read( data.bytes);
}

void GetJobManager::slotData( KIO::Job *job, const QByteArray &data)
{
   kDebug(7007) << "GetJobManager::slotData() " << job << " -- "<< data.size() << endl;
   chunks.enqueue( data );
   emit hasData (this);
}

void GetJobManager::slotCanWrite()
{
   if(!chunks.isEmpty())
      emit hasData (this);

   if(data.bytes && !job && !restarting)
   {
   kDebug(7007) << "emiting segment data: " << data.src << " offset: " << data.offset << " bytes: "<< data.bytes << endl;
   emit segmentData(data);
   restarting = true;
   }
}

void GetJobManager::slotResult( KJob * )
{
   disconnect(job);
   job = 0;
}

MultiSegmentCopyJob *KIO::MultiSegfile_copy( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, uint segments)
{
   return new MultiSegmentCopyJob( src, dest, permissions, showProgressInfo, segments);
}

MultiSegmentCopyJob *KIO::MultiSegfile_copy(
                           const KUrl& src,
                           const KUrl& dest,
                           int permissions,
                           bool showProgressInfo,
                           qulonglong ProcessedSize,
                           KIO::filesize_t totalSize,
                           QList<struct MultiSegData> segments)
{
   return new MultiSegmentCopyJob( src, dest, permissions, showProgressInfo, ProcessedSize, totalSize, segments);
}

#include "MultiSegKio.moc"
