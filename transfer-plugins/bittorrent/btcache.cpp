/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btcache.h"

#include <diskio/chunk.h>

#include <KDebug>
#include <QDataStream>

using namespace bt;
//TODO: Support buffered mode?
BTCache::BTCache(Torrent & tor,const QString & tmpdir,const QString & datadir)
  : Cache(tor, tmpdir, datadir),
    QObject(0)
{
}

BTCache::~BTCache()
{
}

void BTCache::load(Chunk* c)
{
    c->setData(0, Chunk::MMAPPED);
}

void BTCache::save(Chunk* c)
{
    /*if (c->getStatus() == Chunk::MMAPPED)
    {
        KIO::fileoffset_t off = c->getIndex() * tor.getChunkSize();
        kDebug(5001) << "Fileoffset is: " + QString::number(off);
        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly | QIODevice::Unbuffered);
        s << c->getData();
        emit dataArrived(off, data);
        c->clear();
        c->setStatus(Chunk::ON_DISK);
    }
    else if (c->getStatus() == Chunk::BUFFERED)
    {*/
        KIO::fileoffset_t off = c->getIndex() * tor.getChunkSize();
        kDebug(5001) << "Fileoffset is: " + QString::number(off);
        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly | QIODevice::Unbuffered);
        s << c->getData();
        emit dataArrived(off, data);
        //fd->write(c->getData(),c->getSize(),off);//Send a signal here that the signal has arrived
        c->clear();
        c->setStatus(Chunk::ON_DISK);
    //}
}

bool BTCache::prep(Chunk* c)
{
    c->setData(0, Chunk::MMAPPED);
    return true;
}

void BTCache::deleteDataFiles()
{
}

Cache* BTCacheFactory::create(Torrent & tor,const QString & tmpdir,const QString & datadir) 
{
    BTCache *newcache = new BTCache(tor, tmpdir, datadir);
    emit cacheAdded(newcache);
    return newcache;
}

#include "btcache.moc"
