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
#include "btfiletreeview.h"
#include "bittorrentsettings.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kglobal.h>
#include <kdialog.h>
#include <QTimer>

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
    setWindowTitle(i18n("Advanced-Details for %1", m_transfer->source().fileName()));

    fileTreeView = new BTFileTreeView(tc, tabWidget->widget(0));
    tabWidget->widget(0)->layout()->addWidget(fileTreeView);

    updateTrackerList();

    const bt::TorrentStats & s = tc->getStats();
    totalChunksLabel->setText(QString::number(s.total_chunks));
    sizeChunkLabel->setText(KGlobal::locale()->formatByteSize(s.chunk_size));

    /**Set Column-widths**/

    QList<int> fileColumnWidths = BittorrentSettings::fileColumnWidths();
    if (!fileColumnWidths.isEmpty())
    {
        int j = 0;
        foreach(int i, fileColumnWidths)
        {
            fileTreeView->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        fileTreeView->setColumnWidth(0 , 250);
    }

    QList<int> peersColumnWidths = BittorrentSettings::peersColumnWidths();
    if (!peersColumnWidths.isEmpty())
    {
        int j = 0;
        foreach(int i, peersColumnWidths)
        {
            peersTreeWidget->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        peersTreeWidget->setColumnWidth(0 , 250);
    }

    QList<int> chunksColumnWidths = BittorrentSettings::chunksColumnWidths();
    if (!chunksColumnWidths.isEmpty())
    {
        int j = 0;
        foreach(int i, chunksColumnWidths)
        {
            chunkTreeWidget->setColumnWidth(j, i);
            j++;
        }
    }
    else
    {
        chunkTreeWidget->setColumnWidth(0 , 250);
    }

    connect(deleteTrackerButton, SIGNAL(clicked()), SLOT(deleteTracker()));
    connect(updateTrackerButton, SIGNAL(clicked()), SLOT(updateTracker()));
    connect(addTrackerButton, SIGNAL(clicked()), SLOT(addTracker()));
    connect(changeTrackerButton, SIGNAL(clicked()), SLOT(changeTracker()));
    connect(defaultTrackerButton, SIGNAL(clicked()), SLOT(setDefaultTracker()));
}

void BTAdvancedDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    kDebug(5001);
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if (m_transfer->status() == Job::Running)
    {
        updateChunkView();
        peersTreeWidget->update();
        updateTrackerGUI();
    }

    m_transfer->resetChangesFlags(this);
}

void BTAdvancedDetailsWidget::updateTracker()
{
    kDebug(5001);
    tc->updateTracker();
    if (tc->getStats().trackerstatus != "Ok")
        QTimer::singleShot(1000, this, SLOT(updateTracker()));

    QTimer::singleShot(1000, this, SLOT(updateTrackerGUI()));
}

void BTAdvancedDetailsWidget::updateTrackerGUI()
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
    updateTrackerButton->setEnabled(s.running); // && tc->announceAllowed()
    // only enable change when we can actually change and the torrent is running
    changeTrackerButton->setEnabled(s.running && tc->getTrackersList()->getTrackerURLs().size() > 1);

    trackerStatus->setText("<b>" + s.trackerstatus + "</b>");

    if (tc->getTrackersList())
        trackerUrl->setText("<b>" + tc->getTrackersList()->getTrackerURL().prettyUrl() + "</b>");
    else
        trackerUrl->clear();
}

void BTAdvancedDetailsWidget::addTracker()
{
    kDebug(5001);
    addTrackerDialog = new KDialog(this);
    QWidget * widget = new QWidget(this);
    addTrackerWidget.setupUi(widget);
    addTrackerDialog->setMainWidget(widget);
    addTrackerDialog->setButtons(KDialog::Ok | KDialog::Cancel);
    addTrackerDialog->exec();
    
    if (addTrackerDialog->result() == 1)//Dialog accepted
    {
        m_transfer->addTracker(addTrackerWidget.lineEdit->text());
    }

    QTimer::singleShot(1000, this, SLOT(updateTrackerList()));
}

void BTAdvancedDetailsWidget::updateTrackerList()
{
    kDebug(5001);
    trackerList->clear();
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
    kDebug(5001);
    peersTreeWidget->peerAdded(peer);
}

void BTAdvancedDetailsWidget::peerRemoved(bt::PeerInterface* peer)
{
    kDebug(5001);
    peersTreeWidget->peerRemoved(peer);
}

void BTAdvancedDetailsWidget::downloadStarted(bt::ChunkDownloadInterface* chunk)
{
    kDebug(5001);
    items.insert(chunk, new ChunkDownloadViewItem(chunkTreeWidget, chunk, tc));
}

void BTAdvancedDetailsWidget::downloadRemoved(bt::ChunkDownloadInterface* chunk)
{
    kDebug(5001);
    ChunkDownloadViewItem* v = items.find(chunk);
    if (v)
    {
        items.erase(chunk);
        delete v;
    }
}

void BTAdvancedDetailsWidget::stopped()
{
    kDebug(5001);
    if (m_transfer->status() != Job::Running)
    { //Cleanup Chunk- and Peers-View when a transfer stops
        peersTreeWidget->removeAll();
        chunkTreeWidget->clear();
        items.clear();
    }
    tc->setMonitor(0);
    deleteLater();
}

void BTAdvancedDetailsWidget::updateChunkView()
{
    kDebug(5001);

    if (items.count() == 0 || !tc)
        return;

    bt::PtrMap<bt::ChunkDownloadInterface*,ChunkDownloadViewItem>::iterator i = items.begin();

    for (i;  i != items.end(); i++)
    {
        if (i->second)
            i->second->update(false);
    }

    const bt::TorrentStats & s = tc->getStats();
    downloadingChunksLabel->setText(QString::number(s.num_chunks_downloading));
    downloadedChunksLabel->setText(QString::number(s.num_chunks_downloaded));
    excludedChunksLabel->setText(QString::number(s.num_chunks_excluded));
    leftChunksLabel->setText(QString::number(s.num_chunks_left));
}

void BTAdvancedDetailsWidget::hideEvent(QHideEvent * event)
{
    kDebug(5001) << "Save config";
    QList<int>  fileColumnWidths;
    for (int i = 0; i<1; i++)
    {
        fileColumnWidths.append(fileTreeView->columnWidth(i));
    }
    BittorrentSettings::setFileColumnWidths(fileColumnWidths);

    QList<int>  peersColumnWidths;
    for (int i = 0; i<13; i++)
    {
        peersColumnWidths.append(peersTreeWidget->columnWidth(i));
    }
    BittorrentSettings::setPeersColumnWidths(peersColumnWidths);

    QList<int>  chunksColumnWidths;
    for (int i = 0; i<3; i++)
    {
        chunksColumnWidths.append(chunkTreeWidget->columnWidth(i));
    }
    BittorrentSettings::setChunksColumnWidths(chunksColumnWidths);
    BittorrentSettings::self()->writeConfig();
}

#include "btadvanceddetailswidget.moc"
 
