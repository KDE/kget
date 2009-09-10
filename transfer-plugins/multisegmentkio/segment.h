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

        Segment(const KUrl &src, const KIO::fileoffset_t offset, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange, QObject *parent);

        ~Segment();

        /**
         * Create the segment transfer
         */
        bool createTransfer();

        /**
         * stop the segment transfer
         */
        bool stopTransfer ( );

        void setData(KIO::fileoffset_t offset, KIO::filesize_t bytes);

        /**
         * Get the value of m_offset set
         */
        KIO::fileoffset_t offset() const {return m_offset;}

        /**
         * Returns the size the current segment has
         * @return the size of the segment
         */
        KIO::filesize_t size() const {return m_curentSegSize;}

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
        int countUnfinishedSegments() const;
        int takeOneSegment();
        QPair<int, int> split();

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

        void brokenSegments(Segment *segment, QPair<int, int> segmentRange);
        void finishedSegment(Segment *segment, int segmentNum, bool connectionFinished = true);
        void statusChanged( Segment*);
        void speed(ulong speed);

    private Q_SLOTS:
        void slotData(KIO::Job *, const QByteArray& data);
        void slotCanResume(KIO::Job *, KIO::filesize_t);//TODO remove

    private:
        bool writeBuffer();
        void setStatus(Status stat, bool doEmit=true);

    private:
        Status m_status;
        KIO::fileoffset_t m_offset;
        QPair<KIO::fileoffset_t, KIO::fileoffset_t> m_segSize;
        KIO::fileoffset_t m_curentSegSize;
        int m_curentSegment;
        int m_endSegment;
        KIO::filesize_t m_bytesWritten;
        KIO::filesize_t m_totalBytesLeft;
        KIO::TransferJob *m_getJob;
        QByteArray m_buffer;
        bool m_canResume;
        KUrl m_url;
        int m_restarted;
};

#endif // SEGMENT_H
