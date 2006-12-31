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

#include "segmentfactory.h"

/**
  * class MultiSegmentCopyJob
  */

class KIO_EXPORT MultiSegmentCopyJob : public KIO::Job
{
    Q_OBJECT

public:

    /**
     * Do not create a MultiSegmentCopyJob directly. Use MultiSegfile_copy() instead.
     * @param src the source URL
     * @param dest the destination URL
     * @param permissions the permissions of the resulting resource
     * @param showProgressInfo true to show progress information to the user
     * @param segments number of segments
     */
    MultiSegmentCopyJob( const QList<KUrl> Urls, const KUrl& dest, int permissions, bool showProgressInfo, uint segments);

    /**
     * Do not create a MultiSegmentCopyJob directly. Use MultiSegfile_copy() instead.
     * @param src the source URL
     * @param dest the destination URL
     * @param permissions the permissions of the resulting resource
     * @param showProgressInfo true to show progress information to the user
     * @param ProcessedSize
     * @param totalSize
     * @param segments a QList with segments data to resume a started copy
     */
    MultiSegmentCopyJob( const QList<KUrl> Urls, const KUrl& dest,
                          int permissions, bool showProgressInfo,
                          qulonglong ProcessedSize,
                          KIO::filesize_t totalSize,
                          QList<SegData> SegmentsData,
                          uint segments);

    QList<SegData> SegmentsData();
    void stop();

Q_SIGNALS:
    void updateSegmentsData();

public Q_SLOTS:
    void slotDataReq( Segment *, const QByteArray &data, bool &result);

private Q_SLOTS:

    void slotStart();
    void slotOpen( KIO::Job * );
    void slotWritten( KIO::Job * ,KIO::filesize_t bytesWritten);
    void slotClose( KIO::Job * );

    /**
     * Called whenever a subjob finishes.
     * @param job the job that emitted this signal
     */
    void slotResult( KJob *job );

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

    /**
     * Forward signal from subjob
     * @param job the job that emitted this signal
     * @param bytes_per_second the speed
     */
    void slotSpeed( KIO::Job*, unsigned long bytes_per_second );

private:
    KUrl m_dest;
    KUrl m_dest_part;
    int m_permissions;
    SegmentFactory *SegFactory;
    KIO::FileJob* m_putJob;
    bool m_writeBlocked;
    QHash <KIO::Job*, unsigned long>speedHash;
    unsigned long m_speed;

private:
    bool checkLocalFile();
};

    MultiSegmentCopyJob *MultiSegfile_copy( const QList<KUrl> Urls, const KUrl& dest, int permissions, bool showProgressInfo, uint segments);

    MultiSegmentCopyJob *MultiSegfile_copy(
                           const QList<KUrl> Urls,
                           const KUrl& dest,
                           int permissions,
                           bool showProgressInfo,
                           KIO::filesize_t ProcessedSize,
                           KIO::filesize_t totalSize,
                           QList<SegData> SegmentsData,
                           uint segments);


#endif // MULTISEGKIO_H
