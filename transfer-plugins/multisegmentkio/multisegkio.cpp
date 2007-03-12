/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kde_file.h>

#include <QFile>
#include <qtimer.h>

#include <sys/time.h>
#include <fcntl.h>

#include "multisegkio.h"

static const unsigned int max_nums = 8;
class MultiSegmentCopyJob::MultiSegmentCopyJobPrivate
{
public:
    MultiSegmentCopyJobPrivate() {
        start_time.tv_sec = 0;
        start_time.tv_usec = 0;
        last_time = 0;
        nums = 0;
        bytes = 0;
    }
    struct timeval start_time;
    uint nums;
    long times[max_nums];
    KIO::filesize_t sizes[max_nums];
    size_t last_time;
    KIO::filesize_t bytes;

    QTimer speed_timer;
};


/**
  * class MultiSegmentCopyJob
  */
MultiSegmentCopyJob::MultiSegmentCopyJob( const QList<KUrl> Urls, const KUrl& dest, int permissions, uint segments)
   :d(new MultiSegmentCopyJobPrivate),
    KJob(0), m_dest(dest),
    m_permissions(permissions),
    m_writeBlocked(false)
{
    kDebug(5001) << "MultiSegmentCopyJob::MultiSegmentCopyJob()" << endl;
    QList<SegData> emptySegData;
    SegFactory = new SegmentFactory( segments, Urls, emptySegData );
    m_putJob = 0;
    connect(&d->speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

MultiSegmentCopyJob::MultiSegmentCopyJob(
                           const QList<KUrl> Urls,
                           const KUrl& dest,
                           int permissions,
                           qulonglong ProcessedSize,
                           KIO::filesize_t totalSize,
                           QList<SegData> SegmentsData,
                           uint segments)

   :d(new MultiSegmentCopyJobPrivate),
    KJob(0), m_dest(dest),
    m_permissions(permissions),
    m_writeBlocked(false)
{
    kDebug(5001) << "MultiSegmentCopyJob::MultiSegmentCopyJob()" << endl;
    SegFactory = new SegmentFactory( segments, Urls, SegmentsData );
    QList<Segment *> Segments = SegFactory->Segments();
    QList<Segment *>::iterator it = Segments.begin();
    QList<Segment *>::iterator itEnd = Segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        kDebug(5001) << "MultiSegmentCopyJob::MultiSegmentCopyJob() conecting job Signals" << endl;
        connect( (*it), SIGNAL(data( Segment*, const QByteArray&, bool &)),
                 SLOT(slotDataReq( Segment *, const QByteArray&, bool &)));
        connect( (*it)->job(), SIGNAL(speed( KJob*, unsigned long )),
                 SLOT(slotSpeed( KJob*, unsigned long )));
        connect( (*it), SIGNAL(updateSegmentData()),
                 SIGNAL(updateSegmentsData()));
    }

    m_putJob = 0;
    connect(&d->speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
    setProcessedSize(ProcessedSize);
    setTotalSize(totalSize);
    QTimer::singleShot(0, this, SLOT(slotStart()));
}

MultiSegmentCopyJob::~MultiSegmentCopyJob()
{
    kDebug(5001) << "MultiSegmentCopyJob::destructor()" << endl;
    SegFactory->deleteLater();
}

QList<SegData> MultiSegmentCopyJob::SegmentsData()
{
    return SegFactory->SegmentsData();
}

void MultiSegmentCopyJob::stop()
{
    kDebug(5001) << "MultiSegmentCopyJob::stop()" << endl;
    setError(KIO::ERR_USER_CANCELED);
    if (SegFactory)
        SegFactory->stopTransfer();
    if (m_putJob)
        m_putJob->close();
}

void MultiSegmentCopyJob::slotStart()
{
    kDebug(5001) << "MultiSegmentCopyJob::slotStart()" << endl;
    if( !checkLocalFile() )
        emitResult();

    kDebug(5001) << "MultiSegmentCopyJob::slotStart() opening: " << m_dest_part << endl;
    m_putJob = KIO::open(m_dest_part, 3);
    connect( m_putJob, SIGNAL(open(KIO::Job *)), SLOT(slotOpen(KIO::Job *)));
    connect(m_putJob, SIGNAL(close(KIO::Job *)), SLOT(slotClose(KIO::Job *)));
    connect( m_putJob, SIGNAL(written(KIO::Job * ,KIO::filesize_t )), SLOT(slotWritten( KIO::Job * ,KIO::filesize_t )));
    connect( m_putJob, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));
}

void MultiSegmentCopyJob::slotOpen( KIO::Job * job)
{
    kDebug(5001) << "MultiSegmentCopyJob::slotOpen()" << endl;
    if( SegFactory->startTransfer() )
    {

        gettimeofday(&d->start_time, 0);
        d->last_time = 0;
        d->sizes[0] = processedSize() - d->bytes;
        d->times[0] = 0;
        d->nums = 1;
        d->speed_timer.start(1000);

        return;
    }
    SegData data;
    Segment *seg = SegFactory->createSegment(data, SegFactory->nextUrl() );

    connect( seg, SIGNAL(data( Segment*, const QByteArray&, bool &)), 
                 SLOT(slotDataReq( Segment *, const QByteArray&, bool &)));
    connect( seg, SIGNAL(updateSegmentData()),
                 SIGNAL(updateSegmentsData()));
    connect( seg->job(), SIGNAL(speed( KJob*, unsigned long )),
                 SLOT(slotSpeed( KJob*, unsigned long )));
    connect( seg->job(), SIGNAL(totalSize( KJob *, qulonglong )),
                      SLOT(slotTotalSize( KJob *, qulonglong )));
    seg->startTransfer();
}

void MultiSegmentCopyJob::slotWritten( KIO::Job * ,KIO::filesize_t bytesWritten)
{
//     kDebug(5001) << "MultiSegmentCopyJob::slotWritten() " << bytesWritten << endl;
    m_writeBlocked = false;
    setProcessedSize(processedSize()+bytesWritten);
    if( processedSize() == totalSize() )
        m_putJob->close();
}

void MultiSegmentCopyJob::slotClose( KIO::Job * )
{
    kDebug(5001) << "MultiSegmentCopyJob::slotClose() putjob" << endl;
    if( processedSize() == totalSize() )
    {
        kDebug(5001) << "Renaming local file." << endl;
       QString dest_orig = m_dest.path();
       QString dest_part = m_dest_part.path();
       QFile::rename ( dest_part, dest_orig );
    }
    emit updateSegmentsData();
}

// tooked from SlaveInterface.cpp
void MultiSegmentCopyJob::calcSpeed()
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    long diff = ((tv.tv_sec - d->start_time.tv_sec) * 1000000 +
	         tv.tv_usec - d->start_time.tv_usec) / 1000;
    if (diff - d->last_time >= 900) {
        d->last_time = diff;
    if (d->nums == max_nums) {
      // let's hope gcc can optimize that well enough
      // otherwise I'd try memcpy :)
        for (unsigned int i = 1; i < max_nums; ++i) {
	   d->times[i-1] = d->times[i];
	   d->sizes[i-1] = d->sizes[i];
        }
        d->nums--;
    }
    d->times[d->nums] = diff;
    d->sizes[d->nums++] = processedSize() - d->bytes;

    KIO::filesize_t lspeed = 1000 * (d->sizes[d->nums-1] - d->sizes[0]) / (d->times[d->nums-1] - d->times[0]);

    if (!lspeed) {
      d->nums = 1;
      d->times[0] = diff;
      d->sizes[0] = processedSize() - d->bytes;
    }
    emit speed(this, lspeed);
  }
}

void MultiSegmentCopyJob::slotDataReq( Segment *seg, const QByteArray &data, bool &result)
{
//     kDebug(5001) << "MultiSegmentCopyJob::slotDataReq() " << endl;
    if ( m_writeBlocked )
    {
        result = false;
        return;
    }
    m_writeBlocked = true;
    m_putJob->seek(seg->offset());
    m_putJob->write(data);
    result = true;
}

void MultiSegmentCopyJob::slotResult( KJob *job )
{
    kDebug(5001) << "MultiSegmentCopyJob::slotResult()" << job <<endl;

    if( job->error() )
    {
        setError( job->error() );
        setErrorText( job->errorText() );
    }

    if (job == m_putJob )
    {
        kDebug(5001) << "MultiSegmentCopyJob: m_putJob finished " << endl;
        kDebug(5001) << "MultiSegmentCopyJob: finished " << endl;
        m_putJob = 0;
        emitResult();
    }
}

void MultiSegmentCopyJob::slotTotalSize( KJob *job, qulonglong size )
{
    kDebug(5001) << "MultiSegmentCopyJob::slotTotalSize() from job: " << job << " -- " << size << endl;
    setTotalSize (size);

    gettimeofday(&d->start_time, 0);
    d->last_time = 0;
    d->sizes[0] = processedSize() - d->bytes;
    d->times[0] = 0;
    d->nums = 1;
    d->speed_timer.start(1000);

    QList<Segment *> segments = SegFactory->Segments();
    Segment *seg = segments.takeFirst();
    seg->setBytes(size);
    segments = SegFactory->splitSegment( seg ,SegFactory->nunOfSegments() );
    QList<Segment *>::iterator it = segments.begin();
    QList<Segment *>::iterator itEnd = segments.end();
    for ( ; it!=itEnd ; ++it )
    {
        connect( (*it), SIGNAL(data( Segment*, const QByteArray&, bool &)),
                 SLOT(slotDataReq( Segment *, const QByteArray&, bool &)));
        connect( (*it)->job(), SIGNAL(speed( KJob*, unsigned long )),
                 SLOT(slotSpeed( KJob*, unsigned long )));
        connect( (*it), SIGNAL(updateSegmentData()),
                 SIGNAL(updateSegmentsData()));
        (*it)->startTransfer();
    }
}

void MultiSegmentCopyJob::slotPercent( KJob *job, unsigned long pct )
{
}

void MultiSegmentCopyJob::slotSpeed( KJob* job, unsigned long bytes_per_second )
{
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
             kDebug(5001) << "MultiSegmentCopyJob::checkLocalFile() error" << endl;
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
    kDebug(5001) << "MultiSegmentCopyJob::checkLocalFile() success" << endl;
    return true;
}

MultiSegmentCopyJob *MultiSegfile_copy( const QList<KUrl> Urls, const KUrl& dest, int permissions, uint segments)
{
    return new MultiSegmentCopyJob( Urls, dest, permissions, segments);
}

MultiSegmentCopyJob *MultiSegfile_copy(
                           const QList<KUrl> Urls,
                           const KUrl& dest,
                           int permissions,
                           qulonglong ProcessedSize,
                           KIO::filesize_t totalSize,
                           QList<SegData> SegmentsData,
                           uint segments)
{
    return new MultiSegmentCopyJob( Urls, dest, permissions,  ProcessedSize, totalSize,SegmentsData , segments);
}

#include "multisegkio.moc"
