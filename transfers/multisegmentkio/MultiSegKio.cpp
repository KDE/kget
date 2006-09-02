/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include "MultiSegKio.h"
#include <kio/observer.h>
#include <kio/slave.h>
#include <kio/global.h>

#include <klocale.h>

#include <qtimer.h>
#include <QDateTime>
#include <assert.h>

#ifdef Q_OS_UNIX
#include <utime.h>
#endif

using namespace KIO;

/**
**/
MultiSegmentCopyJob::MultiSegmentCopyJob( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, uint segments)
    : Job(showProgressInfo), m_src(src), m_dest(dest), m_permissions(permissions), m_segments(segments)
{
   if (showProgressInfo)
      Observer::self()->slotCopying( this, src, dest );

   kDebug(7007) << "MultiSegmentCopyJob::MultiSegmentCopyJob()" << endl;
   blockWrite = false;
   m_getJob = 0;
   m_putJob = 0;
   QTimer::singleShot(0, this, SLOT(slotStart()));
}

void MultiSegmentCopyJob::add( FileJob* job, KIO::filesize_t offset, KIO::filesize_t bytes)
{
   GetJobManager *jobManager = new GetJobManager();
   jobManager->job = job;
   jobManager->offset = offset;
   jobManager->bytes = bytes;
   m_jobs.append(jobManager);
   connect( job, SIGNAL(data( KIO::Job *, const QByteArray & )), jobManager, SLOT(slotData( KIO::Job *, const QByteArray &)));
   connect( jobManager, SIGNAL(hasData( GetJobManager * )), SLOT(slotDataReq( GetJobManager *)));
}

void MultiSegmentCopyJob::slotStart()
{
   kDebug(7007) << "MultiSegmentCopyJob::slotStart()" << endl;
   m_putJob = KIO::open(m_dest, 3);
   connect( m_putJob, SIGNAL(open(KIO::Job *)), SLOT(slotOpen(KIO::Job *)));
   connect( m_putJob, SIGNAL(written(KIO::Job * ,KIO::filesize_t )), SLOT(slotWritten( KIO::Job * ,KIO::filesize_t )));
   addSubjob( m_putJob );
}

void MultiSegmentCopyJob::slotOpen( KIO::Job * job)
{
   if( job==m_putJob )
   {
      kDebug(7007) << "MultiSegmentCopyJob::slotOpen() putjob" << endl;
      FileJob* getjob = KIO::open(m_src, 1);
      connect(getjob, SIGNAL(open(KIO::Job *)), SLOT(slotOpen(KIO::Job *)));
      addSubjob( getjob );
   }
   else
   {
      FileJob* getjob = static_cast<FileJob*>(job);
      kDebug(7007) << "MultiSegmentCopyJob::slotOpen() getjob: " << getjob->size() << endl;
      KIO::filesize_t size = getjob->size();
      uint min = size/MIN_SIZE;
      if(min < m_segments)
      {
         m_segments = min;
      }
      if(min == 0)
      {
         m_segments = 1;
      }

      KIO::filesize_t segment = size/m_segments;
      KIO::fileoffset_t rest_size = segment + (size%m_segments);
      add( getjob, 0, segment);
      slotOpenGetJob( getjob);
      for( uint i = 1; i < m_segments; i++)
      {
         if(i == m_segments - 1)
         {
            getjob = KIO::open(m_src, 1);
            add( getjob, i*segment, rest_size);
            connect(getjob, SIGNAL(open(KIO::Job *)), SLOT(slotOpenGetJob(KIO::Job *)));
            addSubjob( getjob );
            continue;
         }
            getjob = KIO::open(m_src, 1);
            add( getjob, i*segment, segment);
            connect(getjob, SIGNAL(open(KIO::Job *)), SLOT(slotOpenGetJob(KIO::Job *)));
            addSubjob( getjob );
      }
   }

}

void MultiSegmentCopyJob::slotOpenGetJob(KIO::Job * job)
{
   QList <GetJobManager *>::const_iterator it = m_jobs.begin();
   QList <GetJobManager *>::const_iterator itEnd = m_jobs.end();
   for ( ; it!=itEnd ; ++it )
   {
      if( (*it)->job == job )
      {
         kDebug(7007) << "MultiSegmentCopyJob::slotOpenGetJob() getjob: " << (*it)->job << " offset: " << (*it)->offset <<" bytes: "<< (*it)->bytes << endl;
         (*it)->job->seek((*it)->offset);
         (*it)->job->read((*it)->bytes);
      }
   }
}

void MultiSegmentCopyJob::slotDataReq( GetJobManager *jobManager)
{
   kDebug(7007) << "MultiSegmentCopyJob::slotDataReq()" << endl;
   if ( blockWrite )
      return;
   kDebug(7007) << "MultiSegmentCopyJob::slotDataReq() putjob writing..." << endl;
   blockWrite = true;
   m_getJob = jobManager;
   m_putJob->seek( jobManager->offset );
   m_putJob->write( jobManager->getData() );
}

void MultiSegmentCopyJob::slotWritten( KIO::Job * ,KIO::filesize_t bytesWritten)
{
   kDebug(7007) << "MultiSegmentCopyJob::slotWritten() " << bytesWritten << endl;
   if( blockWrite && m_getJob )
   {
      m_getJob->offset = m_getJob->offset + bytesWritten;
      m_getJob->bytes = m_getJob->bytes - bytesWritten;
      m_getJob = 0;
      blockWrite = false;
   }
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
      kDebug(7007) << "MFileCopyJob: m_putJob finished " << endl;
      m_putJob = 0;
      removeSubjob(job);
   }
   else //is a get job
   {
      kDebug(7007) << "MFileCopyJob: getJob finished " << endl;
      removeSubjob(job);
   }

   if ( !hasSubjobs() )
       emitResult();
}

void MultiSegmentCopyJob::slotProcessedSize( KJob *job, qulonglong size )
{
}

void MultiSegmentCopyJob::slotTotalSize( KJob *job, qulonglong size )
{
   kDebug(7007) << "MultiSegmentCopyJob::slotTotalSize() " << size << endl;
   emit totalSize(this, size);
}

void MultiSegmentCopyJob::slotPercent( KJob *job, unsigned long pct )
{
}

GetJobManager::GetJobManager()
{
   job = 0;
   offset = 0;
   bytes = 0;
};

QByteArray GetJobManager::getData()
{
   QByteArray data;
   while (!chunks.isEmpty())
      data.append( chunks.dequeue() );
   return data;
}

void GetJobManager::slotData( KIO::Job *job, const QByteArray &data)
{
   kDebug(7007) << "GetJobManager::slotData() " << job << " -- "<< data.size() << endl;
   chunks.enqueue( data );
   emit hasData (this);
}


MultiSegmentCopyJob *KIO::MultiSegfile_copy( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, uint segments)
{
   return new MultiSegmentCopyJob( src, dest, permissions, showProgressInfo, segments);
}

#include "MultiSegKio.moc"
