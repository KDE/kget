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

TransferGroupScheduler::TransferGroupScheduler()
  : Scheduler(),
    m_downloadLimit(0),
    m_uploadLimit(0)
{
}

TransferGroupScheduler::~TransferGroupScheduler()
{
}

void TransferGroupScheduler::jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status)
{
    Scheduler::jobQueueChangedEvent(queue, status);
}

void TransferGroupScheduler::jobQueueMovedJobEvent(JobQueue * queue, Job * job)
{
    Scheduler::jobQueueMovedJobEvent(queue, job);
}

void TransferGroupScheduler::jobQueueAddedJobEvent(JobQueue * queue, Job * job)
{
    Scheduler::jobQueueAddedJobEvent(queue, job);
}

void TransferGroupScheduler::jobQueueRemovedJobEvent(JobQueue * queue, Job * job)
{
    Scheduler::jobQueueRemovedJobEvent(queue, job);
}

void TransferGroupScheduler::jobChangedEvent(Job * job, Job::Status status)
{
    Scheduler::jobChangedEvent(job, status);
}

void TransferGroupScheduler::jobChangedEvent(Job * job, Job::Policy status)
{
    Scheduler::jobChangedEvent(job, status);
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
        if (handler->transfers().count() < 1)
        {
            kDebug(5001) << "Adding downloadLimit for group " + handler->name() + " to pool";
            pool = pool + downloadLimit() / n;
        }
        else if (downloadLimit() == 0 && handler->visibleDownloadLimit() != 0)
            continue;
        else if (downloadLimit() == 0 && handler->visibleDownloadLimit() == 0)
            handler->setDownloadLimit(0);
        else if (handler->visibleDownloadLimit() < downloadLimit() / n && handler->visibleDownloadLimit() != 0)
            /*If the handler's visible download limit is under the new one, 
                           we move the KiB/s which are different to the pool*/
            pool = pool + (downloadLimit() / n - handler->visibleDownloadLimit());
        else if (handler->downloadSpeed() + 10 < downloadLimit() / n)
        {
            kDebug(5001) << "Ok the download speed man is tooooo low, so we have a small buffer";
            /*When the downloadSpeed of the handler is under the new downloadLimit + 10 then we 
                    set the downloadLimit to the downloadSpeed + 10*/
            pool = pool + downloadLimit() / n - handler->downloadSpeed() + 10;
            handler->setDownloadLimit(handler->downloadSpeed() + 10);
        }
        else
        {
            kDebug(5001) << "Ok the generic sollution xD";
            handler->setDownloadLimit(downloadLimit() / n);
            transfergroupsNeedSpeed.append(handler);
        }
    }
    foreach (TransferGroupHandler *handler, transfergroupsNeedSpeed)
    {
        kDebug(5001) << "The pool is: " + QString::number(pool);
        handler->setDownloadLimit(downloadLimit() / n + pool / transfergroupsNeedSpeed.count());
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
        if (handler->transfers().count() < 1)
            pool = pool + uploadLimit() / n;
        else if (uploadLimit() == 0 && handler->visibleUploadLimit() != 0)
            continue;
        else if (uploadLimit() == 0 && handler->visibleUploadLimit() == 0)
            handler->setUploadLimit(0);
        else if (handler->visibleUploadLimit() < uploadLimit() / n && handler->visibleUploadLimit() != 0)
            /*If the handler's visible download limit is under the new one, 
                           we move the KiB/s which are different to the pool*/
            pool = pool + (uploadLimit() / n - handler->visibleUploadLimit());       
        else if (handler->uploadSpeed() + 10 < uploadLimit() / n)
        {
            kDebug(5001) << "Ok the download speed is tooooo low, so we have a small buffer";
            /*When the downloadSpeed of the handler is under the new downloadLimit + 10 then we 
                    set the downloadLimit to the downloadSpeed + 10*/
            pool = pool + uploadLimit() / n - handler->uploadSpeed() + 10;
            handler->setUploadLimit(handler->uploadSpeed() + 10);
        }
        else
        {
            kDebug(5001) << "Ok the generic sollution xD";
            handler->setUploadLimit(uploadLimit() / n);
            transfergroupsNeedSpeed.append(handler);
        }
    }
    foreach (TransferGroupHandler *handler, transfergroupsNeedSpeed)
    {
        handler->setUploadLimit(uploadLimit() / n + pool / transfergroupsNeedSpeed.count());
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
