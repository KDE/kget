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

Job * JobQueue::operator[] (int i) const
{
    return m_jobs[i];
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

void JobQueue::append(Job * job)
{
    m_jobs.append(job);

    m_scheduler->jobQueueAddedJobEvent(this, job);    
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

void JobQueue::move(Job * job, Job * after)
{
    kDebug(5001) << "JobQueue::move" << endl;

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
        m_jobs.insert(m_jobs.indexOf(after) +1, job);
    }

    m_scheduler->jobQueueMovedJobEvent(this, job);
}


