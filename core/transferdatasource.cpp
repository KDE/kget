/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "transferdatasource.h"

#include <kdebug.h>

TransferDataSource::TransferDataSource(const KUrl &srcUrl, QObject *parent)
  : QObject(parent),
    m_sourceUrl(srcUrl),
    m_speed(0),
    m_supposedSize(0)
{
    kDebug(5001) ;
}

TransferDataSource::~TransferDataSource()
{
    kDebug(5001) ;
}

bool TransferDataSource::canHandleMultipleSegments() const
{
    return false;
}

void TransferDataSource::addSegments(const KIO::fileoffset_t offset, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    int k = 0;

    for (int i = segmentRange.first; i <= segmentRange.second; ++i)
    {
        KIO::fileoffset_t bytes = segmentSize.first;
        if (i == segmentRange.second)
        {
            bytes = segmentSize.second;
        }
        addSegment(offset + k * segmentSize.first, bytes, i);
        ++k;
    }
}

QPair<int, int> TransferDataSource::removeConnection()
{
    return QPair<int, int>(-1, -1);
}

QList<QPair<int, int> > TransferDataSource::assignedSegments() const
{
    return QList<QPair<int, int> >();
}

int TransferDataSource::countUnfinishedSegments() const
{
    return 0;
}

int TransferDataSource::takeOneSegment()
{
    return -1;
}

QPair<int, int> TransferDataSource::split()
{
    return QPair<int, int>(-1, -1);
}

#include "transferdatasource.moc"
