/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTTRANSFER_H
#define BTTRANSFER_H

#include <QTimer>

#include "core/transfer.h"
#include "torrent/torrentcontrol.h"

class BTTransfer : public QObject, public Transfer
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
            Tc_SeedsConnected         = 0x00160000,
            Tc_SeedsDisconnected      = 0x00320000,
            Tc_LeechesConnected       = 0x00640000,
            Tc_LeechesDisconnected    = 0x01280000,
            Tc_DlRate                 = 0x02560000,
            Tc_UlRate                 = 0x05120000,
            Tc_UlLimit                = 0x07860000,
            Tc_DlLimit                = 0x09840000,
            Tc_SessionBytesDownloaded = 0x10240000,
            Tc_SessionBytesUploaded   = 0x20480000,
            Tc_TrackersList           = 0x40960000
        };

        BTTransfer(TransferGroup* parent, TransferFactory* factory,
                    Scheduler* scheduler, const KUrl& src, const KUrl& dest,
                    const QDomElement * e = 0);
        ~BTTransfer();

        //Job virtual functions
        void start();
        void stop();
        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

        //Bittorrent specific functions (connected with TransferFlags
        int chunksTotal() const;
        int chunksDownloaded() const;
        int chunksExcluded() const;
        int chunksLeft() const;
        int seedsConnected() const;
        int seedsDisconnected() const;
        int leechesConnected() const;
        int leechesDisconnected() const;
        int dlRate() const;
        int ulRate() const;
        float totalSize() const;
        float processedSize() const;
        int sessionBytesDownloaded() const;
        int sessionBytesUploaded() const;
        KUrl::List trackersList() const;
        bt::TorrentControl * torrentControl();
        int ulLimit() const;
        int dlLimit() const;
        int percent() const;

        //More Bittorrent-Functions
        void setPort(int port);
        void setTrafficLimits(int ulLimit, int dlLimit);
        void setMaxShareRatio(float ratio);
        void addTracker(QString url);
        void save(const QDomElement &element);

        bool ready();

    protected:
        void load(const QDomElement &e);

    private slots:
        void init(KUrl src = KUrl());
        void update();
        void slotStoppedByError(bt::TorrentInterface* error, QString errormsg);
        void slotDownloadFinished(bt::TorrentInterface* ti);

    private:
        void startTorrent();
        void stopTorrent();
        void updateTorrent();

        bt::TorrentControl *torrent;
        QString m_tmp;
        int m_dlLimit;
        int m_ulLimit;
        float m_ratio;
        QTimer timer;
        bool m_ready;
};

#endif
