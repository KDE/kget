/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferdetails.h"

#include "core/kget.h"

#include <KIO/Global>
#include <KLocalizedString>

#include "kget_debug.h"

#include <QDebug>
#include <QStyle>
#include <QVBoxLayout>

TransferDetails::TransferDetails(TransferHandler *transfer)
    : QWidget(nullptr)
    , m_transfer(transfer)
{
    m_genericWidget = new QWidget(this);

    frm.setupUi(m_genericWidget);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_genericWidget);
    setLayout(m_layout);

    frm.sourceContentEdit->setText(m_transfer->source().toString());
    frm.destContentEdit->setText(m_transfer->dest().toLocalFile());

    // This updates the widget with the right values
    slotTransferChanged(transfer, 0xFFFFFFFF);

    connect(transfer, &TransferHandler::transferChangedEvent, this, &TransferDetails::slotTransferChanged);
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

void TransferDetails::slotTransferChanged(TransferHandler *transfer, TransferHandler::ChangesFlags flags)
{
    qCDebug(KGET_DEBUG) << "TransferDetails::slotTransferChanged";

    Q_UNUSED(transfer)

    if (flags & Transfer::Tc_Status) {
        frm.statusPixmapContentLabel->setPixmap(
            QIcon::fromTheme(m_transfer->statusIconName()).pixmap(style()->pixelMetric(QStyle::PixelMetric::PM_SmallIconSize)));
        frm.statusTextContentLabel->setText(m_transfer->statusText());

        if (m_transfer->status() == Job::Finished) {
            frm.speedContentLabel->setText(i18n("Average speed: %1/s", KIO::convertSize(m_transfer->averageDownloadSped())));
        }
    }

    if ((flags & Transfer::Tc_TotalSize) || (flags & Transfer::Tc_DownloadedSize)) {
        frm.completedContentLabel->setText(i18n("%1 of %2", KIO::convertSize(m_transfer->downloadedSize()), KIO::convertSize(m_transfer->totalSize())));
    }

    if (flags & Transfer::Tc_Percent) {
        frm.progressBar->setValue(m_transfer->percent());
    }

    if ((flags & Transfer::Tc_DownloadSpeed) && (m_transfer->status() != Job::Finished)) {
        int speed = m_transfer->downloadSpeed();

        if (speed == 0) {
            if (m_transfer->status() == Job::Running)
                frm.speedContentLabel->setText(i18n("Stalled"));
            else
                frm.speedContentLabel->setText(QString());
        } else {
            frm.speedContentLabel->setText(i18n("%1/s", KIO::convertSize(speed)));
        }
    }

    if (flags & Transfer::Tc_FileName) {
        frm.destContentEdit->setText(m_transfer->dest().toLocalFile());
    }

    if (flags & Transfer::Tc_Source) {
        frm.sourceContentEdit->setText(m_transfer->source().toString());
    }

    frm.remainingTimeLabel->setText(KIO::convertSeconds(m_transfer->remainingTime()));
}
