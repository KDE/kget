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
    MultiSegKioDataSource(const QUrl &srcUrl, QObject *parent);
    ~MultiSegKioDataSource() override;

    void start() override;
    void stop() override;

    void findFileSize(KIO::fileoffset_t segmentSize) override;
    void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange) override;
    QPair<int, int> removeConnection() override;
    QList<QPair<int, int>> assignedSegments() const override;
    int countUnfinishedSegments() const override;
    QPair<int, int> split() override;

    void setSupposedSize(KIO::filesize_t supposedSize) override;
    int currentSegments() const override;

private Q_SLOTS:
    void slotSpeed(ulong speed) override;
    void slotFinishedSegment(Segment *segment, int segmentNum, bool connectionFinished);
    void slotRestartBrokenSegment();

    /**
     * There was an error while downloading segment, the number of connections this
     * TransferDataSource uses simultaneously gets reduced
     */
    void slotError(Segment *segment, const QString &errorText, Transfer::LogLevel logLevel);

    /**the following slots are there to check if the size reported by the mirror
     * Checks if the sizre reported by the mirror is correct
     */
    void slotTotalSize(KIO::filesize_t size, const QPair<int, int> &range = qMakePair(-1, -1));

    void slotCanResume();

    void slotFinishedDownload(KIO::filesize_t size);

    void slotUrlChanged(const QUrl &url);

private:
    Segment *mostUnfinishedSegments(int *unfinished = nullptr) const;
    bool tryMerge(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);

private:
    QList<Segment *> m_segments;
    KIO::filesize_t m_size;
    bool m_canResume;
    bool m_started;
};

#endif
