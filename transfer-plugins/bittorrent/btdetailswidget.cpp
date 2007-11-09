/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "btdetailswidget.h"

#include "bttransferhandler.h"
#include "btdetailswidgetfrm.h"

#include <kdebug.h>

BTDetailsWidget::BTDetailsWidget(BTTransferHandler * transfer)
    : m_transfer(transfer)
{
    Ui::BTDetailsWidgetFrm frm;
    frm.setupUi(this);

    findChild<QLabel *>("chunksTotalLabel")->setText(i18n("Total:"));
    findChild<QLabel *>("chunksDownloadedLabel")->setText(i18n("Downloaded:"));

    findChild<QLabel *>("peersConnectedLabel")->setText(i18n("Connected:"));
    findChild<QLabel *>("peersNotConnectedLabel")->setText(i18n("Not connected:"));

    m_chunksTotalLabel = findChild<QLabel *>("chunksTotalContentLabel");
    m_chunksDownloadedLabel = findChild<QLabel *>("chunksDownloadedContentLabel");
    m_peersConnectedLabel = findChild<QLabel *>("peersConnectedContentLabel");
    m_peersNotConnectedLabel = findChild<QLabel *>("peersNotConnectedContentLabel");

    transfer->addObserver(this);
    //This updates the widget with the right values
    transferChangedEvent(transfer);
}

void BTDetailsWidget::transferChangedEvent(TransferHandler * transfer)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if(transferFlags & BTTransfer::Tc_ChunksTotal)
    {
        int chunksTotal = m_transfer->chunksTotal();

        if(chunksTotal!=-1)
            m_chunksTotalLabel->setText(QString(chunksTotal));
        else
            m_chunksTotalLabel->setText(QString());
    }

    if(transferFlags & BTTransfer::Tc_ChunksDownloaded)
    {
        int chunksDownloaded = m_transfer->chunksDownloaded();

        if(chunksDownloaded!=-1)
            m_chunksDownloadedLabel->setText(QString(chunksDownloaded));
        else
            m_chunksDownloadedLabel->setText(QString());
    }

    if(transferFlags & BTTransfer::Tc_PeersConnected)
    {
        int peersConnected = m_transfer->peersConnected();

        if(peersConnected!=-1)
            m_peersConnectedLabel->setText(QString(peersConnected));
        else
            m_peersConnectedLabel->setText(QString());
    }

    if(transferFlags & BTTransfer::Tc_PeersNotConnected)
    {
        int peersNotConnected = m_transfer->peersNotConnected();

        if(peersNotConnected!=-1)
            m_peersNotConnectedLabel->setText(QString(peersNotConnected));
        else
            m_peersNotConnectedLabel->setText(QString());
    }

    m_transfer->resetChangesFlags(this);
}

#include "btdetailswidget.moc"
