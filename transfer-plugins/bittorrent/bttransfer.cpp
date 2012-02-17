/* This file is part of the KDE project

   Copyright (C) 2007-2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2007 Joris Guisson   <joris.guisson@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransfer.h"
#include "bittorrentsettings.h"
#include "bttransferhandler.h"
//#include "btchunkselector.h"
#include "advanceddetails/monitor.h"
#include "core/kget.h"
#include "core/filemodel.h"
#include "core/download.h"

#include <torrent/torrent.h>
#include <peer/peermanager.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/log.h>
#include <peer/authenticationmonitor.h>
#include <interfaces/trackerinterface.h>
#include <utp/utpserver.h>
#include <version.h>

#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KIO/CopyJob>
#include <KStandardDirs>
#include <KUrl>
#include <KMessageBox>

#include <QFile>
#include <QDomElement>
#include <QFileInfo>
#include <QDir>

#ifdef ERROR
#undef ERROR
#endif

BTTransfer::BTTransfer(TransferGroup* parent, TransferFactory* factory,
               Scheduler* scheduler, const KUrl& src, const KUrl& dest,
               const QDomElement * e)
  : Transfer(parent, factory, scheduler, src, dest, e),
    torrent(0),
    m_tmp(KStandardDirs::locateLocal("appdata", "tmp/")),
    m_ready(false),
    m_downloadFinished(false),
    m_movingFile(false),
    m_fileModel(0),
    m_updateCounter(0)
{
    m_directory = m_dest.upUrl();//FIXME test

    setCapabilities(Transfer::Cap_Moving | Transfer::Cap_Renaming | Transfer::Cap_Resuming | Transfer::Cap_SpeedLimit);
}

BTTransfer::~BTTransfer()
{
    if (torrent && m_ready)
        torrent->setMonitor(0);

    delete torrent;
}

void BTTransfer::deinit(Transfer::DeleteOptions options)
{
    kDebug() << "****************************DEINIT";
    if (torrent && (options & Transfer::DeleteFiles)) {//FIXME: Also delete when torrent does not exist
        torrent->deleteDataFiles();
    }
    if (options & Transfer::DeleteTemporaryFiles) {
        QDir tmpDir(m_tmp);
        kDebug(5001) << m_tmp + m_source.fileName().remove(".torrent");
        tmpDir.rmdir(m_source.fileName().remove(".torrent") + "/dnd");
        tmpDir.cd(m_source.fileName().remove(".torrent"));
        QStringList list = tmpDir.entryList();
        foreach (const QString &file, list) {
            tmpDir.remove(file);
        }
        tmpDir.cdUp();
        tmpDir.rmdir(m_source.fileName().remove(".torrent"));

        //only remove the .torrent file if it was downloaded by KGet
        if (!m_tmpTorrentFile.isEmpty()) {
            kDebug(5001) << "Removing" << m_tmpTorrentFile;
            QFile torrentFile(m_tmpTorrentFile);
            torrentFile.remove();
        }
    }
}

/** Reimplemented functions from Transfer-Class **/
bool BTTransfer::isStalled() const
{
    return (status() == Job::Running) && (downloadSpeed() == 0) && torrent && torrent->getStats().status == bt::STALLED;
}

bool BTTransfer::isWorking() const
{
    if (!torrent)
        return false;
    const bt::TorrentStats stats = torrent->getStats();
    return (stats.status != bt::ERROR) && (stats.status != bt::STALLED) && (stats.status != bt::NO_SPACE_LEFT) && (stats.status != bt::INVALID_STATUS);
}

void BTTransfer::start()
{
    if (m_movingFile)
    {
        return;
    }

    if (!torrent)
    {
        if (!m_source.isLocalFile())
        {
            kDebug(5001) << m_dest.path();
            m_tmpTorrentFile = QString(KStandardDirs::locateLocal("appdata", "tmp/") + m_dest.fileName());
            Download *download = new Download(m_source, m_tmpTorrentFile);

            setStatus(Job::Stopped, i18n("Downloading Torrent File...."), SmallIcon("document-save"));
            setTransferChange(Tc_Status, true);

            //m_source = KStandardDirs::locateLocal("appdata", "tmp/") + m_source.fileName();
            connect(download, SIGNAL(finishedSuccessfully(KUrl,QByteArray)), SLOT(btTransferInit(KUrl,QByteArray)));
        }
        else
            btTransferInit();
    }
    else
        startTorrent();
}

bool BTTransfer::setDirectory(const KUrl &newDirectory)
{
    //check if the newDestination is the same as the old
    KUrl temp = newDirectory;
    temp.addPath(torrent->getStats().torrent_name);
    if (newDirectory.isValid() && (newDirectory != dest()) && (temp != dest()))
    {
        if (torrent->changeOutputDir(newDirectory.pathOrUrl(), bt::TorrentInterface::MOVE_FILES))
        {
            connect(torrent, SIGNAL(aboutToBeStarted(bt::TorrentInterface*,bool&)), this, SLOT(newDestResult()));
            m_movingFile = true;
            m_directory = newDirectory;
            m_dest = m_directory;
            m_dest.addPath(torrent->getStats().torrent_name);

            setStatus(Job::Stopped, i18nc("changing the destination of the file", "Changing destination"), SmallIcon("media-playback-pause"));
            setTransferChange(Tc_Status, true);
            return true;
        }
    }
    m_movingFile = false;
    return false;
}

void BTTransfer::newDestResult()
{
    disconnect(torrent, SIGNAL(aboutToBeStarted(bt::TorrentInterface*,bool&)), this, SLOT(newDestResult()));
    m_movingFile = false;

    setStatus(Job::Running, i18nc("transfer state: downloading", "Downloading...."), SmallIcon("media-playback-start"));
    setTransferChange(Tc_FileName | Tc_Status, true);
}

void BTTransfer::stop()
{
    if (m_movingFile)
        return;

    if (m_ready)
    {
        stopTorrent();
    }
}

/**Own public functions**/
void BTTransfer::update()
{
    if (m_movingFile)
    {
        return;
    }

    if (torrent)
    {
        QStringList files;
        if (torrent->hasMissingFiles(files))
        {
            torrent->recreateMissingFiles();
        }
        updateTorrent();
    }
    else
        timer.stop();
}

void BTTransfer::load(const QDomElement *element)
{
    Transfer::load(element);

    if ((m_totalSize == m_downloadedSize) && (m_totalSize != 0))
    {
        setStatus(Job::Stopped, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
    }
}

// void BTTransfer::save(const QDomElement &element)
// {
//     kDebug(5001);
// 
//     QDomElement e = element;
// 
//     Transfer::save(e);
// }

/**Public functions of BTTransfer**/

void BTTransfer::setPort(int port)
{
    bt::Globals::instance().getTCPServer().changePort(port);
    if (BittorrentSettings::enableUTP())
        bt::Globals::instance().getUTPServer().changePort(port + 1);
}

void BTTransfer::setSpeedLimits(int ulLimit, int dlLimit)
{
    kDebug(5001);
    if (!torrent)
        return;

    torrent->setTrafficLimits(ulLimit * 1000, dlLimit * 1000);
}

void BTTransfer::addTracker(const QString &url)
{
    kDebug(5001);
    if(torrent->getStats().priv_torrent) {
        KMessageBox::sorry(0, i18n("Cannot add a tracker to a private torrent."));
        return;
    }

    if(!KUrl(url).isValid()) {
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
        //kDebug(5001) << "Going to download that stuff :-0";
        setSpeedLimits(uploadLimit(Transfer::InvisibleSpeedLimit), downloadLimit(Transfer::InvisibleSpeedLimit));//Set traffic-limits before starting
        torrent->setMonitor(this);
        torrent->start();
        timer.start(250);
        if (chunksTotal() == chunksDownloaded()/* && !m_downloadFinished*/) {
            slotDownloadFinished(torrent);
        } else {
            setStatus(Job::Running, i18nc("transfer state: downloading", "Downloading...."), SmallIcon("media-playback-start"));
        }
        m_totalSize = torrent->getStats().total_bytes_to_download;
        setTransferChange(Tc_Status | Tc_TrackersList | Tc_TotalSize, true);
        updateFilesStatus();
    }
}

void BTTransfer::stopTorrent()
{
    torrent->stop();
    torrent->setMonitor(0);
    m_downloadSpeed = 0;
    timer.stop();

    if (m_downloadFinished)
    {
        setStatus(Job::Stopped, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
    }
    else
    {
        setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    }
    setTransferChange(Tc_Status, true);

    updateFilesStatus();
}

void BTTransfer::updateTorrent()
{
    //kDebug(5001) << "Update torrent";
    bt::UpdateCurrentTime();
    bt::AuthenticationMonitor::instance().update();
    torrent->update();

    ChangesFlags changesFlags = 0;

    if (m_downloadedSize != (m_downloadedSize = torrent->getStats().bytes_downloaded))
        changesFlags |= Tc_DownloadedSize;

    if (m_uploadSpeed != static_cast<int>(torrent->getStats().upload_rate))
    {
        m_uploadSpeed = torrent->getStats().upload_rate;
        changesFlags |= Tc_UploadSpeed;
    }

    if (m_downloadSpeed != static_cast<int>(torrent->getStats().download_rate))
    {
        m_downloadSpeed = torrent->getStats().download_rate;
        changesFlags |= Tc_DownloadSpeed;
    }

    int percent = (chunksDownloaded() * 100) / chunksTotal();
    if (m_percent != percent) {
        m_percent = percent;
        changesFlags |= Tc_Percent;
    }

    setTransferChange(changesFlags, true);

    //update the files status every 3 seconds
    if (!m_updateCounter)
    {
        updateFilesStatus();
        m_updateCounter = 12;
    }
    --m_updateCounter;
}

void BTTransfer::updateFilesStatus()
{
    const Job::Status currentStatus = this->status();
    if (!torrent)
    {
        return;
    }
    const bt::TorrentStats *stats = &torrent->getStats();
    if (stats->multi_file_torrent)
    {
        QHash<KUrl, bt::TorrentFileInterface*>::const_iterator it;
        QHash<KUrl, bt::TorrentFileInterface*>::const_iterator itEnd = m_files.constEnd();
        for (it = m_files.constBegin(); it != itEnd; ++it)
        {
            QModelIndex status = m_fileModel->index(it.key(), FileItem::Status);
            if (!(*it)->doNotDownload() && (currentStatus == Job::Running))
            {
                m_fileModel->setData(status, Job::Running);
            }
            else
            {
                m_fileModel->setData(status, Job::Stopped);
            }
            if (qFuzzyCompare((*it)->getDownloadPercentage(), 100.0f))
            {
                m_fileModel->setData(status, Job::Finished);
            }
        }
    }
    else
    {
        QModelIndexList indexes = fileModel()->fileIndexes(FileItem::Status);
        if (indexes.count() != 1)
        {
            return;
        }

        QModelIndex index = indexes.first();
        if (stats->bytes_left_to_download)
        {
            if (currentStatus == Job::Running)
            {
                fileModel()->setData(index, Job::Running);
            }
            else
            {
                fileModel()->setData(index, Job::Stopped);
            }
        }
        else
        {
            fileModel()->setData(index, Job::Finished);
        }
    }
}

void BTTransfer::btTransferInit(const KUrl &src, const QByteArray &data)
{
    Q_UNUSED(data)
    kDebug(5001);
    if (src != m_source && !src.isEmpty())
        m_source = src;

    QFile file(m_source.toLocalFile());

    if (!file.exists()) {
        setError(i18n("Torrent file does not exist"), SmallIcon("dialog-cancel"), Job::NotSolveable);
        setTransferChange(Tc_Status, true);
        return;
    }

    setStatus(Job::Stopped, i18n("Analyzing torrent...."), SmallIcon("document-preview")); // jpetso says: you should probably use the "process-working" icon here (from the animations category), but that's a multi-frame PNG so it's hard for me to test
    setTransferChange(Tc_Status, true);

    bt::InitLog(KStandardDirs::locateLocal("appdata", "torrentlog.log"), false, false);//initialize the torrent-log

    bt::SetClientInfo("KGet", 2, KDE_VERSION_MINOR, KDE_VERSION_RELEASE, bt::NORMAL, "KG");//Set client info to KGet

    bt::Uint16 i = 0;
    while (!bt::Globals::instance().initTCPServer(BittorrentSettings::port() + i) && i < 10)
        i++;
    
    if (i == 10) {
        setError(i18n("Cannot initialize port..."), SmallIcon("dialog-cancel"));
        setTransferChange(Tc_Status);
        return;
    }
    if (BittorrentSettings::enableUTP()) {
        while (!bt::Globals::instance().initUTPServer(BittorrentSettings::port() + i) && i < 10) //We don't care if it fails for now as UTP is experimental...
            i++;
    }

    QDir tmpDir(m_tmp + m_source.fileName().remove(".torrent"));
    if (tmpDir.exists())
    {
        tmpDir.remove("torrent");
    }
    try
    {
        torrent = new bt::TorrentControl();

        if (!BittorrentSettings::tmpDir().isEmpty() && QFileInfo(BittorrentSettings::tmpDir()).isDir())
        {
            m_tmp = BittorrentSettings::tmpDir();
        }

        m_ready = true;

        kDebug() << "Source:" << m_source.path() << "Destination:" << m_dest.path();
        torrent->init(0, m_source.toLocalFile(), m_tmp + m_source.fileName().remove(".torrent"), KUrl(m_dest.directory()).toLocalFile());

        m_dest = torrent->getStats().output_path;
        if (!torrent->getStats().multi_file_torrent && (m_dest.fileName() != torrent->getStats().torrent_name))//TODO check if this is needed, so if that case is true at some point
        {
            m_dest.addPath(torrent->getStats().torrent_name);
        }

        torrent->createFiles();

        torrent->setPreallocateDiskSpace(BittorrentSettings::preAlloc());

        connect(torrent, SIGNAL(stoppedByError(bt::TorrentInterface*,QString)), SLOT(slotStoppedByError(bt::TorrentInterface*,QString)));
        connect(torrent, SIGNAL(finished(bt::TorrentInterface*)), this, SLOT(slotDownloadFinished(bt::TorrentInterface*)));
        //FIXME connect(tc,SIGNAL(corruptedDataFound(bt::TorrentInterface*)), this, SLOT(emitCorruptedData(bt::TorrentInterface*)));//TODO: Fix it
    }
    catch (bt::Error &err)
    {
        m_ready = false;
        torrent->deleteLater();
        torrent = 0;
        setError(err.toString(), SmallIcon("dialog-cancel"), Job::NotSolveable);
        setTransferChange(Tc_Status);
        return;
    }
    startTorrent();
    connect(&timer, SIGNAL(timeout()), SLOT(update()));
}

void BTTransfer::slotStoppedByError(const bt::TorrentInterface* &error, const QString &errormsg)
{
    Q_UNUSED(error)
    stop();
    setError(errormsg, SmallIcon("dialog-cancel"), Job::NotSolveable);
    setTransferChange(Tc_Status);
}

void BTTransfer::slotDownloadFinished(bt::TorrentInterface* ti)
{
    kDebug(5001) << "Start seeding *********************************************************************";
    Q_UNUSED(ti)
    m_downloadFinished = true;
    //timer.stop();
    setStatus(Job::FinishedKeepAlive, i18nc("Transfer status: seeding", "Seeding...."), SmallIcon("media-playback-start"));
    setTransferChange(Tc_Status, true);
}

/**Property-Functions**/
KUrl::List BTTransfer::trackersList() const
{
    if (!torrent)
        return KUrl::List();

    KUrl::List trackers;
    foreach (bt::TrackerInterface * tracker, torrent->getTrackersList()->getTrackers())
        trackers << tracker->trackerURL();
    return trackers;
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
        return Transfer::remainingTime();

    return torrent->getETA();
}

bt::TorrentControl * BTTransfer::torrentControl()
{
    return torrent;
}

bool BTTransfer::ready()
{
    return m_ready;
}

void BTTransfer::downloadRemoved(bt::ChunkDownloadInterface* cd)
{
    if (static_cast<BTTransferHandler*>(handler())->torrentMonitor())
        static_cast<BTTransferHandler*>(handler())->torrentMonitor()->downloadRemoved(cd);

    setTransferChange(Tc_ChunksDownloaded | Tc_ChunksExcluded | Tc_ChunksLeft, true);
}

void BTTransfer::downloadStarted(bt::ChunkDownloadInterface* cd)
{
    if (static_cast<BTTransferHandler*>(handler())->torrentMonitor())
        static_cast<BTTransferHandler*>(handler())->torrentMonitor()->downloadStarted(cd);

    setTransferChange(Tc_ChunksDownloaded | Tc_ChunksExcluded | Tc_ChunksLeft, true);
}

void BTTransfer::peerAdded(bt::PeerInterface* peer)
{
    if (static_cast<BTTransferHandler*>(handler())->torrentMonitor())
        static_cast<BTTransferHandler*>(handler())->torrentMonitor()->peerAdded(peer);

    setTransferChange(Tc_SeedsConnected | Tc_SeedsDisconnected | Tc_LeechesConnected | Tc_LeechesDisconnected, true);
}

void BTTransfer::peerRemoved(bt::PeerInterface* peer)
{
    if (static_cast<BTTransferHandler*>(handler())->torrentMonitor())
        static_cast<BTTransferHandler*>(handler())->torrentMonitor()->peerRemoved(peer);

    setTransferChange(Tc_SeedsConnected | Tc_SeedsDisconnected | Tc_LeechesConnected | Tc_LeechesDisconnected, true);
}

void BTTransfer::stopped()
{
    if (static_cast<BTTransferHandler*>(handler())->torrentMonitor())
        static_cast<BTTransferHandler*>(handler())->torrentMonitor()->stopped();
}

void BTTransfer::destroyed()
{
    if (static_cast<BTTransferHandler*>(handler())->torrentMonitor())
        static_cast<BTTransferHandler*>(handler())->torrentMonitor()->destroyed();
}

QList<KUrl> BTTransfer::files() const
{
    QList<KUrl> urls;

    if (!torrent)
    {
        return urls;
    }

    //multiple files
    if (torrent->getStats().multi_file_torrent)
    {
        for (uint i = 0; i < torrent->getNumFiles(); ++i)
        {
            const QString path = torrent->getTorrentFile(i).getPathOnDisk();
            urls.append(KUrl(path));
        }
    }
    //one single file
    else
    {
        KUrl temp = m_dest;
        if (m_dest.fileName() != torrent->getStats().torrent_name)//TODO check if the body is ever entered!
        {
            temp.addPath(torrent->getStats().torrent_name);
        }
        urls.append(temp);
    }

    return urls;
}

void BTTransfer::filesSelected()
{
    QModelIndexList indexes = fileModel()->fileIndexes(FileItem::File);
    //one single file
    if (indexes.count() == 1)
    {
        QModelIndex index = indexes.first();
        const bool doDownload = index.data(Qt::CheckStateRole).toBool();
        if (torrent && torrent->getStats().bytes_left_to_download)
        {
            if (doDownload)
            {
                start();
            }
            else
            {
                stop();
            }
        }
    }
    //multiple files
    else
    {
        foreach (const QModelIndex &index, indexes)
        {
            const KUrl dest = fileModel()->getUrl(index);
            const bool doDownload = index.data(Qt::CheckStateRole).toBool();
            bt::TorrentFileInterface *file = m_files[dest];
            file->setDoNotDownload(!doDownload);
        }
    }

//     setTransferChange(Tc_TotalSize | Tc_DownloadedSize | Tc_Percent, true);
}

FileModel *BTTransfer::fileModel()//TODO correct file model for one-file-torrents
{
    if (!m_fileModel)
    {
        if (!torrent)
        {
            return 0;
        }

        //multiple files
        if (torrent->getStats().multi_file_torrent)
        {
            for (bt::Uint32 i = 0; i < torrent->getNumFiles(); ++i)
            {
                bt::TorrentFileInterface *file = &torrent->getTorrentFile(i);
                m_files[KUrl(file->getPathOnDisk())] = file;
            }
            m_fileModel = new FileModel(m_files.keys(), directory(), this);
    //         connect(m_fileModel, SIGNAL(rename(KUrl,KUrl)), this, SLOT(slotRename(KUrl,KUrl)));
            connect(m_fileModel, SIGNAL(checkStateChanged()), this, SLOT(filesSelected()));

            //set the checkstate, the status and the size of the model items
            QHash<KUrl, bt::TorrentFileInterface*>::const_iterator it;
            QHash<KUrl, bt::TorrentFileInterface*>::const_iterator itEnd = m_files.constEnd();
            const Job::Status curentStatus = this->status();
            for (it = m_files.constBegin(); it != itEnd; ++it)
            {
                QModelIndex size = m_fileModel->index(it.key(), FileItem::Size);
                m_fileModel->setData(size, static_cast<qlonglong>((*it)->getSize()));

                const bool doDownload = !(*it)->doNotDownload();
                QModelIndex checkIndex = m_fileModel->index(it.key(), FileItem::File);
                const Qt::CheckState checkState = doDownload ? Qt::Checked : Qt::Unchecked;
                m_fileModel->setData(checkIndex, checkState, Qt::CheckStateRole);

                QModelIndex status = m_fileModel->index(it.key(), FileItem::Status);
                if (doDownload && (curentStatus == Job::Running))
                {
                    m_fileModel->setData(status, Job::Running);
                }
                else
                {
                    m_fileModel->setData(status, Job::Stopped);
                }
                if (qFuzzyCompare((*it)->getDownloadPercentage(), 100.0f))
                {
                    m_fileModel->setData(status, Job::Finished);
                }
            }
        }
        //one single file
        else
        {
            QList<KUrl> urls;
            KUrl temp = m_dest;
            if (m_dest.fileName() != torrent->getStats().torrent_name)//TODO check if the body is ever entered!
            {
                temp.addPath(torrent->getStats().torrent_name);
            }
            const KUrl url = temp;
            urls.append(url);

            m_fileModel = new FileModel(urls, directory(), this);
            //         connect(m_fileModel, SIGNAL(rename(KUrl,KUrl)), this, SLOT(slotRename(KUrl,KUrl)));
            connect(m_fileModel, SIGNAL(checkStateChanged()), this, SLOT(filesSelected()));

            QModelIndex size = m_fileModel->index(url, FileItem::Size);
            m_fileModel->setData(size, static_cast<qlonglong>(torrent->getStats().total_bytes));

            QModelIndex checkIndex = m_fileModel->index(url, FileItem::File);
            m_fileModel->setData(checkIndex, Qt::Checked, Qt::CheckStateRole);

            QModelIndex status = m_fileModel->index(url, FileItem::Status);
            if (this->status() == Job::Running)
            {
                m_fileModel->setData(status, Job::Running);
            }
            else
            {
                m_fileModel->setData(status, Job::Stopped);
            }
            if (!torrent->getStats().bytes_left_to_download)
            {
                m_fileModel->setData(status, Job::Finished);
            }
        }
    }

    return m_fileModel;
}

#include "bttransfer.moc"
