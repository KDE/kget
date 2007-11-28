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
#include <QDateTime>
#include <QByteArray>

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
            Tc_PeersConnected         = 0x00040000,
            Tc_PeersNotConnected      = 0x00080000,
            Tc_DlRate                 = 0x00160000,
            Tc_UlRate                 = 0x00320000,
            Tc_SessionBytesDownloaded = 0x00640000,
            Tc_SessionBytesUploaded   = 0x01280000,
            Tc_TrackersList           = 0x02560000
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
        int chunksTotal();
        int chunksDownloaded();
        int peersConnected();
        int peersNotConnected();
        int dlRate();
        int ulRate();
	int sessionBytesDownloaded();
	int sessionBytesUploaded();
        KUrl::List trackersList();

        //More Bittorrent-Functions
        void setPort(int port);
        void save(QDomElement e); // krazy:exclude=passbyvalue

    protected:
        void load(const QDomElement &e);

    private slots:
        void update();
        void slotStoppedByError(bt::TorrentInterface* error, QString errormsg);
        void slotDownloadFinished(bt::TorrentInterface* ti);

    private:
        //TODO: are all these functions necessary??
        void hashingFinished();

        int m_chunksTotal;
        int m_chunksDownloaded;
        int m_peersConnected;
        int m_peersNotConnected;

        bt::TorrentControl *torrent;
        bt::TorrentStats   *stats;

        QTimer timer;
};

#endif
