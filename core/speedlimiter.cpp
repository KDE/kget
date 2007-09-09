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

using namespace KIO;

KSpeedLimiter::KSpeedLimiter()
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
    m_jobs << job;
    connect( job, SIGNAL(result(KJob *)), SLOT(slotResult( KJob *)));
}

void KSpeedLimiter::start()
{
    if(!m_speedLimit)
        return;
    if(!m_timer)
        m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeOut()),SLOT(slotTimer()));
    m_timer->start(1000);
}

void KSpeedLimiter::stop()
{
    if(m_timer)
        m_timer->stop();
    QList<KJob *>::const_iterator it = m_jobs.constBegin();
    QList<KJob *>::const_iterator end = m_jobs.constEnd();
    for ( ; it != end ; ++it )
        (*it)->resume();

}

void KSpeedLimiter::slotTimer()
{
    QList<KJob *>::const_iterator it = m_jobs.constBegin();
    QList<KJob *>::const_iterator end = m_jobs.constEnd();
    for ( ; it != end ; ++it )
    {
        (*it)->resume();
    }
}

void KSpeedLimiter::slotResult(KJob *job)
{
    m_jobs.removeAll(job);
    if(m_jobs.isEmpty())
        deleteLater();
}
