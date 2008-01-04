/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "kgetglobaljob.h"

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
    if(m_jobs.size() <= 0) {
        m_timer->start(DEFAULT_UPDATE_TIME);
    }
    m_jobs.append(job);
}

void KGetGlobalJob::unregisterJob(KJob *job)
{
    m_jobs.removeAll(job);

    if(m_jobs.size() <= 0) {
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
    return 100 * processedAmount(KJob::Bytes) / totalAmount(KJob::Bytes);
}

void KGetGlobalJob::update()
{
    emit description(this, "KGet global information", 
                    qMakePair(QString("source"), QString("KGet is downloading %1 files").arg(m_jobs.size())),
                    qMakePair(QString("destination"), QString("to different locations")));

    setProcessedAmount(KJob::Bytes, processedAmount(KJob::Bytes));
    setTotalAmount(KJob::Bytes, totalAmount(KJob::Bytes));
    setPercent(percent());
}
