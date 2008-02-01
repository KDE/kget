/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "multisegkiodatasource.h"
#include "segmentfactory.h"

#include <kdebug.h>

MultiSegKioDataSource::MultiSegKioDataSource()
:TransferDataSource(0) , m_SegFactory(new SegmentFactory)
{
    kDebug(5001);
}

void MultiSegKioDataSource::start()
{
    kDebug(5001);
}

void MultiSegKioDataSource::stop()
{
    kDebug(5001);
}

void MultiSegKioDataSource::addSegment(const KUrl &srcUrl, const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes)
{
    kDebug(5001);
    SegData data;
    data.offset = offset;
    data.bytes = bytes;
    Segment *seg = m_SegFactory->createSegment(data, srcUrl );
    connect( seg, SIGNAL(data( Segment*, const QByteArray&, bool &)),
                 SLOT(slotDataReq( Segment *, const QByteArray&, bool &)));
}

