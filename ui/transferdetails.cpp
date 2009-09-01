/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferdetails.h"

#include "core/kget.h"

#include <klocale.h>
#include <kio/global.h>
#include <kdebug.h>

#include <QVBoxLayout>

TransferDetails::TransferDetails(TransferHandler * transfer)
    : m_transfer(transfer)
{
    m_genericWidget = new QWidget(this);

    frm.setupUi(m_genericWidget);

    m_detailsWidget = KGet::factory(m_transfer)->createDetailsWidget(m_transfer);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_genericWidget);
    if (m_detailsWidget)
        m_layout->addWidget(m_detailsWidget);
    setLayout(m_layout);

    frm.sourceContentEdit->setText(m_transfer->source().prettyUrl());
    frm.destContentEdit->setText(m_transfer->dest().prettyUrl());

    frm.sourceContentEdit->setText(transfer->source().prettyUrl());
    frm.destContentEdit->setText(transfer->dest().prettyUrl());

    //This updates the widget with the right values
    slotTransferChanged(transfer, 0xFFFFFFFF);

    connect(transfer, SIGNAL(transferChangedEvent(TransferHandler *, TransferHandler::ChangesFlags)),
            this,     SLOT(slotTransferChanged(TransferHandler *, TransferHandler::ChangesFlags)));
}

TransferDetails::~TransferDetails()
{
}

QWidget *TransferDetails::detailsWidget(TransferHandler *handler)
{
    QWidget *details = KGet::factory(handler)->createDetailsWidget(handler);

    if (!details) { // the transfer factory doesn't override the details widget so use the generic one
        details = new TransferDetails(handler);
    }

    return details;
}

void TransferDetails::slotTransferChanged(TransferHandler * transfer, TransferHandler::ChangesFlags flags)
{
    kDebug(5001) << "TransferDetails::slotTransferChanged";

    Q_UNUSED(transfer);

    if(flags & Transfer::Tc_Status)
    {
        frm.statusPixmapContentLabel->setPixmap(m_transfer->statusPixmap());
        frm.statusTextContentLabel->setText(m_transfer->statusText());
    }

    if((flags & Transfer::Tc_TotalSize) || (flags & Transfer::Tc_DownloadedSize))
    {
        frm.completedContentLabel->setText(i18n("%1 of %2", KIO::convertSize(m_transfer->downloadedSize()), KIO::convertSize(m_transfer->totalSize())));
    }

    if(flags & Transfer::Tc_Percent)
    {
        frm.progressBar->setValue(m_transfer->percent());
    }

    if(flags & Transfer::Tc_DownloadSpeed)
    {
        int speed = m_transfer->downloadSpeed();

        if(speed==0)
        {
            if(m_transfer->status() == Job::Running)
                frm.speedContentLabel->setText(i18n("Stalled") );
            else
                frm.speedContentLabel->setText(QString());
        }
        else
            frm.speedContentLabel->setText(i18n("%1/s", KIO::convertSize(speed)));
    }

    if(flags & Transfer::Tc_FileName)
    {
        frm.destContentEdit->setText(m_transfer->dest().prettyUrl());
    }

    frm.remainingTimeLabel->setText(KIO::convertSeconds(m_transfer->remainingTime()));
}

#include "transferdetails.moc"
