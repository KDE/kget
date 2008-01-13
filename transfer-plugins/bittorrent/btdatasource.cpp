/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "btdatasource.h"
#include <kdebug.h>

BTDataSource::BTDataSource()
  :TransferDataSource(0)
{
   kDebug(5001);
    m_bitset = new BitSet();
}

void BTDataSource::start()
{
   kDebug(5001);
}

void BTDataSource::stop()
{
   kDebug(5001);
}

void BTDataSource::addSegment(const KUrl &srcUrl, const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes)
{
   Q_UNUSED(srcUrl);
   Q_UNUSED(offset);
   Q_UNUSED(bytes);
   kDebug(5001);
}

