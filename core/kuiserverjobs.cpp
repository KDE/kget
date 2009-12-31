/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2009 by Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kuiserverjobs.h"

#include "kgetglobaljob.h"
#include "transferhandler.h"
#include "settings.h"
#include "kget.h"

#include <kuiserverjobtracker.h>
#include <kdebug.h>

KUiServerJobs::KUiServerJobs(QObject *parent)
    : QObject(parent), m_globalJob(0)
{
}

KUiServerJobs::~KUiServerJobs()
{
    while(m_registeredJobs.size()) {
        unregisterJob(m_registeredJobs.begin().value(), m_registeredJobs.begin().key());
    }

    if(m_globalJob) {
        delete m_globalJob;
    }
}

void KUiServerJobs::settingsChanged()
{
    QList<TransferHandler *> transfers = KGet::allTransfers();

    foreach(TransferHandler * transfer, transfers) {
        if(shouldBeShown(transfer))
            registerJob((KJob*) transfer->kJobAdapter(), transfer);
        else
            unregisterJob((KJob*) transfer->kJobAdapter(), transfer);
    }
    
    // GlobalJob is associated to a virtual transfer pointer of value == 0
    if(shouldBeShown(0))
        registerJob(globalJob(), 0);
    else
        unregisterJob(globalJob(), 0);
}

void KUiServerJobs::slotTransferAdded(TransferHandler * transfer, TransferGroupHandler * group)
{
    kDebug(5001);
    
    if(shouldBeShown(transfer))
        registerJob((KJob*) transfer->kJobAdapter(), transfer);
    
    if(shouldBeShown(0)) {
        globalJob()->update();
        registerJob(globalJob(), 0);
    }
    else
        unregisterJob(globalJob(), 0);
}

void KUiServerJobs::slotTransferAboutToBeRemoved(TransferHandler * transfer, TransferGroupHandler * group)
{
    kDebug(5001);
    
    m_invalidTransfers.append(transfer);
    
    unregisterJob((KJob*) transfer->kJobAdapter(), transfer);
    
    if(shouldBeShown(0)) {
        globalJob()->update();
        registerJob(globalJob(), 0);
    }
    else
        unregisterJob(globalJob(), 0);
}

void KUiServerJobs::slotTransfersChanged(QMap<TransferHandler *, Transfer::ChangesFlags> transfers)
{
    kDebug(5001);
    
    if(!Settings::enableKUIServerIntegration())
        return;
    
    QMapIterator<TransferHandler *, Transfer::ChangesFlags> i(transfers);
    while (i.hasNext()) {
        i.next();
//         if(!m_invalidTransfers.contains(i.key()))
        {
            TransferHandler * transfer = i.key();
            
            if(shouldBeShown(transfer))
                registerJob((KJob*) transfer->kJobAdapter(), transfer);
            else 
                unregisterJob((KJob*) transfer->kJobAdapter(), transfer);
        }
    }
    
    if(shouldBeShown(0)) {
        globalJob()->update();
        registerJob(globalJob(), 0);
    }
    else
        unregisterJob(globalJob(), 0);
}

void KUiServerJobs::registerJob(KJob * job, TransferHandler * transfer)
{
    if(m_registeredJobs.contains(transfer) || !job)
        return;
    
    KIO::getJobTracker()->registerJob(job);
    m_registeredJobs[transfer] = job;
}

void KUiServerJobs::unregisterJob(KJob * job, TransferHandler * transfer)
{
    if(!m_registeredJobs.contains(transfer)  || !job)
        return;
    
    KIO::getJobTracker()->unregisterJob(m_registeredJobs[transfer]);
    m_registeredJobs.remove(transfer);
}

bool KUiServerJobs::shouldBeShown(TransferHandler * transfer)
{
    if(!Settings::enableKUIServerIntegration())
        return false;
    
    if(Settings::exportGlobalJob() && (transfer == 0) && existRunningTransfers())
        return true;
    
    if(!Settings::exportGlobalJob() && (transfer) && (transfer->status() == Job::Running))
        return true;
    
    return false;
}

bool KUiServerJobs::existRunningTransfers()
{
    foreach(TransferHandler * transfer, KGet::allTransfers()) {
        if(transfer->status() == Job::Running)
            return true;
    }
    
    return false;
}

KGetGlobalJob * KUiServerJobs::globalJob()
{
    if(!m_globalJob)
        m_globalJob = new KGetGlobalJob();
    
    return m_globalJob;
}

