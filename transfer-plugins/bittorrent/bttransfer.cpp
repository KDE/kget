/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2007 Joris Guisson   <joris.guisson@gmail.com>

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
#include <btdownload.h>
#include <btversion.h>

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KStandardDirs>
#include <KUrl>
#include <KMessageBox>

#include <QFile>
#include <QDomElement>
#include <QFileInfo>

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
               Scheduler* scheduler, const KUrl& src, const KUrl& dest,
               const QDomElement * e)
  : Transfer(parent, factory, scheduler, src, dest, e),
    torrent(0),
    m_tmp(0),
    m_dlLimit(BittorrentSettings::downloadLimit()),
    m_ulLimit(BittorrentSettings::uploadLimit()),
    m_ratio(BittorrentSettings::maxShareRatio()),
    m_ready(false),
    m_downloadFinished(false)
{
    kDebug(5001);
    if (m_source.url().isEmpty())
        return;

    if (e)
        load(*e);
}

BTTransfer::~BTTransfer()
{
    kDebug(5001);
    if (torrent)
        delete torrent;
}

/**Reimplemented functions from Transfer-Class (transfer.cpp)**/
bool BTTransfer::isResumable() const
{
    kDebug(5001);
    return true;
}

void BTTransfer::start()
{
    kDebug(5001);
    if (!torrent)
    {
        if (!m_source.isLocalFile())
        {
            kDebug(5001) << m_dest.path();
             BTDownload *download = new BTDownload(m_source);

            setStatus(Job::Stopped, i18n("Downloading Torrent-File.."), SmallIcon("document-save"));
            setTransferChange(Tc_Status, true);

            m_source = KStandardDirs::locateLocal("appdata", "tmp/") + m_source.fileName();
            connect(download, SIGNAL(finishedSuccessfully(KUrl)), SLOT(init(KUrl)));
        }
        else
            init();
    }
    else
        startTorrent();
}

void BTTransfer::stop()
{
    kDebug(5001);
    if (m_ready)
    {
        stopTorrent();
    }
}

void BTTransfer::update()
{
    kDebug(5001);

    if (torrent)
    {
        QStringList files;
        if (torrent->hasMissingFiles(files))
        {
            kDebug(5001) << "Recreate Missing Files";
            torrent->recreateMissingFiles();
        }
        updateTorrent();
    }
    else
        timer.stop();
}

void BTTransfer::load(const QDomElement &e)
{
    kDebug(5001);
    m_source = KUrl(e.attribute("Source"));
    m_dest = KUrl(e.attribute("Dest"));

    m_totalSize = e.attribute("TotalSize").toULongLong();
    m_processedSize = e.attribute("ProcessedSize").toULongLong();

    if( m_totalSize != 0)
        m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
    else
        m_percent = 0;

    m_ulLimit = e.attribute("Upload-Limit").toInt();
    m_dlLimit = e.attribute("Download-Limit").toInt();

    if((m_totalSize == m_processedSize) && (m_totalSize != 0))
    {
        setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
        // "ok" icon should probably be "dialog-success", but we don't have that icon in KDE 4.0
    }
    else
    {
        setStatus(status(), i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    }
}

void BTTransfer::save(const QDomElement &element)
{
    kDebug(5001);

    QDomElement e = element;

    Transfer::save(e);

    e.setAttribute("Upload-Limit", ulLimit());
    e.setAttribute("Download-Limit", dlLimit());
}

/**Public functions of BTTransfer**/

void BTTransfer::setPort(int port)
{
    kDebug(5001);
    bt::Globals::instance().getServer().changePort(port);
}

void BTTransfer::setTrafficLimits(int ulLimit, int dlLimit)
{
    kDebug(5001);
    if (!torrent)
        return;

    torrent->setTrafficLimits(ulLimit * 1000, dlLimit * 1000);
    m_dlLimit = dlLimit;
    m_ulLimit = ulLimit;
}

void BTTransfer::setMaxShareRatio(float ratio)
{
    if (m_ratio != ratio)
        m_ratio = ratio;

    if (m_ratio != 0)
        torrent->setMaxShareRatio(m_ratio);
}

void BTTransfer::addTracker(const QString &url)
{
    kDebug(5001);
    if(torrent->getStats().priv_torrent)
    {
	KMessageBox::sorry(0, i18n("Cannot add a tracker to a private torrent."));
	return;
    }

    if(!KUrl(url).isValid())
    {
	KMessageBox::error(0, i18n("Malformed URL."));
	return;
    }

    torrent->getTrackersList()->addTracker(url,true);
}

/**Private functions**/

void BTTransfer::startTorrent()
{
    if (m_ready)
    {
        kDebug(5001) << "Going to download that stuff :-0";
        setTrafficLimits(m_ulLimit, m_dlLimit);//Set traffic-limits before starting
        kDebug(5001) << "Here we are";
        torrent->start();
        kDebug(5001) << "Got started??";
        timer.start(250);
        setStatus(Job::Running, i18nc("transfer state: downloading", "Downloading.."), SmallIcon("media-playback-start"));
        kDebug(5001) << "Jepp, it does";
        m_totalSize = totalSize();
        setTransferChange(Tc_Status | Tc_TrackersList | Tc_TotalSize, true);
        kDebug(5001) << "Completely";
    }
}

void BTTransfer::stopTorrent()
{
    torrent->stop(true);
    m_speed = 0;
    timer.stop();

    if (m_downloadFinished)
    {
        setStatus(Job::Stopped, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
    }
    else
        setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    setTransferChange(Tc_Status, true);
}

void BTTransfer::updateTorrent()
{
    kDebug(5001) << "Update torrent";
    bt::UpdateCurrentTime();
    bt::AuthenticationMonitor::instance().update();
    kDebug(5001) << "Ignore this ;)";
    torrent->update();
    kDebug(5001) << "Done";
    m_speed = dlRate();
    m_percent = percent();
    m_processedSize = processedSize();

    setTransferChange(Tc_ProcessedSize | Tc_Speed | Tc_Percent, true);
}

void BTTransfer::init(const KUrl &src)
{
    kDebug(5001);
    if (src != m_source && !src.isEmpty())
        m_source = src;

    setStatus(Job::Running, i18n("Analizing torrent.."), SmallIcon("document-preview")); // jpetso says: you should probably use the "process-working" icon here (from the animations category), but that's a multi-frame PNG so it's hard for me to test
    setTransferChange(Tc_Status, true);

    bt::InitLog(KStandardDirs::locateLocal("appdata", "torrentlog.log"));//initialize the torrent-log

    bt::SetClientInfo("KGet",2,0,0,bt::NORMAL,"KG");//Set client info to KGet, WARNING: Pls change this for every release

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
        torrent = new bt::TorrentControl();

        if (!BittorrentSettings::tmpDir().isEmpty())
        {
            m_tmp = BittorrentSettings::tmpDir();
            kDebug(5001) << "Trying to set" << m_tmp << " as tmpDir";
            if (!QFileInfo(m_tmp).isDir())
                m_tmp = KStandardDirs::locateLocal("appdata", "tmp/");
        }
        else
            m_tmp = KStandardDirs::locateLocal("appdata", "tmp/");

        m_ready = true;

        torrent->init(0, m_source.url().remove("file://"), m_tmp + m_source.fileName().remove(".torrent"),
                                                             m_dest.directory().remove("file://"), 0);

        if (torrent->getStats().multi_file_torrent)
            m_dest = torrent->getStats().output_path;
        else
            m_dest = torrent->getDataDir() + torrent->getStats().torrent_name;

        torrent->createFiles();
      
        torrent->setPreallocateDiskSpace(BittorrentSettings::preAlloc());

        setMaxShareRatio(m_ratio);
        setTrafficLimits(m_ulLimit, m_dlLimit);

        connect(torrent, SIGNAL(stoppedByError(bt::TorrentInterface*, QString)), SLOT(slotStoppedByError(bt::TorrentInterface*, QString)));
        connect(torrent, SIGNAL(finished(bt::TorrentInterface*)), this, SLOT(slotDownloadFinished(bt::TorrentInterface* )));
        //FIXME connect(tc,SIGNAL(corruptedDataFound( bt::TorrentInterface* )), this, SLOT(emitCorruptedData( bt::TorrentInterface* )));
    }
    catch (bt::Error &err)
    {
        kDebug(5001) << err.toString();
        //m_ready = false;
    }
    startTorrent();
    connect(&timer, SIGNAL(timeout()), SLOT(update()));
}

void BTTransfer::slotStoppedByError(const bt::TorrentInterface* &error, const QString &errormsg)
{
    Q_UNUSED(error);
    kDebug(5001) << errormsg;
}

void BTTransfer::slotDownloadFinished(bt::TorrentInterface* ti)
{
    Q_UNUSED(ti);
    kDebug(5001);
    m_downloadFinished = true;
    timer.stop();
    setStatus(Job::Running, i18n("Seeding"), SmallIcon("media-playback-start"));
    setTransferChange(Tc_Status, true);
}

/**Property-Functions**/
KUrl::List BTTransfer::trackersList() const
{
    if (!torrent)
        return KUrl::List();

    const KUrl::List trackers = torrent->getTrackersList()->getTrackerURLs();
    return trackers;
}

int BTTransfer::dlRate() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().download_rate;
}

int BTTransfer::ulRate() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().upload_rate;
}

float BTTransfer::totalSize() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().total_bytes_to_download;
}

float BTTransfer::processedSize() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().bytes_downloaded;
}

int BTTransfer::sessionBytesDownloaded() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().session_bytes_downloaded;
}

int BTTransfer::sessionBytesUploaded() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().session_bytes_uploaded;
}

int BTTransfer::chunksTotal() const
{
    if (!torrent)
        return -1;

    return torrent->getTorrent().getNumChunks();
}

int BTTransfer::chunksDownloaded() const
{
    if (!torrent)
        return -1;

    return torrent->downloadedChunksBitSet().numOnBits();
}

int BTTransfer::chunksExcluded() const
{
    if (!torrent)
        return -1;

    return torrent->excludedChunksBitSet().numOnBits();
}

int BTTransfer::chunksLeft() const
{
    if (!torrent)
        return -1;

    return chunksTotal() - chunksDownloaded();
}

int BTTransfer::seedsConnected() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().seeders_connected_to;
}

int BTTransfer::seedsDisconnected() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().seeders_total;
}

int BTTransfer::leechesConnected() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().leechers_connected_to;
}

int BTTransfer::leechesDisconnected() const
{
    if (!torrent)
        return -1;

    return torrent->getStats().leechers_total;
}

int BTTransfer::elapsedTime() const
{
    if (!torrent)
        return -1;

    return torrent->getRunningTimeDL();
}

int BTTransfer::remainingTime() const
{
    if (!torrent)
        return -1;

    return torrent->getETA();
}

int BTTransfer::ulLimit() const
{
    if (!torrent)
        return -1;
    else
        return m_ulLimit;
}

int BTTransfer::dlLimit() const
{
    if (!torrent)
        return -1;
    else
        return m_dlLimit;
}

bt::TorrentControl * BTTransfer::torrentControl()
{
    return torrent;
}

int BTTransfer::percent() const
{
    if (!torrent)
        return -1;

    return ((float) chunksDownloaded() / (float) chunksTotal()) * 100;
}

bool BTTransfer::ready()
{
    return m_ready;
}

float BTTransfer::maxShareRatio() const
{
    return m_ratio;
}

#include "bttransfer.moc"
