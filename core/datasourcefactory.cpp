/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "datasourcefactory.h"

#include "transferdatasource.h"

#include <KDebug>

DataSourceFactory::DataSourceFactory(const KUrl &source, const KUrl &dest, const KIO::fileoffset_t &size, 
                                                                    const KIO::fileoffset_t &segSize, QObject *parent)
  : QObject(parent),
    m_source(source),
    m_dest(dest),
    m_size(size),
    m_segSize(segSize),
    m_chunks(0)
{
    if ((float) size / segSize != (int) size / segSize)
        m_chunks = new BitSet((size / segSize) + 1);
    else
        m_chunks = new BitSet(size / segSize);

    m_chunks->clear();
}

DataSourceFactory::~DataSourceFactory()
{
}

void DataSourceFactory::addDataSource(TransferDataSource *source)
{
    m_dataSources.insert(source, qMakePair(KIO::fileoffset_t(-1), KIO::fileoffset_t(-1)));
    assignSegment(source);
    connect(source, SIGNAL(broken(TransferDataSource*)), SLOT(assignSegment(source)));
    connect(source, SIGNAL(finished(TransferDataSource*)), SLOT(assignSegment(source)));
    connect(source, SIGNAL(data(const KIO::fileoffset_t&, const QByteArray&)), SLOT(writeData(const KIO::fileoffset_t&, const QByteArray&)));
}

void DataSourceFactory::removeDataSource(TransferDataSource *source)
{
    source->stop();
    m_dataSources.remove(source);
    disconnect(source, 0, 0, 0);
}

void DataSourceFactory::assignSegment(TransferDataSource *source)
{
    //TODO: Grep a _random_ chunk
    if (m_chunks->allOn())
    {
        m_dataSources.insert(source, qMakePair(KIO::fileoffset_t(-1), KIO::fileoffset_t(-1)));
        return;
    }

    int newchunk;
    for (int i = 0; i != m_chunks->getNumBits(); i++)
    {
        if (m_chunks->get(i) == 0)
        {
            newchunk = i;
            continue;
        }
    }
    KIO::fileoffset_t newoff = KIO::fileoffset_t(newchunk * m_segSize);
    m_chunks->set(newchunk, true);
    m_dataSources.insert(source, qMakePair(newoff, m_segSize));
    source->addSegment(m_source, newoff, m_segSize);
}

void DataSourceFactory::writeData(const KIO::fileoffset_t &offset, const QByteArray &data)
{
    kDebug(5001) << "Offset: " << offset << " Data: " << data;
}
