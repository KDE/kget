/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "kgetglobaljob.h"

#include <klocale.h>

#include <QTimer>

KGetGlobalJob::KGetGlobalJob(QObject *parent)
    : KJob(parent), m_jobs()
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(false);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

KGetGlobalJob::~KGetGlobalJob()
{
    delete m_timer;
}

void KGetGlobalJob::registerJob(KJob *job)
{
    if(!m_jobs.contains(job)) {
        if(m_jobs.size() <= 0) {
            m_timer->start(DEFAULT_UPDATE_TIME);
        }
        m_jobs.append(job);
    }
}

void KGetGlobalJob::unregisterJob(KJob *job)
{
    m_jobs.removeAll(job);

    if(m_jobs.size() <= 0) {
        update();
        m_timer->stop();
    }
}

qulonglong KGetGlobalJob::processedAmount(Unit unit) const
{
    qulonglong amount = 0;
    foreach(KJob *child, m_jobs) {
        amount += child->processedAmount(unit);
    }

    return amount;
}

qulonglong KGetGlobalJob::totalAmount(Unit unit) const
{
    qulonglong amount = 0;
    foreach(KJob *child, m_jobs) {
        amount += child->totalAmount(unit);
    }

    return amount;
}

unsigned long KGetGlobalJob::percent() const
{
    if (totalAmount(KJob::Bytes) > 0) 
        return 100 * processedAmount(KJob::Bytes) / totalAmount(KJob::Bytes);
    else
        return 0;
}

void KGetGlobalJob::update()
{
    emit description(this, "KGet global information", 
                    qMakePair(QString("source"), i18np("KGet is downloading %1 file", "KGet is downloading %1 files", m_jobs.size())),
                    qMakePair(QString("destination"), QString("to different locations")));

    setProcessedAmount(KJob::Bytes, processedAmount(KJob::Bytes));
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Bytes));
    setPercent(percent());
}
