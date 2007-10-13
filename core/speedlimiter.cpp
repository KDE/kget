/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "speedlimiter.h"

#include <kdebug.h>

#include <QtCore/QTimer>
#include <QtCore/QList>


KSpeedLimiter::KSpeedLimiter(unsigned long speed)
    :m_speedLimit (speed)
{
}

KSpeedLimiter::~KSpeedLimiter()
{
}

void KSpeedLimiter::setSpeedLimit(unsigned long speed)
{
    m_speedLimit = speed;
}

unsigned long KSpeedLimiter::speedLimit()
{
    return m_speedLimit;
}

void KSpeedLimiter::addJob(KJob *job)
{
    if(!m_speedLimit)
        return;
    m_JobSpeedLimiters << new KJobSpeedLimiter(job, m_speedLimit);
    kDebug(5001) << "Limiting job:" << job << " speed to: " << m_speedLimit << endl;
}


KJobSpeedLimiter::KJobSpeedLimiter(KJob *job, unsigned long speedLimit)
{
    m_bytes = job->processedAmount(KJob::Bytes);
    m_job = job;
    m_speedLimit = speedLimit;
    m_channelSpeed = 0;
    m_OnTime = 0;
    m_OffTime = 0;
    connect( m_job, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));
    m_time.start();
    QTimer::singleShot(50, this, SLOT(slotOnTimer()));
}

KJobSpeedLimiter::~KJobSpeedLimiter()
{
}

void KJobSpeedLimiter::slotOnTimer()
{
    qulonglong bytes = m_job->processedAmount(KJob::Bytes);
    int expected = 1000.0 * (bytes - m_bytes) / m_speedLimit;
    m_bytes = bytes;
    int elapsed = m_time.restart();

    QString status = isSuspended() ? QString("suspended") : QString("running");
    kDebug(5001) << "Status: " << status << " expected time: " << expected << " elapsed time: " << elapsed << endl;

    if (expected > elapsed)
    {
        m_OnTime = (elapsed * 4500)/expected;
        m_OffTime = 4500 - m_OnTime;
        suspend();
        kDebug(5001) << "sleeping for: " << m_OffTime << " msegs." << endl;
        QTimer::singleShot(m_OffTime, this, SLOT(slotOffTimer()));
    }
    else
    {
        kDebug(5001) << "speed below limit, checking in 500 msegs " << endl;
        resume();
        QTimer::singleShot(500, this, SLOT(slotOnTimer()));
    }
}

void KJobSpeedLimiter::slotOffTimer()
{
    resume();
    m_time.restart();
    kDebug(5001) << "resuming for: " << m_OnTime << " msegs." << endl;
    QTimer::singleShot(m_OnTime, this, SLOT(slotOnTimer()));
}

void KJobSpeedLimiter::slotResult(KJob *job)
{
    Q_UNUSED(job);
    m_job = 0;
    deleteLater();
}

bool  KJobSpeedLimiter::isSuspended()
{
    m_isSuspended = m_job->isSuspended();
    return m_isSuspended;
}

void KJobSpeedLimiter::suspend()
{
    if(!isSuspended())
        m_job->suspend();
}

void KJobSpeedLimiter::resume()
{
    if(isSuspended())
        m_job->resume();
}
