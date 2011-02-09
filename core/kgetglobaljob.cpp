/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2009 by Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "kgetglobaljob.h"
#include "transferhandler.h"
#include "kget.h"

#include <klocale.h>
#include <kuiserverjobtracker.h>

#include <QTimer>

KGetGlobalJob::KGetGlobalJob(QObject *parent)
    : KJob(parent)
{
    setCapabilities(Killable);
}

KGetGlobalJob::~KGetGlobalJob()
{
}


void KGetGlobalJob::update()
{
    int runningTransfers = 0;
    qulonglong processedAmount = 0;
    qulonglong totalAmount = 0;
    unsigned long speed = 0;
    unsigned long percent = 0;
    
    foreach(TransferHandler * transfer, KGet::allTransfers()) {
        if(transfer->status() == Job::Running) {
            runningTransfers++;
            processedAmount += transfer->downloadedSize();
            speed += transfer->downloadSpeed();
            totalAmount += transfer->totalSize();
        }
    }
  
//     kDebug(5001) << totalAmount;
  
    if (totalAmount > 0) 
        percent = 100 * processedAmount / totalAmount;
    else
        percent =  0;
  
    emit description(this, "KGet global information", 
                      qMakePair(QString("source"), i18np("KGet is downloading %1 file", "KGet is downloading %1 files", runningTransfers)));

    emitSpeed(speed);
    setTotalAmount(KJob::Bytes, totalAmount);
    setProcessedAmount(KJob::Bytes, processedAmount);

    setPercent(percent);
}

bool KGetGlobalJob::doKill()
{
    kDebug(5001) << "Kill of global job called:" << this;
    emit requestStop(this, 0);
    return KJob::doKill();
}

