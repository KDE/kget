/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btadvanceddetailswidget.h"

#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>

#include "bttransferhandler.h"
#include "torrentfiletreemodel.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kglobal.h>

BTAdvancedDetailsWidget::BTAdvancedDetailsWidget(BTTransferHandler * transfer)
    : m_transfer(transfer)
{
    /**A lot of code is from KTorrent's infowidget by Joris Guisson, thx for the nice work**/
    setupUi(this);

    tc = m_transfer->torrentControl();

    tc->setMonitor(this);

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
    setWindowTitle("Advanced-Details for" + m_transfer->source().fileName());
    const KUrl::List trackers = tc->getTrackersList()->getTrackerURLs();

    if (trackers.empty())
    {
        trackerList->addItem(tc->getTrackersList()->getTrackerURL().prettyUrl());
    }
    else
    {
        foreach (KUrl u,trackers)
            trackerList->addItem(u.prettyUrl());
    }
    updateTracker();

    kt::TorrentFileTreeModel *fileTree = new kt::TorrentFileTreeModel(tc,kt::TorrentFileTreeModel::DeselectMode(1),this);
    fileTreeView->setModel(fileTree);

    const bt::TorrentStats & s = tc->getStats();
    totalChunksLabel->setText(QString::number(s.total_chunks));
    sizeChunkLabel->setText(KGlobal::locale()->formatByteSize(s.chunk_size));

    connect(deleteTrackerButton, SIGNAL(clicked()), SLOT(deleteTracker()));
    connect(updateTrackerButton, SIGNAL(clicked()), SLOT(updateTracker()));
    connect(addTrackerButton, SIGNAL(clicked()), SLOT(addTracker()));
    connect(changeTrackerButton, SIGNAL(clicked()), SLOT(changeTracker()));
    connect(defaultTrackerButton, SIGNAL(clicked()), SLOT(setDefaultTracker()));
}

void BTAdvancedDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if(transferFlags && Transfer::Tc_Status)
    {
        if (m_transfer->statusText() != "Stopped")
            peersTreeWidget->update();
        if (m_transfer->statusText() == "Stopped")
            peersTreeWidget->removeAll();
    }

    updateChunkView();

    m_transfer->resetChangesFlags(this);
}

void BTAdvancedDetailsWidget::updateTracker()
{
    kDebug(5001);
    const bt::TorrentStats & s = tc->getStats();

    if (s.running)
    {
        QTime t;
        t = t.addSecs(tc->getTimeToNextTrackerUpdate());
        trackerUpdateTime->setText(t.toString("mm:ss"));
    }

    //Update manual annunce button
    updateTrackerButton->setEnabled(s.running);  // && tc->announceAllowed()
    // only enable change when we can actually change and the torrent is running
    changeTrackerButton->setEnabled(s.running); // && tc->getTrackersList(). > 1

    trackerStatus->setText("<b>" + s.trackerstatus + "</b>");

    if (tc->getTrackersList())
        trackerUrl->setText("<b>" + tc->getTrackersList()->getTrackerURL().prettyUrl() + "</b>");
    else
        trackerUrl->clear();
}

void BTAdvancedDetailsWidget::addTracker(const QString &url)
{
}

void BTAdvancedDetailsWidget::deleteTracker()
{
    kDebug(5001);
    QListWidgetItem* current = trackerList->currentItem();
    if(!current)
        return;
    
    KUrl url(current->text());
    if(tc->getTrackersList()->removeTracker(url))
        delete current;
    else
        KMessageBox::sorry(0, i18n("Cannot remove torrent default tracker."));
}

void BTAdvancedDetailsWidget::setDefaultTracker()
{
    kDebug(5001);
    tc->getTrackersList()->restoreDefault();
    tc->updateTracker();
		
    // update the list of trackers
    trackerList->clear();
		
    const KUrl::List trackers = tc->getTrackersList()->getTrackerURLs();
    if(trackers.empty())
        return;
		
    foreach (KUrl u,trackers)
        trackerList->addItem(u.prettyUrl());
}

void BTAdvancedDetailsWidget::changeTracker()
{
    kDebug(5001);
    QListWidgetItem* current = trackerList->currentItem();
    if(!current)
        return;
		
    KUrl url(current->text());
    tc->getTrackersList()->setTracker(url);
    tc->updateTracker();
}

void BTAdvancedDetailsWidget::peerAdded(bt::PeerInterface* peer)
{
    peersTreeWidget->peerAdded(peer);
}

void BTAdvancedDetailsWidget::peerRemoved(bt::PeerInterface* peer)
{
    peersTreeWidget->peerRemoved(peer);
}

void BTAdvancedDetailsWidget::downloadStarted(bt::ChunkDownloadInterface* chunk)
{
    items.insert(chunk, new ChunkDownloadViewItem(chunkTreeWidget,chunk));
}

void BTAdvancedDetailsWidget::downloadRemoved(bt::ChunkDownloadInterface* chunk)
{
    ChunkDownloadViewItem* v = items.find(chunk);
    if (v)
    {
        items.erase(chunk);
        delete v;
    }
}

void BTAdvancedDetailsWidget::updateChunkView()
{
    bt::PtrMap<bt::ChunkDownloadInterface*,ChunkDownloadViewItem>::iterator i = items.begin();
    while (i != items.end())
    {
        i->second->update(false);
        i++;
    }

    const bt::TorrentStats & s = tc->getStats();
    downloadingChunksLabel->setText(QString::number(s.num_chunks_downloading));
    downloadedChunksLabel->setText(QString::number(s.num_chunks_downloaded));
    excludedChunksLabel->setText(QString::number(s.num_chunks_excluded));
    leftChunksLabel->setText(QString::number(s.num_chunks_left));
}

#include "btadvanceddetailswidget.moc"
 
