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
}

BTDetailsWidget::~BTDetailsWidget()
{
}

void BTDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    Q_UNUSED(transfer);
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if(transferFlags & Transfer::Tc_DownloadSpeed)
        dlSpeedLabel->setText(KGlobal::locale()->formatByteSize(m_transfer->downloadSpeed()));

    if(transferFlags & Transfer::Tc_UploadSpeed)
        ulSpeedLabel->setText(KGlobal::locale()->formatByteSize(m_transfer->uploadSpeed()));

    if(transferFlags & BTTransfer::Tc_SeedsConnected)
        seederLabel->setText(QString().setNum(m_transfer->seedsConnected()) + '(' + QString().setNum(m_transfer->seedsDisconnected()) + ')');

    if(transferFlags & BTTransfer::Tc_LeechesConnected)
        leecherLabel->setText(QString().setNum(m_transfer->leechesConnected()) + '(' + QString().setNum(m_transfer->leechesDisconnected()) + ')');

    if(transferFlags & BTTransfer::Tc_ChunksDownloaded)
        chunksDownloadedLabel->setText(QString().setNum(m_transfer->chunksDownloaded()));

    if(transferFlags & BTTransfer::Tc_ChunksExcluded)
        chunksExcludedLabel->setText(QString().setNum(m_transfer->chunksExcluded()));

    if(transferFlags & BTTransfer::Tc_ChunksTotal)
        chunksAllLabel->setText(QString().setNum(m_transfer->chunksTotal()));

    if(transferFlags & BTTransfer::Tc_ChunksLeft)
        chunksLeftLabel->setText(QString().setNum(m_transfer->chunksLeft()));

    if(transferFlags & Transfer::Tc_Percent)
        progressBar->setValue(m_transfer->percent());

    m_transfer->resetChangesFlags(this);
}

void BTDetailsWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)
    m_transfer->addObserver(this);
}

void BTDetailsWidget::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event)
    m_transfer->delObserver(this);
}

#include "btdetailswidget.moc"
