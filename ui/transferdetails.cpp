/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferdetails.h"

#include "ui_transferdetailsfrm.h"
#include "core/kget.h"

#include <klocale.h>
#include <kio/global.h>
#include <kdebug.h>

#include <QVBoxLayout>

TransferDetails::TransferDetails(TransferHandler * transfer)
    : m_transfer(transfer)
{
    m_genericWidget = new QWidget(this);

    Ui::TransferDetailsFrm frm;
    frm.setupUi(m_genericWidget);

    m_detailsWidget = KGet::factory(transfer)->createDetailsWidget(transfer);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_genericWidget);
    if (m_detailsWidget)
        m_layout->addWidget(m_detailsWidget);
    setLayout(m_layout);

    frm.sourceLabel->setText(i18nc("@label transfer source", "Source:"));
    frm.destLabel->setText(i18n("Saving to:"));
    frm.statusLabel->setText(i18n("Status:"));

    frm.sourceContentEdit->setText(transfer->source().prettyUrl());
    frm.destContentEdit->setText(transfer->dest().prettyUrl());

    m_statusPixmapLabel = frm.statusPixmapContentLabel;
    m_statusTextLabel = frm.statusTextContentLabel;
    m_completedLabel = frm.completedContentLabel;
    m_speedLabel = frm.speedContentLabel;
    m_progressBar = frm.progressBar;
    m_remainingTimeLabel = frm.remainingTimeLabel;

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
        m_statusPixmapLabel->setPixmap(m_transfer->statusPixmap());
        m_statusTextLabel->setText(m_transfer->statusText());
    }

    if((flags & Transfer::Tc_TotalSize) || (flags & Transfer::Tc_DownloadedSize))
    {
        m_completedLabel->setText(i18n("%1 of %2", KIO::convertSize(m_transfer->downloadedSize()), KIO::convertSize(m_transfer->totalSize())));
    }

    if(flags & Transfer::Tc_Percent)
    {
        m_progressBar->setValue(m_transfer->percent());
    }

    if(flags & Transfer::Tc_DownloadSpeed)
    {
        int speed = m_transfer->downloadSpeed();

        if(speed==0)
        {
            if(m_transfer->status() == Job::Running)
                m_speedLabel->setText(i18n("Stalled") );
            else
                m_speedLabel->setText(QString());
        }
        else
            m_speedLabel->setText(i18n("%1/s", KIO::convertSize(speed)));
    }

    m_remainingTimeLabel->setText(KIO::convertSeconds(m_transfer->remainingTime()));
}

#include "transferdetails.moc"
