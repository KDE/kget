/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#ifndef BTDATASOURCE_H
#define BTDATASOURCE_H

#include "core/transferdatasource.h"

#include <kio/job.h>
#include <KUrl>
#include <QTimer>

namespace bt
{
    class TorrentControl;
}

class BTChunkSelectorFactory;
class BTChunkSelector;
class BTCacheFactory;
class BTCache;

class BTDataSource : public TransferDataSource
{
    Q_OBJECT
    public:
        BTDataSource(const KUrl &srcUrl, QObject *parent);
        ~BTDataSource();

        void start();
        void stop();
        void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);
        void getData(const KIO::fileoffset_t &off, const QByteArray &dataArray);

    private slots:
        void init(const KUrl &torrentSource, const QByteArray &data);
        void cacheAdded(BTCache *cache);
        void selectorAdded(BTChunkSelector *selector);
        void update();

    private:
        bt::TorrentControl *tc;
        BTChunkSelectorFactory *csf;
        BTChunkSelector *cs;
        BTCacheFactory *cf;

        KIO::fileoffset_t m_offset;
        KIO::fileoffset_t m_bytes;
        KUrl m_torrentSource;
        QTimer timer;
};

#endif
