/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTADVANCEDDETAILSWIDGET_H
#define BTADVANCEDDETAILSWIDGET_H

#include "ui_btadvanceddetailswidget.h"

#include <interfaces/monitorinterface.h>
#include <torrent/torrentcontrol.h>

#include <QWidget>

#include "core/observer.h"

class BTTransferHandler;

class BTAdvancedDetailsWidget : public QWidget, public TransferObserver, public Ui::BTAdvancedDetailsWidget, bt::MonitorInterface
{
    Q_OBJECT
    public:
        BTAdvancedDetailsWidget(BTTransferHandler * transfer);
        ~BTAdvancedDetailsWidget();

        void transferChangedEvent(TransferHandler * transfer);
        void init();

     private slots:
        //TrackerView
        void updateTracker();
        void addTracker(const QString &url);
        void deleteTracker();
        void setDefaultTracker();
        void changeTracker();
        //PeerView
        void peerAdded(bt::PeerInterface* peer);
        void peerRemoved(bt::PeerInterface* peer);

        void downloadStarted(bt::ChunkDownloadInterface* chunk) {}
        void downloadRemoved(bt::ChunkDownloadInterface* chunk) {}
        void stopped() {}
        void destroyed() {}


    private:
        BTTransferHandler * m_transfer;

        bt::TorrentControl * tc;
};

#endif

