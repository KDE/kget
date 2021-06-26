/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGETBTCHUNKSELECTOR_H
#define KGETBTCHUNKSELECTOR_H

#include <list>
#include <util/timer.h>
#include <util/constants.h>
#include <download/chunkselector.h>

#include <QObject>

namespace bt
{
    class BitSet;
    class ChunkManager;
    class Downloader;
    class PeerManager;
    class PieceDownloader;
}

class BTChunkSelector : public bt::ChunkSelector
{
    public:
        BTChunkSelector(bt::ChunkManager & cman,bt::Downloader & downer,bt::PeerManager & pman);
        ~BTChunkSelector();
};

class BTChunkSelectorFactory : public QObject, public bt::ChunkSelectorFactoryInterface
{
    Q_OBJECT
    public:
        BTChunkSelectorFactory();
        ~BTChunkSelectorFactory();

        bt::ChunkSelectorInterface* createChunkSelector(bt::ChunkManager & cman, bt::Downloader & downer, bt::PeerManager & pman);

    Q_SIGNALS:
        void selectorAdded(BTChunkSelector *selector);
};

#endif
