/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "core/jobqueue.h"

#include "core/scheduler.h"
#include "settings.h"

#include <kdebug.h>

JobQueue::JobQueue(Scheduler * parent)
    : QObject(parent),
      m_maxSimultaneousJobs(2),
      m_scheduler(parent),
      m_status(Running)
{
}

JobQueue::~JobQueue()
{
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
    
    // Now make sure to reset all the job policy that shouldn't
    // be applied anymore.
    iterator it = begin();
    iterator itEnd = end();
    
    for( ; it!=itEnd  ; ++it )
    {
	if( ( m_status == JobQueue::Running ) &&
	    ( (*it)->status() == Job::Running ) )
	{
	    (*it)->setPolicy(Job::None);
	}
	
	if( ( m_status == JobQueue::Stopped ) &&
	    ( (*it)->status() == Job::Stopped ) )
	{
	    (*it)->setPolicy(Job::None);
	}
    }
    
    m_scheduler->jobQueueChangedEvent(this, m_status);
}

int JobQueue::maxSimultaneousJobs() const
{
    const int maxConnections = Settings::maxConnections();
    return (maxConnections ? maxConnections : 1000);// High value just to indicate no limit
}

void JobQueue::append(Job * job)
{
    m_jobs.append(job);

    m_scheduler->jobQueueAddedJobEvent(this, job);
}

void JobQueue::append(const QList<Job*> &jobs)
{
    m_jobs.append(jobs);

    m_scheduler->jobQueueAddedJobsEvent(this, jobs);
}

void JobQueue::prepend(Job * job)
{
    m_jobs.prepend(job);

    m_scheduler->jobQueueAddedJobEvent(this, job);
}

void JobQueue::insert(Job * job, Job * after)
{
    if((job->jobQueue() == this) || ((after) && (after->jobQueue() != this)))
        return;

    m_jobs.insert(m_jobs.indexOf(after) +1, job);
    m_scheduler->jobQueueAddedJobEvent(this, job);
}

void JobQueue::remove(Job * job)
{
    m_jobs.removeAll(job);

    m_scheduler->jobQueueRemovedJobEvent(this, job);
}

void JobQueue::remove(const QList<Job*> jobs)
{
    foreach (Job *job, jobs) {
        m_jobs.removeAll(job);
    }

    m_scheduler->jobQueueRemovedJobsEvent(this, jobs);
}

void JobQueue::move(Job * job, Job * after)
{
    kDebug(5001) << "JobQueue::move";

    if( (m_jobs.removeAll(job) == 0) || (job == after)  ||
        ((after) && (after->jobQueue() != this)) )
    {
        //The job doesn't belong to this JobQueue or the requested
        //operations doesn't make any sense since job==after
        return;
    }

    if(!after)
    {
        //The job must be inserted in front of the list
        m_jobs.prepend(job);
    }
    else
    {
        m_jobs.insert(m_jobs.indexOf(after) + 1, job);
    }

    m_scheduler->jobQueueMovedJobEvent(this, job);
}


