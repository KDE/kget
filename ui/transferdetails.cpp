/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QVBoxLayout>

#include <klocale.h>
#include <kio/global.h>

#include "core/kget.h"

#include "transferdetails.h"
#include "ui_transferdetailsfrm.h"

TransferDetails::TransferDetails(TransferHandler * transfer)
    : m_transfer(transfer)
{
    m_genericWidget = new QWidget(this);

    Ui::TransferDetailsFrm frm;
    frm.setupUi(m_genericWidget);

    m_detailsWidget = KGet::factory(transfer)->createDetailsWidget(transfer);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_genericWidget);
    m_layout->addWidget(m_detailsWidget);
    setLayout(m_layout);

    frm.sourceLabel->setText(i18n("Source:"));
    frm.destLabel->setText(i18n("Saving to:"));
    frm.statusLabel->setText(i18n("Status:"));

    frm.sourceContentEdit->setText(transfer->source().url());
    frm.destContentEdit->setText(transfer->dest().url());

    m_statusPixmapLabel = frm.statusPixmapContentLabel;
    m_statusTextLabel = frm.statusTextContentLabel;
    m_completedLabel = frm.completedContentLabel;
    m_speedLabel = frm.speedContentLabel;
    m_progressBar = frm.progressBar;

    transfer->addObserver(this);
    //This updates the widget with the right values
    transferChangedEvent(transfer);
}

void TransferDetails::transferChangedEvent(TransferHandler * transfer)
{
    TransferHandler::ChangesFlags transferFlags = m_transfer->changesFlags(this);

    if(transferFlags & Transfer::Tc_Status)
    {
        m_statusPixmapLabel->setPixmap(m_transfer->statusPixmap());
        m_statusTextLabel->setText(m_transfer->statusText());
    }

    if(transferFlags & Transfer::Tc_TotalSize)
    {
        m_completedLabel->setText(KIO::convertSize(m_transfer->processedSize())
                                  + i18n(" of ") +
                                  KIO::convertSize(m_transfer->totalSize()));
    }

    if(transferFlags & Transfer::Tc_ProcessedSize)
    {
        m_completedLabel->setText(KIO::convertSize(m_transfer->processedSize())
                                  + i18n(" of ") +
                                  KIO::convertSize(m_transfer->totalSize()));
    }

    if(transferFlags & Transfer::Tc_Percent)
    {
        m_progressBar->setValue(m_transfer->percent());
    }

    if(transferFlags & Transfer::Tc_Speed)
    {
        int speed = m_transfer->speed();

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


    m_transfer->resetChangesFlags(this);
}

#include "transferdetails.moc"
