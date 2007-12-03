/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransfer.h"
#include "bittorrentsettings.h"

#include "core/kget.h"

#include <torrent/torrent.h>
#include <peer/peermanager.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/log.h>
#include <peer/authenticationmonitor.h>

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KStandardDirs>
#include <KUrl>

#include <QFile>
#include <QDomElement>
#include <QFileInfo>

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
               Scheduler* scheduler, const KUrl& src, const KUrl& dest,
               const QDomElement * e)
  : Transfer(parent, factory, scheduler, src, dest, e),
    m_dlLimit(BittorrentSettings::downloadLimit()),
    m_ulLimit(BittorrentSettings::uploadLimit())
{
    kDebug(5001);
    if (m_source.url().isEmpty())
        return;

    bt::InitLog(KStandardDirs::locateLocal("appdata", "torrentlog.log"));//initialize the torrent-log

    bt::Uint16 i = 0;
    do
    {
        kDebug(5001) << "Trying to set port to" << BittorrentSettings::port() + i;
        bt::Globals::instance().initServer(BittorrentSettings::port() + i);
        i++;
    }while (!bt::Globals::instance().getServer().isOK() && i < 10);

    if (!bt::Globals::instance().getServer().isOK())
        return;

    try
    {
        m_dlLimit = BittorrentSettings::downloadLimit();
        m_ulLimit = BittorrentSettings::uploadLimit();

        torrent = new bt::TorrentControl();
        QString tmpDir;
        if (!BittorrentSettings::tmpDir().isEmpty())
        {
            tmpDir = BittorrentSettings::tmpDir();
            kDebug(5001) << "Trying to set" << tmpDir << " as tmpDir";
            if (!QFileInfo(tmpDir).isDir())
                tmpDir = KStandardDirs::locateLocal("appdata", "tmp/");
        }
        else
            tmpDir = KStandardDirs::locateLocal("appdata", "tmp/");
            
        torrent->init(0, m_source.url().remove("file://"), tmpDir + m_source.fileName(), 
                                                             m_dest.url().remove("file://").remove(".torrent"), 0);
        torrent->setTrafficLimits(bt::Uint32(5), bt::Uint32(5));
        torrent->createFiles();

        bt::Uint32 dl;
        bt::Uint32 ul;
        torrent->getTrafficLimits(ul, dl);
        kDebug(5001) <<"Speedlimits" << ul<< dl;
      
        torrent->setPreallocateDiskSpace(BittorrentSettings::preAlloc());
        //torrent->setMaxShareRatio(1); //TODO: Make configurable...
        stats = new bt::TorrentStats(torrent->getStats());
        kDebug(5001) << "Source:" << m_source.url().remove("file://");
        kDebug(5001) << "Dest:" << m_dest.url().remove("file://").remove(".torrent");
        kDebug(5001) << "Temp:" << tmpDir;
    }
    catch (bt::Error &err)
    {
        kDebug(5001) << err.toString();
    }

    connect(torrent, SIGNAL(stoppedByError(bt::TorrentInterface*, QString)), SLOT(slotStoppedByError(bt::TorrentInterface*, QString)));
    connect(torrent, SIGNAL(finished(bt::TorrentInterface*)), this, SLOT(slotDownloadFinished(bt::TorrentInterface* )));
    //FIXME connect(tc,SIGNAL(corruptedDataFound( bt::TorrentInterface* )), this, SLOT(emitCorruptedData( bt::TorrentInterface* )));
    connect(&timer, SIGNAL(timeout()), SLOT(update()));
}

BTTransfer::~BTTransfer()
{
    if (torrent)
        delete torrent;
}

bool BTTransfer::isResumable() const
{
    kDebug(5001);
    return true;
}

void BTTransfer::start()
{
    kDebug(5001);

    if (torrent)
    {
        kDebug(5001) << "Going to download that stuff :-0";
        kDebug(5001) << "Here we are";
        torrent->start();
        kDebug(5001) << "Got started??";
        timer.start(250);
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("media-playback-start"));
        kDebug(5001) << "Jepp, it does";
        m_totalSize = totalSize();
        setTransferChange(Tc_Status | Tc_TrackersList | Tc_Percent | Tc_TotalSize, true);
        kDebug(5001) << "Completely";
        setTrafficLimits(m_ulLimit, m_dlLimit);
    }
}

void BTTransfer::stop()
{
    kDebug(5001);
    torrent->stop(true);
    m_speed = 0;
    timer.stop();
    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("process-stop"));
    setTransferChange(Tc_Status | Tc_Speed, true);
}

void BTTransfer::update()
{
    kDebug(5001);

    torrent->update();

    bt::UpdateCurrentTime();
    bt::AuthenticationMonitor::instance().update();

    m_speed = dlRate();
    m_percent = percent();

    setTransferChange(Tc_ProcessedSize | Tc_Speed | Tc_TotalSize | Tc_Speed | Tc_TotalSize | Tc_Percent, true);
}

void BTTransfer::save(QDomElement e) // krazy:exclude=passbyvalue
{
}

void BTTransfer::load(const QDomElement &e)
{
}

void BTTransfer::slotStoppedByError(bt::TorrentInterface* error, QString errormsg)
{
    kDebug(5001) << errormsg;
}

void BTTransfer::setPort(int port)
{
    bt::Globals::instance().getServer().changePort(port);
}

void BTTransfer::slotDownloadFinished(bt::TorrentInterface* ti)
{
    kDebug(5001);
    timer.stop();
    setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
    setTransferChange(Tc_Status, true);
}

void BTTransfer::setTrafficLimits(int ulLimit, int dlLimit)
{
    torrent->setTrafficLimits(ulLimit * 1000, dlLimit * 1000);
    m_dlLimit = dlLimit;
    m_ulLimit = ulLimit;
}

/**Property-Functions**/
KUrl::List BTTransfer::trackersList() const
{
    const KUrl::List trackers = torrent->getTrackersList()->getTrackerURLs();
    return trackers;
}

int BTTransfer::dlRate() const
{
    return torrent->getStats().download_rate;
}

int BTTransfer::ulRate() const
{
    return torrent->getStats().upload_rate;
}

int BTTransfer::totalSize() const
{
    return torrent->getStats().total_bytes_to_download;
}

int BTTransfer::sessionBytesDownloaded() const
{
    return torrent->getStats().session_bytes_downloaded;
}

int BTTransfer::sessionBytesUploaded() const
{
    return torrent->getStats().session_bytes_uploaded;
}

int BTTransfer::chunksTotal() const
{
    return torrent->getTorrent().getNumChunks();
}

int BTTransfer::chunksDownloaded() const
{
    return torrent->downloadedChunksBitSet().numOnBits();
}

int BTTransfer::chunksExcluded() const
{
    return torrent->excludedChunksBitSet().numOnBits();
}

int BTTransfer::chunksLeft() const
{
    return chunksTotal() - chunksDownloaded();
}

int BTTransfer::seedsConnected() const
{
    return torrent->getStats().seeders_connected_to;
}

int BTTransfer::seedsDisconnected() const
{
    return torrent->getStats().seeders_total;
}

int BTTransfer::leechesConnected() const
{
    return torrent->getStats().leechers_connected_to;
}

int BTTransfer::leechesDisconnected() const
{
    return torrent->getStats().leechers_total;
}

int BTTransfer::elapsedTime() const
{
    kDebug(5001);
    return torrent->getRunningTimeDL();
}

int BTTransfer::remainingTime() const
{
    kDebug(5001);
    return torrent->getETA();
}

int BTTransfer::ulLimit() const
{
}

int BTTransfer::dlLimit() const
{
}

bt::TorrentControl * BTTransfer::torrentControl()
{
    return torrent;
}

int BTTransfer::percent() const
{
    return ((float) chunksDownloaded() / (float) chunksTotal()) * 100;
}

#include "bttransfer.moc"
