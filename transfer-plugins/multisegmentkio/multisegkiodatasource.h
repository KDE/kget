/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGET_MULTISEGKIODATASOURCE_H
#define KGET_MULTISEGKIODATASOURCE_H

#include "core/transferdatasource.h"

class Segment;

class MultiSegKioDataSource : public TransferDataSource
{
    Q_OBJECT

    public:
        MultiSegKioDataSource(const KUrl &srcUrl);
        ~MultiSegKioDataSource();

        void start();
        void stop();

        bool canHandleMultipleSegments() const;
        void addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum);
        void addSegments(const KIO::fileoffset_t offset, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);
        QPair<int, int> removeConnection();
        QList<QPair<int, int> > assignedSegments() const;
        int countUnfinishedSegments() const;
        int takeOneSegment();
        QPair<int, int> split();

        void setSupposedSize(KIO::filesize_t supposedSize);

    private Q_SLOTS:
        void slotSpeed(ulong speed);
        void slotBrokenSegment(Segment *segment, int segmentNum);
        void slotFinishedSegment(Segment *segment, int segmentNum, bool connectionFinished);

        /**the following slots are there to check if the size reported by the mirror
         * Checks if the sizre reported by the mirror is correct
         */
        void slotTotalSize(KJob *job, qulonglong size);

        /**
         * This method is only there to check if more than 100 kb have
         * been downloaded to stop it afterwards
         */
        void slotInitData(KIO::Job *job, const QByteArray &data);

        /**
         * This method is only there to check if the job supports
         * resuming
         */
        void slotCanResume(KIO::Job *job, KIO::filesize_t offset);

        void slotInitResult(KJob *job);

    private:
        Segment *mostUnfinishedSegments(int *unfinished = 0) const;
        void killInitJob();

    private:
        QList<Segment*> m_segments;
        KIO::filesize_t m_size;
        bool m_canResume;
        KIO::TransferJob *m_getInitJob;
        bool m_hasInitJob;
        bool m_started;
};

#endif
