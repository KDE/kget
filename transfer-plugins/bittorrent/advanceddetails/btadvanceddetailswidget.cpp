/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2007 Joris Guisson   <joris.guisson@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btadvanceddetailswidget.h"

#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>

#include "bttransferhandler.h"
#include "bittorrentsettings.h"
#include "fileview.h"
#include "chunkdownloadview.h"
#include "peerview.h"
#include "monitor.h"
#include "trackerview.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kdialog.h>
#include <QGridLayout>

using namespace kt;

BTAdvancedDetailsWidget::BTAdvancedDetailsWidget(BTTransferHandler * transfer)
    : m_transfer(transfer)
{
    tc = m_transfer->torrentControl();

    init();

    transfer->addObserver(this);
    //This updates the widget with the right values
    transferChangedEvent(transfer);
}

BTAdvancedDetailsWidget::~BTAdvancedDetailsWidget()
{
    m_transfer->delObserver(this);
}

void BTAdvancedDetailsWidget::init()
{
    setWindowTitle(i18n("Advanced-Details for %1", m_transfer->source().fileName()));
    QGridLayout *layout = new QGridLayout();
    QTabWidget *tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
    file_view = new FileView(this);
    file_view->changeTC(tc, KGlobal::config());
    tabWidget->insertTab(0, file_view, i18n("Files"));
    peer_view = new PeerView(this);
    tabWidget->insertTab(1, peer_view, i18n("Peers"));
    cd_view = new ChunkDownloadView(this);
    cd_view->changeTC(tc);
    tabWidget->insertTab(2, cd_view, i18n("Chunks"));
    tracker_view = new TrackerView(this);
    tracker_view->changeTC(tc);
    tabWidget->insertTab(3, tracker_view, i18n("Trackers"));
    monitor = new Monitor(tc,peer_view,cd_view);
}

void BTAdvancedDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    kDebug(5001);
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if (m_transfer->status() == Job::Running)
    {
        peer_view->update();
        cd_view->update();
        tracker_view->update();
    }
    else
    {
    }

    m_transfer->resetChangesFlags(this);
}

void BTAdvancedDetailsWidget::hideEvent(QHideEvent * event)
{
    tc->setMonitor(0);
    deleteLater();
}

#include "btadvanceddetailswidget.moc"
 
