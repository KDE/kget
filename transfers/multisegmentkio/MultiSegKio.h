/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef MULTISEGKIO_H
#define MULTISEGKIO_H

#include <kio/filejob.h>
#include <kio/job.h>

#include <QQueue>

namespace KIO
{

#define	MIN_SIZE	1024*100

struct MultiSegData
{
    KUrl src;
    KIO::fileoffset_t offset;
    KIO::filesize_t bytes;
};

class GetJobManager;

class KIO_EXPORT MultiSegmentCopyJob : public Job
{
   Q_OBJECT

   public:

      /**
      * Do not create a MultiSegmentCopyJob directly. Use KIO::MultiSegfile_copy() instead.
      * @param src the source URL
      * @param dest the destination URL
      * @param permissions the permissions of the resulting resource
      * @param showProgressInfo true to show progress information to the user
      * @param segments number of segments
      */
      MultiSegmentCopyJob( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, uint segments);

      /**
      * Do not create a MultiSegmentCopyJob directly. Use KIO::MultiSegfile_copy() instead.
      * @param src the source URL
      * @param dest the destination URL
      * @param permissions the permissions of the resulting resource
      * @param showProgressInfo true to show progress information to the user
      * @param segments a QList with segments data to resume a started copy
      */
      MultiSegmentCopyJob( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, QList<struct MultiSegData> segments);

      ~MultiSegmentCopyJob() {};
      void addGetJobManagers( QList<struct MultiSegData> segments);
      void addFisrtGetJobManager( FileJob* job, KIO::filesize_t offset, KIO::filesize_t bytes);

    public Q_SLOTS:
        void slotStart();
        void slotOpen( KIO::Job * );
        void slotClose( KIO::Job * );
        void slotDataReq( GetJobManager *jobManager);
        void slotWritten( KIO::Job * ,KIO::filesize_t bytesWritten);

    protected Q_SLOTS:
        /**
         * Called whenever a subjob finishes.
	 * @param job the job that emitted this signal
         */
        virtual void slotResult( KJob *job );

        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the processed size in bytes
         */
        void slotProcessedSize( KJob *job, qulonglong size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param size the total size
         */
        void slotTotalSize( KJob *job, qulonglong size );
        /**
         * Forward signal from subjob
	 * @param job the job that emitted this signal
	 * @param pct the percentage
         */
        void slotPercent( KJob *job, unsigned long pct );

   protected:

      KUrl m_src;
      KUrl m_dest;
      KUrl m_dest_part;
      int m_permissions;
      uint m_segments;
      bool blockWrite;
      bool bcopycompleted;
      bool bstartsaved;
      FileJob* m_putJob;
      GetJobManager* m_getJob;
      QList <GetJobManager*> m_jobs;

   private:
      bool checkLocalFile();
};

class GetJobManager :public QObject
{
   Q_OBJECT

   public:
      GetJobManager();
      QByteArray getData();

   public Q_SLOTS:
      void slotOpen( KIO::Job * );
      void slotData( KIO::Job *, const QByteArray &data);
      void slotResult( KJob *job );

   Q_SIGNALS:
      void hasData (GetJobManager *);

   public:
      FileJob *job;
      KIO::filesize_t offset;
      KIO::filesize_t bytes;
      QQueue<QByteArray> chunks;
      bool doKill;
};

   MultiSegmentCopyJob *MultiSegfile_copy( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, uint segments);

   MultiSegmentCopyJob *MultiSegfile_copy( const KUrl& src, const KUrl& dest, int permissions, bool showProgressInfo, QList<struct MultiSegData> segments);

}

#endif //MULTISEGKIO_H
