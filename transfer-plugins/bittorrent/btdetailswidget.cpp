/* This file is part of the KDE project

   Copyright (C) 2007 - 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btdetailswidget.h"

#include "bttransferhandler.h"

#include "kget_debug.h"

#include <KFormat>

BTDetailsWidget::BTDetailsWidget(BTTransferHandler *transfer)
    : m_transfer(transfer)
{
    setupUi(this);

    // Update the view with the correct values
    srcEdit->setText(transfer->source().toDisplayString(QUrl::PreferLocalFile));
    destEdit->setText(transfer->dest().toDisplayString(QUrl::PreferLocalFile));

    seederLabel->setText(i18nc("not available", "n/a"));
    leecherLabel->setText(i18nc("not available", "n/a"));
    chunksDownloadedLabel->setText(i18nc("not available", "n/a"));
    chunksExcludedLabel->setText(i18nc("not available", "n/a"));
    chunksAllLabel->setText(i18nc("not available", "n/a"));
    chunksLeftLabel->setText(i18nc("not available", "n/a"));
    dlSpeedLabel->setText(i18nc("not available", "n/a"));
    ulSpeedLabel->setText(i18nc("not available", "n/a"));

    progressBar->setValue(m_transfer->percent());

    connect(m_transfer, &TransferHandler::transferChangedEvent, this, &BTDetailsWidget::slotTransferChanged);
}

BTDetailsWidget::~BTDetailsWidget()
{
}

void BTDetailsWidget::slotTransferChanged(TransferHandler *transfer, TransferHandler::ChangesFlags flags)
{
    Q_UNUSED(transfer)

    qCDebug(KGET_DEBUG) << "BTDetailsWidget::slotTransferChanged";

    if (flags & Transfer::Tc_DownloadSpeed)
        dlSpeedLabel->setText(KFormat().formatByteSize(m_transfer->downloadSpeed()) + "/s");

    if (flags & Transfer::Tc_UploadSpeed)
        ulSpeedLabel->setText(KFormat().formatByteSize(m_transfer->uploadSpeed()) + "/s");

    if (flags & BTTransfer::Tc_SeedsConnected)
        seederLabel->setText((m_transfer->seedsConnected() != -1 ? QString().setNum(m_transfer->seedsConnected()) : i18nc("not available", "n/a")) + " ("
                             + (m_transfer->seedsDisconnected() != -1 ? QString().setNum(m_transfer->seedsDisconnected()) : i18nc("not available", "n/a"))
                             + ')');

    if (flags & BTTransfer::Tc_LeechesConnected)
        leecherLabel->setText((m_transfer->leechesConnected() != -1 ? QString().setNum(m_transfer->leechesConnected()) : i18nc("not available", "n/a")) + " ("
                              + (m_transfer->leechesDisconnected() != -1 ? QString().setNum(m_transfer->leechesDisconnected()) : i18nc("not available", "n/a"))
                              + ')');

    if (flags & BTTransfer::Tc_ChunksDownloaded)
        chunksDownloadedLabel->setText(m_transfer->chunksDownloaded() != -1 ? QString().setNum(m_transfer->chunksDownloaded()) : i18nc("not available", "n/a"));

    if (flags & BTTransfer::Tc_ChunksExcluded)
        chunksExcludedLabel->setText(m_transfer->chunksExcluded() != -1 ? QString().setNum(m_transfer->chunksExcluded()) : i18nc("not available", "n/a"));

    if (flags & BTTransfer::Tc_ChunksTotal)
        chunksAllLabel->setText((m_transfer->chunksTotal() != -1) ? QString().setNum(m_transfer->chunksTotal()) : i18nc("not available", "n/a"));

    if (flags & BTTransfer::Tc_ChunksLeft)
        chunksLeftLabel->setText((m_transfer->chunksLeft() != -1) ? QString().setNum(m_transfer->chunksLeft()) : i18nc("not available", "n/a"));

    if (flags & Transfer::Tc_Percent)
        progressBar->setValue(m_transfer->percent());

    if (flags & Transfer::Tc_FileName)
        destEdit->setText(m_transfer->dest().toDisplayString(QUrl::PreferLocalFile));
}

void BTDetailsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    slotTransferChanged(m_transfer, 0xFFFFFFFF);
}

#include "moc_btdetailswidget.cpp"
