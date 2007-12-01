/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btdetailswidget.h"

#include "bttransferhandler.h"
#include "ui_btdetailswidgetfrm.h"

#include <kdebug.h>

BTDetailsWidget::BTDetailsWidget(BTTransferHandler * transfer)
    : m_transfer(transfer)
{
    setupUi(this);

    srcEdit->setText(transfer->source().url());
    destEdit->setText(transfer->dest().url());

    transfer->addObserver(this);
    //This updates the widget with the right values
    transferChangedEvent(transfer);
}

void BTDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    /**seederLabel->setText(transfer->seedsConnected() + "(" + transfer->seedsDisconnected() + ")");
    leecherLabel->setText(transfer->leechesConnected() + "(" + transfer->leechesDisconnected() + ")");
    dlSpeedLabel->setText(transfer->dlRate());
    ulSpeedLabel->setText(transfer->ulRate());
    chunksDownloadedLabel->setText(transfer->chunksDownloaded());
    chunksExcludedLabel->setText(transfer->chunksExcluded());
    chunksAllLabel->setText(transfer->chunksTotal());
    chunksLeftLabel->setText(transfer->chunksLeft());**/

    m_transfer->resetChangesFlags(this);
}

#include "btdetailswidget.moc"
