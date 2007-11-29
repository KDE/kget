/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransfer.h"

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
  : Transfer(parent, factory, scheduler, src, dest, e) 
{
    kDebug(5001);
    if (m_source.url().isEmpty())
        return;

    bt::InitLog(KStandardDirs::locateLocal("appdata", "torrentlog.log"));//initialize the torrent-log

    bt::Uint16 i = 0;
    do
    {
        bt::Globals::instance().initServer(6881 + i);
        i++;
    }while (!bt::Globals::instance().getServer().isOK() && i < 10);

    if (!bt::Globals::instance().getServer().isOK())
        return;

    kDebug(5001) << "Source:" << m_source.url().remove("file://");
    kDebug(5001) << "Dest:" << m_dest.url().remove("file://").remove(".torrent");
    kDebug(5001) << "Temp:" << KStandardDirs::locateLocal("appdata", "tmp/");
    try
    {
        torrent = new bt::TorrentControl();
        torrent->init(0, m_source.url().remove("file://"), KStandardDirs::locateLocal("appdata", "tmp/") + m_source.fileName(), 
                                                             m_dest.url().remove("file://").remove(".torrent"), 0);
        torrent->createFiles();
        //torrent->setPreallocateDiskSpace(true);//TODO: Make this configurable
        //torrent->setMaxShareRatio(1); //TODO: Make configurable...
        stats = new bt::TorrentStats(torrent->getStats());
    }
    catch (bt::Error &err)
    {
        kDebug(5001) << err.toString();
    }

    connect(torrent, SIGNAL(stoppedByError(bt::TorrentInterface*, QString)), SLOT(slotStoppedByError(bt::TorrentInterface*, QString)));
    connect(torrent, SIGNAL(finished(bt::TorrentInterface*)), this, SLOT(downloadFinished(bt::TorrentInterface* )));
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

int BTTransfer::chunksTotal()
{
    kDebug(5001);
    return torrent->getTorrent().getNumChunks();
}

int BTTransfer::chunksDownloaded()
{
    kDebug(5001);
    //FIXME
}

int BTTransfer::dlRate()
{
    return torrent->getStats().download_rate;
}

int BTTransfer::ulRate()
{
    return stats->upload_rate;
}

int BTTransfer::totalSize()
{
    return stats->total_bytes_to_download;
}

int BTTransfer::sessionBytesDownloaded()
{
    return stats->session_bytes_downloaded;
}

int BTTransfer::sessionBytesUploaded()
{
    return stats->session_bytes_uploaded;
}

int BTTransfer::peersConnected()
{
    kDebug(5001);
    return -1;
}

int BTTransfer::peersNotConnected()
{
    kDebug(5001);
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
        setTransferChange(Tc_Status | Tc_TrackersList | Tc_Percent, true);
        kDebug(5001) << "Completely";
    }
}

void BTTransfer::stop()
{
    kDebug(5001);
    torrent->stop(true);
    m_speed = 0;
    timer.stop();
    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("process-stop"));
    m_speed = 0;
    setTransferChange(Tc_Status | Tc_Speed, true);
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

void BTTransfer::slotDownloadFinished(bt::TorrentInterface* ti)
{
    kDebug(5001);
    timer.stop();
    setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
    setTransferChange(Tc_Status, true);
}

void BTTransfer::hashingFinished()
{
    kDebug(5001);

    setTransferChange(Tc_Status, true);
    start();
}

void BTTransfer::update()
{
    kDebug(5001);

    torrent->update();
    bt::UpdateCurrentTime();
    bt::AuthenticationMonitor::instance().update();

    m_speed = dlRate();
    setTransferChange(Tc_ProcessedSize | Tc_Speed | Tc_TotalSize | Tc_Speed | Tc_TotalSize, true);
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

KUrl::List BTTransfer::trackersList()
{
    const KUrl::List trackers = torrent->getTrackersList()->getTrackerURLs();
    return trackers;
}

#include "bttransfer.moc"
