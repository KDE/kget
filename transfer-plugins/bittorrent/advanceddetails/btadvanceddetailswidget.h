/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2007 Joris Guisson   <joris.guisson@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTADVANCEDDETAILSWIDGET_H
#define BTADVANCEDDETAILSWIDGET_H

#include <torrent/torrentcontrol.h>

#include <QWidget>

#include "core/transferhandler.h"

class BTTransferHandler;
class QTabWidget;

namespace kt
{
    class PeerView;
    class ChunkDownloadView;
    class FileView;
    class Monitor;
    class TrackerView;
    class WebSeedsTab;
    }

class BTAdvancedDetailsWidget : public QWidget
{
    Q_OBJECT
    public:
        BTAdvancedDetailsWidget(BTTransferHandler * transfer);

        void deleteEvent(TransferHandler * transfer){/**FIXME: Implement this^^**/ Q_UNUSED(transfer) }
        void init();
        kt::Monitor* torrentMonitor() const;

    public Q_SLOTS:
        void slotTransferChanged(TransferHandler * transfer, TransferHandler::ChangesFlags flags);
        
    Q_SIGNALS:
         void aboutToClose();


    private:
        void hideEvent(QHideEvent * event) override;

        QTabWidget *tabWidget;

        BTTransferHandler * m_transfer;
        //kt::PeerView* peer_view;
	//kt::ChunkDownloadView* cd_view;
	kt::FileView* file_view;
	kt::Monitor* monitor; 
        kt::TrackerView *tracker_view;
        kt::WebSeedsTab *webseeds_tab;

        bt::TorrentControl * tc;
};

#endif

