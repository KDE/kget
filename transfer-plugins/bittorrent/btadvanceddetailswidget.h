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

#include "chunkdownloadviewitem.h"
#include "core/observer.h"

#include <interfaces/monitorinterface.h>
#include <torrent/torrentcontrol.h>

#include <QWidget>

class BTTransferHandler;
class BTFileTreeView;

class BTAdvancedDetailsWidget : public QWidget, public TransferObserver, public Ui::BTAdvancedDetailsWidget, public bt::MonitorInterface
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
        void updateTrackerGUI();
        void addTracker(const QString &url);
        void deleteTracker();
        void setDefaultTracker();
        void changeTracker();
        //PeerView
        void peerAdded(bt::PeerInterface* peer);
        void peerRemoved(bt::PeerInterface* peer);
        //ChunkView
        void updateChunkView();

        void downloadStarted(bt::ChunkDownloadInterface* chunk);
        void downloadRemoved(bt::ChunkDownloadInterface* chunk);
        void stopped();
        void destroyed() {}


    private:
        void hideEvent(QHideEvent * event);

        BTTransferHandler * m_transfer;

        bt::TorrentControl * tc;

        bt::PtrMap<bt::ChunkDownloadInterface*,ChunkDownloadViewItem> items;

        BTFileTreeView *fileTreeView;
};

#endif

