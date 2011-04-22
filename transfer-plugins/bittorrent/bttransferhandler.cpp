/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransferhandler.h"

#include "bttransfer.h"
#include "advanceddetails/monitor.h"
#include "advanceddetails/btadvanceddetailswidget.h"
#include "scandlg.h"

#include "core/scheduler.h"

#include <KDebug>

BTTransferHandler::BTTransferHandler(BTTransfer * transfer, Scheduler * scheduler)
    : TransferHandler(transfer, scheduler),
      m_transfer(transfer)
{
    advancedDetails = 0;
    scanDlg = 0;
}

BTTransferHandler::~BTTransferHandler()
{
    if(advancedDetails)
        delete(advancedDetails);
}

void BTTransferHandler::createAdvancedDetails()
{
    if (!torrentControl())
        return;
    kDebug(5001);

    if (!advancedDetails)
    {
        kDebug(5001) << "Going to create AdvancedDetails";
        advancedDetails = new BTAdvancedDetailsWidget(this);
        advancedDetails->show();
        connect(advancedDetails, SIGNAL(aboutToClose()), SLOT(removeAdvancedDetails()));
        if (m_transfer->torrentControl())
        {
            m_transfer->torrentControl()->setMonitor(0);
            m_transfer->torrentControl()->setMonitor(m_transfer);
        }
    }
}

void BTTransferHandler::removeAdvancedDetails()
{
    advancedDetails->close();
    advancedDetails = 0;
}

kt::Monitor* BTTransferHandler::torrentMonitor() const
{
    if (advancedDetails)
        return advancedDetails->torrentMonitor();
    else
        return 0;
}

void BTTransferHandler::createScanDlg()
{
    if (!torrentControl())
        return;
#if LIBKTORRENT_VERSION < 0x010100
    if (scanDlg)
    {
        scanDlg->stop();
        scanDlg->close();
    }
#endif

#if LIBKTORRENT_VERSION >= 0x010200
    scanDlg = new kt::ScanDlg(m_transfer->torrentControl()->startDataCheck(false, 0, m_transfer->chunksTotal()), 0);//TODO: Maybe start/stop it
    scanDlg->exec();
#elif LIBKTORRENT_VERSION >= 0x010100
    scanDlg = new kt::ScanDlg(m_transfer->torrentControl()->startDataCheck(false), 0);//TODO: Maybe start/stop it
    scanDlg->exec();
#else
    scanDlg = new kt::ScanDlg(0);
    scanDlg->show();
    scanDlg->execute(torrentControl(), false);
    connect(scanDlg, SIGNAL(finished(int)), SLOT(removeScanDlg()));
#endif
}
#if LIBKTORRENT_VERSION < 0x010100
void BTTransferHandler::removeScanDlg()
{
    kDebug(5001);
    scanDlg = 0;
}
#endif
