/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "transfergroupscheduler.h"

#include "kget.h"
#include "transfergrouphandler.h"
#include "settings.h"

TransferGroupScheduler::TransferGroupScheduler(QObject *parent)
  : Scheduler(parent),
    m_downloadLimit(0),
    m_uploadLimit(0)
{
}

TransferGroupScheduler::~TransferGroupScheduler()
{
}

void TransferGroupScheduler::calculateSpeedLimits()
{
    calculateDownloadLimit();
    calculateUploadLimit();
}

void TransferGroupScheduler::calculateDownloadLimit()
{
    int n = KGet::allTransferGroups().count();
    int pool = 0;//We create a pool where we have some KiB/s to go to other groups...
    QList<TransferGroupHandler*> transfergroupsNeedSpeed;
    foreach (TransferGroupHandler *handler, KGet::allTransferGroups())
    {
        if (!Settings::speedLimit())
            handler->setDownloadLimit(handler->downloadLimit(Transfer::VisibleSpeedLimit), Transfer::InvisibleSpeedLimit);
        else if (handler->transfers().count() < 1)
        {
            pool = pool + downloadLimit() / n;
        }
        else if (downloadLimit() == 0 && handler->downloadLimit(Transfer::VisibleSpeedLimit) != 0)
            continue;
        else if (downloadLimit() == 0 && handler->downloadLimit(Transfer::VisibleSpeedLimit) == 0)
            handler->setDownloadLimit(0, Transfer::InvisibleSpeedLimit);
        else if (handler->downloadLimit(Transfer::VisibleSpeedLimit) < downloadLimit() / n 
                                               && handler->downloadLimit(Transfer::VisibleSpeedLimit) != 0)
            /*If the handler's visible download limit is under the new one, 
                           we move the KiB/s which are different to the pool*/
            pool = pool + (downloadLimit() / n - handler->downloadLimit(Transfer::VisibleSpeedLimit));
        else if (handler->downloadSpeed() + 10 < downloadLimit() / n)
        {
            /*When the downloadSpeed of the handler is under the new downloadLimit + 10 then we 
                    set the downloadLimit to the downloadSpeed + 10*/
            pool = pool + downloadLimit() / n - handler->downloadSpeed() + 10;
            handler->setDownloadLimit(handler->downloadSpeed() + 10, Transfer::InvisibleSpeedLimit);
        }
        else
        {
            handler->setDownloadLimit(downloadLimit() / n, Transfer::InvisibleSpeedLimit);
            transfergroupsNeedSpeed.append(handler);
        }
    }
    foreach (TransferGroupHandler *handler, transfergroupsNeedSpeed)
    {
        handler->setDownloadLimit(downloadLimit() / n + pool / transfergroupsNeedSpeed.count(), Transfer::InvisibleSpeedLimit);
    }
}

void TransferGroupScheduler::calculateUploadLimit()
{
    int n = KGet::allTransferGroups().count();
    kDebug(5001) << n;
    int pool = 0;//We create a pool where we have some KiB/s to go to other groups...
    QList<TransferGroupHandler*> transfergroupsNeedSpeed;
    foreach (TransferGroupHandler *handler, KGet::allTransferGroups())
    {
        if (!Settings::speedLimit())
            handler->setUploadLimit(handler->uploadLimit(Transfer::VisibleSpeedLimit), Transfer::InvisibleSpeedLimit);
        else if (handler->transfers().count() < 1)
            pool = pool + uploadLimit() / n;
        else if (uploadLimit() == 0 && handler->uploadLimit(Transfer::VisibleSpeedLimit) != 0)
            continue;
        else if (uploadLimit() == 0 && handler->uploadLimit(Transfer::VisibleSpeedLimit) == 0)
            handler->setUploadLimit(0, Transfer::InvisibleSpeedLimit);
        else if (handler->uploadLimit(Transfer::VisibleSpeedLimit) < uploadLimit() / n && handler->uploadLimit(Transfer::VisibleSpeedLimit) != 0)
            /*If the handler's visible download limit is under the new one, 
                           we move the KiB/s which are different to the pool*/
            pool = pool + (uploadLimit() / n - handler->uploadLimit(Transfer::VisibleSpeedLimit));       
        else if (handler->uploadSpeed() + 10 < uploadLimit() / n)
        {
            /*When the downloadSpeed of the handler is under the new downloadLimit + 10 then we 
                    set the downloadLimit to the downloadSpeed + 10*/
            pool = pool + uploadLimit() / n - handler->uploadSpeed() + 10;
            handler->setUploadLimit(handler->uploadSpeed() + 10, Transfer::InvisibleSpeedLimit);
        }
        else
        {
            handler->setUploadLimit(uploadLimit() / n, Transfer::InvisibleSpeedLimit);
            transfergroupsNeedSpeed.append(handler);
        }
    }
    foreach (TransferGroupHandler *handler, transfergroupsNeedSpeed)
    {
        handler->setUploadLimit(uploadLimit() / n + pool / transfergroupsNeedSpeed.count(), Transfer::InvisibleSpeedLimit);
    }
}

void TransferGroupScheduler::setDownloadLimit(int limit)
{
    m_downloadLimit = limit;
    calculateDownloadLimit();
}

void TransferGroupScheduler::setUploadLimit(int limit)
{
    m_uploadLimit = limit;
    calculateUploadLimit();
}

#include "transfergroupscheduler.moc"
