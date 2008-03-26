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
#include <interfaces/chunkselectorinterface.h>
#include <QObject>

namespace bt
{
    class BitSet;
    class ChunkManager;
    class Downloader;
    class PeerManager;
    class PieceDownloader;
}

class BTChunkSelector : public bt::ChunkSelectorInterface 
{
    public:
        BTChunkSelector(bt::ChunkManager & cman,bt::Downloader & downer,bt::PeerManager & pman);
        ~BTChunkSelector();

        virtual bool select(bt::PieceDownloader* pd,bt::Uint32 & chunk);
        virtual void dataChecked(const bt::BitSet & ok_chunks);
        virtual void reincluded(bt::Uint32 from, bt::Uint32 to);
        virtual void reinsert(bt::Uint32 chunk);
        virtual void excludeAll();
        virtual void exclude(bt::Uint32 chunk);

    private:
        bt::Uint32 leastPeers(const std::list<bt::Uint32> & lp);

        std::list<bt::Uint32> chunks;
        bt::Timer sort_timer;
};

class BTChunkSelectorFactory : public QObject, public bt::ChunkSelectorFactoryInterface
{
    Q_OBJECT
    public:
        BTChunkSelectorFactory();
        ~BTChunkSelectorFactory();

        bt::ChunkSelectorInterface* createChunkSelector(bt::ChunkManager & cman, bt::Downloader & downer, bt::PeerManager & pman);

    signals:
        void selectorAdded(BTChunkSelector *selector);
};

#endif
