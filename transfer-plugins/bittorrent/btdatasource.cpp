/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "btdatasource.h"
#include "btcache.h"
#include "btchunkselector.h"
#include "btdownload.h"
#include "bittorrentsettings.h"
#include <torrent/torrentcontrol.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <btversion.h>
#include <util/log.h>

#include <kdebug.h>
#include <kstandarddirs.h>

using namespace bt;

BTDataSource::BTDataSource()
  : TransferDataSource(0),
    m_offset(0),
    m_bytes(0),
    m_source(KUrl())
{
    bt::InitLog(KStandardDirs::locateLocal("appdata", "torrentlog.log"));//initialize the torrent-log

    bt::SetClientInfo("KGet",2,1,0,bt::NORMAL,"KG");//Set client info to KGet, WARNING: Pls change this for every release

    bt::Uint16 i = 0;
    do
    {
        kDebug(5001) << "Trying to set port to" << BittorrentSettings::port() + i;
        bt::Globals::instance().initServer(BittorrentSettings::port() + i);
        i++;
    }while (!bt::Globals::instance().getServer().isOK() && i < 10);

    if (!bt::Globals::instance().getServer().isOK())
        return;
    tc = new TorrentControl();
    csf = new BTChunkSelectorFactory();
    cf = new BTCacheFactory();
    connect(cf, SIGNAL(cacheAdded(BTCache*)), SLOT(cacheAdded(BTCache *)));
    connect(csf, SIGNAL(selectorAdded(BTChunkSelector*)), SLOT(selectorAdded(BTChunkSelector*)));
    tc->setChunkSelectorFactory(csf);
    tc->setCacheFactory(cf);
}

BTDataSource::~BTDataSource()
{
    delete tc;
    delete cs;
    delete cf;
}

void BTDataSource::cacheAdded(BTCache *cache)
{
     connect(cache, SIGNAL(dataArrived(const KIO::fileoffset_t &, const QByteArray &)), SLOT(getData(const KIO::fileoffset_t &, const QByteArray &)));
}

void BTDataSource::selectorAdded(BTChunkSelector* selector)
{
    cs = selector;
}

void BTDataSource::start()
{
    if (!m_source.isLocalFile())
    {
        BTDownload *download = new BTDownload(m_source, KStandardDirs::locateLocal("appdata", "tmp/") + m_source.fileName());
        connect(download, SIGNAL(finishedSuccessfully(KUrl)), SLOT(init(KUrl)));
    }

    cs->excludeAll();
    Uint32 firstChunk = m_offset / tc->getStats().chunk_size;
    Uint32 lastChunk = ((m_offset + m_bytes) / tc->getStats().chunk_size) + 1;//The +1 is only a workaround for rounding up, but I dunno how to do it ;)
    cs->reincluded(firstChunk, lastChunk);
    tc->start();
}

void BTDataSource::stop()
{
    tc->stop(true);
}

void BTDataSource::init(const KUrl &torrentSource)
{
    m_torrentSource = torrentSource;
    try
    {
        tc->init(0, m_torrentSource.url(), QString(), QString(), 0);
        tc->createFiles();
    }
    catch (bt::Error &err)
    {
        kDebug(5001) << err.toString();
        //m_ready = false;
    }
}

void BTDataSource::addSegment(const KUrl &srcUrl, const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes)
{
    kDebug(5001);
    if (m_source == srcUrl)
    {
        if (offset < m_offset)
        {
            m_offset = offset;
            if (m_bytes < bytes + m_offset - offset)
            {
                m_bytes = bytes + m_offset - offset;
            }
        }
        if (offset > m_offset && m_bytes < bytes + m_offset - offset)
        {
            m_bytes = bytes + m_offset - offset;
        }
        if (offset == m_offset && m_bytes < bytes)
        {
            m_bytes = bytes;
        }
    }
    else
    {
        m_source = srcUrl;
        m_offset = offset;
        m_bytes = bytes;
    }
}

void BTDataSource::getData(const KIO::fileoffset_t &off, const QByteArray &dataArray)
{
    QByteArray splittedData;
    if (off < m_offset)
        splittedData = dataArray.right(dataArray.size() - (m_offset - off));
    else if (m_offset + m_bytes < off + dataArray.size())
        splittedData = dataArray.left((off + dataArray.size()) - (m_offset + m_bytes));
    else
        splittedData = dataArray;
        
    emit data(off, splittedData);
}

#include "btdatasource.moc"
