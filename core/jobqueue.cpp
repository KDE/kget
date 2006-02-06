/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/jobqueue.h"
#include "core/scheduler.h"

JobQueue::JobQueue(Scheduler * scheduler)
    : m_maxSimultaneousJobs(2),
      m_scheduler(scheduler),
      m_status(Stopped)
{
    m_scheduler->addQueue(this);
}

JobQueue::~JobQueue()
{
    m_scheduler->delQueue(this);
}

const QList<Job *> JobQueue::runningJobs()
{
    QList<Job *> jobs;

    iterator it = begin();
    iterator itEnd = end();

    for( ; it!=itEnd ; ++it )
    {
        if( (*it)->status() == Job::Running )
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
    m_jobs.removeAll(job);
}

void JobQueue::move(Job * job, Job * after)
{
    kDebug() << "JobQueue::move" << endl;

    if( (m_jobs.removeAll(job) == 0) || (job == after) )
    {
        //The job doesn't belong to this JobQueue or the requested
        //operations doesn't make any sense since job==after
        return;
    }

    if(!after)
    {
        //The job must be inserted in front of the list
        m_jobs.prepend(job);
        m_scheduler->jobQueueMovedJobEvent(this, job);
        return;
    }

    QList<Job *>::iterator it = m_jobs.find(after);
    if( it!=m_jobs.end() )
    {
        m_jobs.insert(++it, job);
        m_scheduler->jobQueueMovedJobEvent(this, job);
    }
}

int JobQueue::size() const
{
    return m_jobs.size();
}

