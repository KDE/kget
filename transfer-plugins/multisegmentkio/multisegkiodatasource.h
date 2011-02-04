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
        MultiSegKioDataSource(const KUrl &srcUrl, QObject *parent);
        ~MultiSegKioDataSource();

        void start();
        void stop();

        void findFileSize(KIO::fileoffset_t segmentSize);
        void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);
        QPair<int, int> removeConnection();
        QList<QPair<int, int> > assignedSegments() const;
        int countUnfinishedSegments() const;
        QPair<int, int> split();

        void setSupposedSize(KIO::filesize_t supposedSize);
        int currentSegments() const;

    private Q_SLOTS:
        void slotSpeed(ulong speed);
        void slotFinishedSegment(Segment *segment, int segmentNum, bool connectionFinished);
        void slotRestartBrokenSegment();

        /**
         * There was an error while downloading segment, the number of connections this
         * TransferDataSource uses simultanously gets reduced
         */
        void slotError(Segment *segment, const QString &errorText, Transfer::LogLevel logLevel);

        /**the following slots are there to check if the size reported by the mirror
         * Checks if the sizre reported by the mirror is correct
         */
        void slotTotalSize(KIO::filesize_t size, const QPair<int, int> &range = qMakePair(-1, -1));

        void slotCanResume();

        void slotFinishedDownload(KIO::filesize_t size);

    private:
        Segment *mostUnfinishedSegments(int *unfinished = 0) const;
        bool tryMerge(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int,int> &segmentRange);

    private:
        QList<Segment*> m_segments;
        KIO::filesize_t m_size;
        bool m_canResume;
        bool m_started;
};

#endif
