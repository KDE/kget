/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2005 Joris Guisson <joris.guisson@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btchunkselector.h"

#include <algorithm>
#include <cstdlib>
#include <vector>

#include <diskio/chunkmanager.h>
#include <download/downloader.h>
#include <interfaces/piecedownloader.h>
#include <peer/chunkcounter.h>
#include <peer/peer.h>
#include <peer/peermanager.h>
#include <util/bitset.h>
#include <util/log.h>

using namespace bt;

BTChunkSelector::BTChunkSelector(ChunkManager &cman, Downloader &downer, PeerManager &pman)
{
    this->ChunkSelector::init(&cman, &downer, &pman);
}

BTChunkSelector::~BTChunkSelector()
{
}

BTChunkSelectorFactory::BTChunkSelectorFactory()
{
}

BTChunkSelectorFactory::~BTChunkSelectorFactory()
{
}

bt::ChunkSelectorInterface *BTChunkSelectorFactory::createChunkSelector(bt::ChunkManager &cman, bt::Downloader &downer, bt::PeerManager &pman)
{
    BTChunkSelector *selector = new BTChunkSelector(cman, downer, pman);
    emit selectorAdded(selector);
    return selector;
}
