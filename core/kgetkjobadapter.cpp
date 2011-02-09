/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday @ gmail.com>
   Idea by Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "kgetkjobadapter.h"

#include <KLocale>

KGetKJobAdapter::KGetKJobAdapter(QObject *parent, TransferHandler *transfer)
  : KJob(parent),
    m_transferHandler(transfer)
{
    setCapabilities(Killable | Suspendable);
}

KGetKJobAdapter::~KGetKJobAdapter()
{
}

qulonglong KGetKJobAdapter::processedAmount(Unit unit) const
{
    Q_UNUSED(unit)
    return m_transferHandler->downloadedSize();
}

qulonglong KGetKJobAdapter::totalAmount(Unit unit) const
{
    Q_UNUSED(unit)
    return m_transferHandler->totalSize();
}

unsigned long KGetKJobAdapter::percent() const
{
    return m_transferHandler->percent();
}

void KGetKJobAdapter::slotUpdateDescription()
{
    emit description(this, i18n("KGet Transfer"), 
                    qMakePair(QString("source"), m_transferHandler->source().prettyUrl()),
                    qMakePair(QString("destination"), m_transferHandler->dest().prettyUrl()));

    emitSpeed(m_transferHandler->downloadSpeed());
    setProcessedAmount(KJob::Bytes, processedAmount(KJob::Bytes));
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Bytes));
    setPercent(percent());
}

bool KGetKJobAdapter::doKill()
{
    kDebug(5001) << "Kill of job adapter called:" << this << m_transferHandler->dest();
    emit requestStop(this, m_transferHandler);
    return KJob::doKill();
}

bool KGetKJobAdapter::doSuspend()
{
    if (m_transferHandler->capabilities() & Transfer::Cap_Resuming) {
        emit requestSuspend(this, m_transferHandler);
        return true;
    }

    return false;
}

bool KGetKJobAdapter::doResume()
{
    emit requestResume(this, m_transferHandler);
    return true;
}
