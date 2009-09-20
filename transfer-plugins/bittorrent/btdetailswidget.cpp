/* This file is part of the KDE project

   Copyright (C) 2007 - 2008 Lukas Appelhans <l.appelhans@gmx.de>

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

    // Update the view with the correct values
    srcEdit->setText(transfer->source().url());
    destEdit->setText(transfer->dest().url());

    seederLabel->setText(i18nc("not available", "n/a"));
    leecherLabel->setText(i18nc("not available", "n/a"));
    chunksDownloadedLabel->setText(i18nc("not available", "n/a"));
    chunksExcludedLabel->setText(i18nc("not available", "n/a"));
    chunksAllLabel->setText(i18nc("not available", "n/a"));
    chunksLeftLabel->setText(i18nc("not available", "n/a"));
    dlSpeedLabel->setText(i18nc("not available", "n/a"));
    ulSpeedLabel->setText(i18nc("not available", "n/a"));

    progressBar->setValue(m_transfer->percent());
    
    connect(m_transfer, SIGNAL(transferChangedEvent(TransferHandler *, TransferHandler::ChangesFlags)),
            this,       SLOT(slotTransferChanged(TransferHandler *, TransferHandler::ChangesFlags)));
}

BTDetailsWidget::~BTDetailsWidget()
{
}

void BTDetailsWidget::slotTransferChanged(TransferHandler * transfer, TransferHandler::ChangesFlags flags)
{
    Q_UNUSED(transfer)

    kDebug(5001) << "BTDetailsWidget::slotTransferChanged";
    
    if(flags & Transfer::Tc_DownloadSpeed)
        dlSpeedLabel->setText(KGlobal::locale()->formatByteSize(m_transfer->downloadSpeed()) + "/s");

    if(flags & Transfer::Tc_UploadSpeed)
        ulSpeedLabel->setText(KGlobal::locale()->formatByteSize(m_transfer->uploadSpeed()) + "/s");

    if(flags & BTTransfer::Tc_SeedsConnected)
        seederLabel->setText(QString().setNum(m_transfer->seedsConnected()) + '(' + QString().setNum(m_transfer->seedsDisconnected()) + ')');

    if(flags & BTTransfer::Tc_LeechesConnected)
        leecherLabel->setText(QString().setNum(m_transfer->leechesConnected()) + '(' + QString().setNum(m_transfer->leechesDisconnected()) + ')');

    if(flags & BTTransfer::Tc_ChunksDownloaded)
        chunksDownloadedLabel->setText(QString().setNum(m_transfer->chunksDownloaded()));

    if(flags & BTTransfer::Tc_ChunksExcluded)
        chunksExcludedLabel->setText(QString().setNum(m_transfer->chunksExcluded()));

    if(flags & BTTransfer::Tc_ChunksTotal)
        chunksAllLabel->setText(QString().setNum(m_transfer->chunksTotal()));

    if(flags & BTTransfer::Tc_ChunksLeft)
        chunksLeftLabel->setText(QString().setNum(m_transfer->chunksLeft()));

    if(flags & Transfer::Tc_Percent)
        progressBar->setValue(m_transfer->percent());

    if(flags & Transfer::Tc_FileName)
        destEdit->setText(m_transfer->dest().prettyUrl());
}

void BTDetailsWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    slotTransferChanged(m_transfer, 0xFFFFFFFF);
}


#include "btdetailswidget.moc"
