/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "core/jobqueue.h"
#include "core/scheduler.h"

JobQueue::JobQueue(Scheduler * scheduler)
    : m_scheduler(scheduler),
      m_status(Stopped),
      m_maxSimultaneousJobs(2)
{
    m_scheduler->addQueue(this);
}

JobQueue::~JobQueue()
{
    m_scheduler->delQueue(this);
}

const QValueList<Job *> & JobQueue::runningJobs()
{
    QValueList<Job *> jobs;

    iterator it = begin();
    iterator itEnd = end();

    for( ; it!=itEnd ; ++it )
    {
        if( (*it)->status() == Running )
            jobs.append(*it);
    }
    return jobs;
}

void JobQueue::setStatus(Status queueStatus)
{
    m_status = queueStatus;
    m_scheduler->jobQueueChangedEvent(this, m_status);
}

void JobQueue::append(Job * job)
{
    m_jobs.append(job);
}

void JobQueue::prepend(Job * job)
{
    m_jobs.prepend(job);
}

void JobQueue::remove(Job * job)
{
    m_jobs.remove(job);
}

void JobQueue::move(Job * job, int position)
{
    if( m_jobs.remove(job) == 0)
        return;

    QValueList<Job *>::iterator it = m_jobs.at(position);
    if( it!=m_jobs.end() )
    {
        m_jobs.insert(it, job);
    }
}

int JobQueue::size() const
{
    return m_jobs.size();
}

