/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btdatasource.h"
#include "bittorrentsettings.h"
#include "btcache.h"
#include "btchunkselector.h"
#include "core/download.h"
#include <QStandardPaths>
#include <btversion.h>
#include <peer/authenticationmonitor.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <torrent/torrentcontrol.h>
#include <util/bitset.h>
#include <util/error.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

BTDataSource::BTDataSource(const QUrl &srcUrl, QObject *parent)
    : TransferDataSource(srcUrl, parent)
    , m_offset(0)
    , m_bytes(0)
    , m_torrentSource(QUrl())
{
    // make sure that the DataLocation directory exists (earlier this used to be handled by KStandardDirs)
    if (!QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    bt::InitLog(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/torrentlog.log")); // initialize the torrent-log

    bt::SetClientInfo("KGet", 2, 1, 0, bt::NORMAL, "KG"); // Set client info to KGet, WARNING: Pls change this for every release

    bt::Uint16 i = 0;
    do {
        qCDebug(KGET_DEBUG) << "Trying to set port to" << BittorrentSettings::port() + i;
        bt::Globals::instance().initServer(BittorrentSettings::port() + i);
        i++;
    } while (!bt::Globals::instance().getServer().isOK() && i < 10);

    if (!bt::Globals::instance().getServer().isOK())
        return;
    tc = new TorrentControl();
    csf = new BTChunkSelectorFactory();
    cf = new BTCacheFactory();
    connect(cf, SIGNAL(cacheAdded(BTCache *)), SLOT(cacheAdded(BTCache *)));
    connect(csf, SIGNAL(selectorAdded(BTChunkSelector *)), SLOT(selectorAdded(BTChunkSelector *)));
    tc->setChunkSelectorFactory(csf);
    tc->setCacheFactory(cf);
    connect(&timer, SIGNAL(timeout()), SLOT(update()));
}

BTDataSource::~BTDataSource()
{
    delete tc;
    delete cs;
    delete cf;
}

void BTDataSource::cacheAdded(BTCache *cache)
{
    connect(cache, SIGNAL(dataArrived(KIO::fileoffset_t, QByteArray)), SLOT(getData(KIO::fileoffset_t, QByteArray)));
}

void BTDataSource::selectorAdded(BTChunkSelector *selector)
{
    cs = selector;
}

void BTDataSource::start()
{
    if (m_torrentSource.isEmpty()) {
        QString tmpDirName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/tmp/");
        // make sure that the /tmp directory exists (earlier this used to be handled by KStandardDirs)
        if (!QFileInfo::exists(tmpDirName)) {
            QDir().mkpath(tmpDirName);
        }
        Download *download = new Download(m_source, tmpDirName + m_source.fileName());
        connect(download, SIGNAL(finishedSuccessfully(QUrl, QByteArray)), SLOT(init(QUrl, QByteArray)));
    } else {
        cs->excludeAll();
        const BitSet &bits = tc->availableChunksBitSet();
        bool av = true;
        Uint32 firstChunk = m_offset / tc->getStats().chunk_size;
        Uint32 lastChunk = ((m_offset + m_bytes) / tc->getStats().chunk_size) + 1; // The +1 is only a workaround for rounding up, but I dunno how to do it ;)
        for (int i = firstChunk * tc->getStats().chunk_size * 8; i <= lastChunk * tc->getStats().chunk_size * 8; i++) {
            if (!bits.get(i)) {
                emit broken();
                av = false;
                continue;
            }
        }
        if (av) {
            cs->reincluded(firstChunk, lastChunk);
            tc->start();
            timer.start(250);
        }
    }
}

void BTDataSource::stop()
{
    tc->stop(true);
    timer.stop();
}

void BTDataSource::update()
{
    bt::UpdateCurrentTime();
    bt::AuthenticationMonitor::instance().update();
    tc->update();
}

void BTDataSource::init(const QUrl &torrentSource, const QByteArray &data)
{
    Q_UNUSED(data)
    m_torrentSource = torrentSource;
    try {
        tc->init(0, m_torrentSource.url(), QString(), QString(), 0);
    } catch (bt::Error &err) {
        qCDebug(KGET_DEBUG) << err.toString();
        // m_ready = false;
    }
    start();
}

void BTDataSource::addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum)
{
    qCDebug(KGET_DEBUG);

    if (offset < m_offset) {
        m_offset = offset;
        if (m_bytes < bytes + m_offset - offset) {
            m_bytes = bytes + m_offset - offset;
        }
    }
    if (offset > m_offset && m_bytes < bytes + m_offset - offset) {
        m_bytes = bytes + m_offset - offset;
    }
    if (offset == m_offset && m_bytes < bytes) {
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

    if (m_offset + m_bytes == off + dataArray.size())
        emit finished();
}

#include "moc_btdatasource.cpp"
