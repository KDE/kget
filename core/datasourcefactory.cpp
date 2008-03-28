/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "datasourcefactory.h"

#include "transferdatasource.h"

#include <kio/filejob.h>

#include <KDebug>

DataSourceFactory::DataSourceFactory(const KUrl &dest, const KIO::fileoffset_t &size, 
                                                                    const KIO::fileoffset_t &segSize, QObject *parent)
  : QObject(parent),
    m_dest(dest),
    m_size(size),
    m_segSize(segSize),
    m_chunks(0),
    m_putJob(0)
{
    kDebug(5001) << "Initialize DataSourceFactory: Dest: " + m_dest.url() + "Size: " + QString::number(size) + "SegSize: " + QString::number(segSize);
    if ((float) size / segSize != (int) size / segSize)
        m_chunks = new BitSet((size / segSize) + 1);
    else
        m_chunks = new BitSet(size / segSize);

    m_chunks->clear();
}

DataSourceFactory::~DataSourceFactory()
{
}

void DataSourceFactory::start()
{
    kDebug(5001) << "Start DataSourceFactory";
    m_putJob = KIO::open(m_dest, QIODevice::WriteOnly | QIODevice::ReadOnly);
    foreach (TransferDataSource *source, m_dataSources.keys())
        source->start();
}

void DataSourceFactory::stop()
{
    foreach (TransferDataSource *source, m_dataSources.keys())
        source->stop();

    if (m_putJob)
        m_putJob->close();
}

void DataSourceFactory::addDataSource(TransferDataSource *source, const KUrl &url)
{
    m_dataSources.insert(source, url);
    assignSegment(source);
    connect(source, SIGNAL(broken()), SLOT(assignSegment()));
    connect(source, SIGNAL(finished()), SLOT(assignSegment()));
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
    if (!source)
    {
        source = qobject_cast<TransferDataSource*>(QObject::sender());
        if (!source)
            return;
    }
    //TODO: Grep a _random_ chunk
    if (m_chunks->allOn())
        return;

    int newchunk = 0;
    for (uint i = 0; i != m_chunks->getNumBits(); i++)
    {
        if (m_chunks->get(i) == 0)
        {
            newchunk = i;
            continue;
        }
    }
    KIO::fileoffset_t newoff = KIO::fileoffset_t(newchunk * m_segSize);
    m_chunks->set(newchunk, true);
    source->addSegment(m_dataSources.value(source), newoff, m_segSize);
}

void DataSourceFactory::writeData(const KIO::fileoffset_t &offset, const QByteArray &data)
{
    kDebug(5001) << "Offset: " << offset << " Data: " << data;
    m_putJob->seek(offset);
    m_putJob->write(data);
}
