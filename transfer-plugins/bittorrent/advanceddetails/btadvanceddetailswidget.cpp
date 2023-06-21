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

#include "bittorrentsettings.h"
#include "bttransferhandler.h"
#include "chunkdownloadview.h"
#include "fileview.h"
#include "monitor.h"
#include "peerview.h"
#include "trackerview.h"
#include "webseedstab.h"

#include "kget_debug.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KTitleWidget>
#include <QDialog>
#include <QGridLayout>
#include <QTabWidget>

using namespace kt;

BTAdvancedDetailsWidget::BTAdvancedDetailsWidget(BTTransferHandler *transfer)
    : m_transfer(transfer)
{
    tc = m_transfer->torrentControl();

    init();

    // This updates the widget with the right values
    slotTransferChanged(transfer, 0xFFFFFFFF);

    connect(m_transfer, &TransferHandler::transferChangedEvent, this, &BTAdvancedDetailsWidget::slotTransferChanged);
}

void BTAdvancedDetailsWidget::init()
{
    setWindowTitle(i18n("Advanced Details for %1", m_transfer->source().fileName()));
    resize(500, 400);
    auto *layout = new QGridLayout();
    auto *titleWidget = new KTitleWidget(this);
    titleWidget->setText(i18n("Advanced Details for %1", m_transfer->source().fileName()));
    titleWidget->setIcon(QIcon::fromTheme("dialog-information"));
    layout->addWidget(titleWidget);
    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
    file_view = new FileView(this);
    file_view->changeTC(tc, KSharedConfig::openConfig());
    tabWidget->insertTab(0, file_view, QIcon::fromTheme("inode-directory"), i18n("Files"));
    // peer_view = new PeerView(this);
    // tabWidget->insertTab(1, peer_view, QIcon::fromTheme("system-users"), i18n("Peers"));
    // cd_view = new ChunkDownloadView(this);
    // cd_view->changeTC(tc);
    // tabWidget->insertTab(2, cd_view, QIcon::fromTheme("preferences-plugin"), i18n("Chunks"));
    tracker_view = new TrackerView(this);
    tracker_view->changeTC(tc);
    tabWidget->insertTab(1, tracker_view, QIcon::fromTheme("network-server"), i18n("Trackers"));
    webseeds_tab = new WebSeedsTab(this);
    webseeds_tab->changeTC(tc);
    tabWidget->insertTab(2, webseeds_tab, QIcon::fromTheme("network-server"), i18n("Webseeds"));
    monitor = new Monitor(tc, nullptr, nullptr, file_view);
}

void BTAdvancedDetailsWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    if (tc)
        tc->setMonitor(nullptr);
    Q_EMIT aboutToClose();
    deleteLater();
}

kt::Monitor *BTAdvancedDetailsWidget::torrentMonitor() const
{
    return monitor;
}

void BTAdvancedDetailsWidget::slotTransferChanged(TransferHandler *transfer, TransferHandler::ChangesFlags flags)
{
    qCDebug(KGET_DEBUG) << "BTAdvancedDetailsWidget::slotTransferChanged";

    Q_UNUSED(transfer)

    if (flags & BTTransfer::Tc_ChunksTotal || flags & BTTransfer::Tc_ChunksDownloaded || flags & BTTransfer::Tc_ChunksExcluded
        || flags & BTTransfer::Tc_ChunksLeft || flags & Transfer::Tc_DownloadSpeed || flags & Transfer::Tc_UploadSpeed) {
        // if (tabWidget->currentIndex() == 1)
        //         peer_view->update();
        // else if (tabWidget->currentIndex() == 2)
        //         cd_view->update();
        /*else */ if (tabWidget->currentIndex() == 1)
            tracker_view->update();
    }
    /*else if (m_transfer->status() == Job::Stopped)
    {
        peer_view->removeAll();
        //cd_view->removeAll();
    }*/
}

#include "moc_btadvanceddetailswidget.cpp"
