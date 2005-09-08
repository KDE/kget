/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <klocale.h>
#include <kio/global.h>

#include "transferdetails.h"
#include "transferdetailsfrm.h"

TransferDetails::TransferDetails(TransferHandler * transfer)
    : m_transfer(transfer)
{
    Ui::TransferDetailsFrm frm;
    frm.setupUi(this);

    findChild<QLabel *>("sourceLabel")->setText(i18n("Source:"));
    findChild<QLabel *>("destLabel")->setText(i18n("Saving to:"));
    findChild<QLabel *>("statusLabel")->setText(i18n("Status:"));

    findChild<QLabel *>("sourceContentLabel")->setText(transfer->source().url());
    findChild<QLabel *>("destContentLabel")->setText(transfer->dest().url());

    m_statusPixmapLabel = findChild<QLabel *>("statusPixmapContentLabel");
    m_statusTextLabel = findChild<QLabel *>("statusTextContentLabel");
    m_completedLabel = findChild<QLabel *>("completedContentLabel");
    m_speedLabel = findChild<QLabel *>("speedContentLabel");
    m_progressBar = findChild<QProgressBar *>("progressBar");

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
                m_speedLabel->setText("");
        }
        else
            m_speedLabel->setText(i18n("%1/s").arg(KIO::convertSize( speed )) );
    }


    m_transfer->resetChangesFlags(this);
}

#include "transferdetails.moc"
