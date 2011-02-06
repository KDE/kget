/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef SEGMENT_H
#define SEGMENT_H

#include <QtCore/QObject>

#include <KIO/Job>

#include "core/transfer.h"

/**
 * class Segment
 */

class Segment : public QObject
{
    Q_OBJECT

    public:
        /**
         * The status property describes the current segment status
         *
         * @param Running The transfer is being executed
         * @param Stopped The transfer is stopped
         * @param Killed The transfer have been killed due unhandled errors 
         * @param Timeout The transfer is broken because an error ocoured
         * @param Finished The transfer exited successfully
         */
        enum Status
        {
            Running,
            Stopped,
            Killed,
            Timeout,
            Finished
        };

        Segment(const KUrl &src, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange, QObject *parent);

        ~Segment();

        /**
         * Create the segment transfer
         */
        bool createTransfer();

        /**
         * stop the segment transfer
         */
        bool stopTransfer ( );

        /**
         * Get the value of m_offset set
         */
        KIO::fileoffset_t offset() const {return m_offset;}

        /**
         * Returns the size the current segment has
         * @return the size of the segment
         */
        KIO::filesize_t size() const {return m_currentSegSize;}

        /**
         * Returns the written bytes
         * @return the value of m_bytesWritten
         */
        KIO::filesize_t BytesWritten ( ){ return m_bytesWritten; }//TODO needed???

        /**
         * Get the job
         * @return the value of m_getJob
         */
        KIO::TransferJob *job(){ return m_getJob; }//TODO needed?

        /**
         * Get the segment status
         * @return the value of m_status
         */
        Status status() const {return m_status;}//TODO needed?

        QPair<int, int> assignedSegments() const;
        QPair<KIO::fileoffset_t, KIO::fileoffset_t> segmentSize() const;
        int countUnfinishedSegments() const;
        QPair<int, int> split();
        bool merge(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int,int> &segmentRange);
        bool findingFileSize() const;

    public Q_SLOTS:
        /**
         * start the segment transfer
         */
        bool startTransfer ( );

        /**
         * Called whenever a subjob finishes
         * @param job the job that emitted this signal
         */
        void slotResult( KJob *job );

    Q_SIGNALS:
        void data(KIO::fileoffset_t offset, const QByteArray &data, bool &worked);
        /**
         * Emitted whenever the transfer is closed with an error
         */
        void error(Segment *segment, const QString &errorText, Transfer::LogLevel logLevel);
        void finishedSegment(Segment *segment, int segmentNum, bool connectionFinished = true);
        void statusChanged( Segment*);
        void speed(ulong speed);
        void connectionProblem();
        void totalSize(KIO::filesize_t size, QPair<int, int> segmentRange);
        void finishedDownload(KIO::filesize_t size);
        void canResume();

    private Q_SLOTS:
        void slotData(KIO::Job *job, const QByteArray &data);
        void slotCanResume(KIO::Job *job, KIO::filesize_t);//TODO remove
        void slotTotalSize(KJob *job, qulonglong size);

        /**
         * Writes the buffer, assuming that this segment is finished and the rest should be written.
         * Tries to write and if that fails waits for 50 msec and retries again.
         * If this whole process still fails after 100 times, then error is emitted.
         */
        void slotWriteRest();

    private:
        bool writeBuffer();
        void setStatus(Status stat, bool doEmit=true);

    private:
        bool m_findFilesize;
        bool m_canResume;
        Status m_status;
        int m_currentSegment;
        int m_endSegment;
        int m_errorCount;
        KIO::fileoffset_t m_offset;
        KIO::fileoffset_t m_currentSegSize;
        KIO::filesize_t m_bytesWritten;
        KIO::filesize_t m_totalBytesLeft;
        KIO::TransferJob *m_getJob;
        KUrl m_url;
        QByteArray m_buffer;
        QPair<KIO::fileoffset_t, KIO::fileoffset_t> m_segSize;
};

#endif // SEGMENT_H
