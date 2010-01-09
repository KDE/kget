/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#ifndef MIRRORSEARCHTRANSFERDATASOURCE_H
#define MIRRORSEARCHTRANSFERDATASOURCE_H

#include "core/transferdatasource.h"

#include <kio/job.h>

class MirrorSearchTransferDataSource : public TransferDataSource
{
    Q_OBJECT
    public:
        MirrorSearchTransferDataSource(const KUrl &srcUrl, QObject *parent);

        void start();
        void stop();
        void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);

    private Q_SLOTS:
        void slotSearchUrls(QList<KUrl>& Urls);

    private:
        QString m_filename;
};

#endif
