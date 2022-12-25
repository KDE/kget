/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransferhandler.h"

#include "advanceddetails/btadvanceddetailswidget.h"
#include "advanceddetails/monitor.h"
#include "scandlg.h"

#include "core/scheduler.h"

#include "kget_debug.h"

BTTransferHandler::BTTransferHandler(BTTransfer *transfer, Scheduler *scheduler)
    : TransferHandler(transfer, scheduler)
    , m_transfer(transfer)
{
    advancedDetails = nullptr;
    scanDlg = nullptr;
}

BTTransferHandler::~BTTransferHandler()
{
    if (advancedDetails)
        delete (advancedDetails);
}

void BTTransferHandler::createAdvancedDetails()
{
    if (!torrentControl())
        return;
    qCDebug(KGET_DEBUG);

    if (!advancedDetails) {
        qCDebug(KGET_DEBUG) << "Going to create AdvancedDetails";
        advancedDetails = new BTAdvancedDetailsWidget(this);
        advancedDetails->show();
        connect(advancedDetails, &BTAdvancedDetailsWidget::aboutToClose, this, &BTTransferHandler::removeAdvancedDetails);
        if (m_transfer->torrentControl()) {
            m_transfer->torrentControl()->setMonitor(nullptr);
            m_transfer->torrentControl()->setMonitor(m_transfer);
        }
    }
}

void BTTransferHandler::removeAdvancedDetails()
{
    advancedDetails->close();
    advancedDetails = nullptr;
}

kt::Monitor *BTTransferHandler::torrentMonitor() const
{
    if (advancedDetails)
        return advancedDetails->torrentMonitor();
    else
        return nullptr;
}

void BTTransferHandler::createScanDlg()
{
    if (!torrentControl())
        return;

    scanDlg = new kt::ScanDlg(m_transfer->torrentControl()->startDataCheck(false, 0, m_transfer->chunksTotal()), nullptr); // TODO: Maybe start/stop it
    scanDlg->exec();
}
