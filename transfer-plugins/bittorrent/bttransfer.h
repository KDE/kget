/* This file is part of the KDE project

   Copyright (C) 2007 - 2010 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTTRANSFER_H
#define BTTRANSFER_H

#include "core/transfer.h"
#include "torrent/torrentcontrol.h"

#include <interfaces/monitorinterface.h>

#include <QTimer>

class FileModel;

namespace bt
{
    class ChunkDownloadInterface;
    class TorrentFileInterface;
    class PeerInterface;
}

class BTTransfer : public Transfer, public bt::MonitorInterface
{
    Q_OBJECT

    public:
        /**
         * Here we define the Bittorrent specific flags.
         */
        enum BTTransferChange
        {
            Tc_ChunksTotal            = 0x00010000,
            Tc_ChunksDownloaded       = 0x00020000,
            Tc_ChunksExcluded         = 0x00040000,
            Tc_ChunksLeft             = 0x00080000,
            Tc_SeedsConnected         = 0x00100000,
            Tc_SeedsDisconnected      = 0x00200000,
            Tc_LeechesConnected       = 0x00400000,
            Tc_LeechesDisconnected    = 0x00800000,
            Tc_SessionBytesDownloaded = 0x04000000,
            Tc_SessionBytesUploaded   = 0x08000000,
            Tc_TrackersList           = 0x10000000
        };

        BTTransfer(TransferGroup* parent, TransferFactory* factory,
                    Scheduler* scheduler, const KUrl& src, const KUrl& dest,
                    const QDomElement * e = 0);
        ~BTTransfer();

        void deinit(Transfer::DeleteOptions options);

        //Job virtual functions        
        void start();
        void stop();
        virtual int elapsedTime() const;
        virtual int remainingTime() const;
        virtual bool isStalled() const;
        virtual bool isWorking() const;

        /**
         * @returns the directory the Transfer will be stored to
         */
        virtual KUrl directory() const {return m_directory;}

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        virtual bool setDirectory(const KUrl &newDirectory);

        virtual QList<KUrl> files() const;

        FileModel *fileModel();

        //Bittorrent specific functions (connected with TransferFlags
        int chunksTotal() const;
        int chunksDownloaded() const;
        int chunksExcluded() const;
        int chunksLeft() const;
        int seedsConnected() const;
        int seedsDisconnected() const;
        int leechesConnected() const;
        int leechesDisconnected() const;
        int sessionBytesDownloaded() const;
        int sessionBytesUploaded() const;
        KUrl::List trackersList() const;
        bt::TorrentControl * torrentControl();

        //More Bittorrent-Functions
        void setPort(int port);
        void addTracker(const QString &url);
        //void save(const QDomElement &element);

        bool ready();

    protected:
        void load(const QDomElement *element);
        void setSpeedLimits(int ulLimit, int dlLimit);

    private slots:
        void btTransferInit(const KUrl &src = KUrl(), const QByteArray &data = QByteArray());
        void update();
        void slotStoppedByError(const bt::TorrentInterface* &error, const QString &errormsg);
        void slotDownloadFinished(bt::TorrentInterface* ti);
        void newDestResult();
        void filesSelected();

    private:
        void startTorrent();
        void stopTorrent();
        void updateTorrent();
        void updateFilesStatus();

        // bt::MonitorInterface functions
        virtual void downloadRemoved(bt::ChunkDownloadInterface* cd);
        virtual void downloadStarted(bt::ChunkDownloadInterface* cd);
        virtual void peerAdded(bt::PeerInterface* peer);
        virtual void peerRemoved(bt::PeerInterface* peer);
        virtual void stopped();
        virtual void destroyed();
        virtual void filePercentageChanged(bt::TorrentFileInterface*, float) {}
        virtual void filePreviewChanged(bt::TorrentFileInterface*, bool) {}

        bt::TorrentControl *torrent;
        KUrl m_directory;
        QString m_tmp;
        QString m_tmpTorrentFile;
        float m_ratio;
        QTimer timer;
        bool m_ready;
        bool m_downloadFinished;
        bool m_movingFile;
        FileModel *m_fileModel;
        QHash<KUrl, bt::TorrentFileInterface*> m_files;
        int m_updateCounter;
};

#endif
