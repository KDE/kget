/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btadvanceddetailswidget.h"

#include "bttransferhandler.h"

#include <kdebug.h>

BTAdvancedDetailsWidget::BTAdvancedDetailsWidget(BTTransferHandler * transfer)
    : m_transfer(transfer)
{
    setupUi(this);

    m_transfer->torrentControl()->setMonitor(this);

    transfer->addObserver(this);
    //This updates the widget with the right values
    transferChangedEvent(transfer);
}

BTAdvancedDetailsWidget::~BTAdvancedDetailsWidget()
{
    m_transfer->delObserver(this);
}

void BTAdvancedDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    peersTreeWidget->update();

    m_transfer->resetChangesFlags(this);
}

void BTAdvancedDetailsWidget::peerAdded(bt::PeerInterface* peer)
{
    peersTreeWidget->peerAdded(peer);
}	

void BTAdvancedDetailsWidget::peerRemoved(bt::PeerInterface* peer)
{
    peersTreeWidget->peerRemoved(peer);
}
#include "btadvanceddetailswidget.moc"
 
