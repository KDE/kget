/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qvaluelist.h>

#include <kdebug.h>

#include "scheduler.h"
#include "job.h"
#include "jobqueue.h"

Scheduler::Scheduler()
{
    
}

void Scheduler::run()
{
    
}

void Scheduler::stop()
{
    
}

void Scheduler::addQueue(JobQueue * queue)
{
    if(!m_queues.contains(queue))
        m_queues.append(queue);
}

void Scheduler::delQueue(JobQueue * queue)
{
    m_queues.remove(queue);
}

void Scheduler::jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status)
{

}

void Scheduler::jobQueueMovedJobEvent(JobQueue * queue, Job * job)
{

}

void Scheduler::jobQueueAddedJobEvent(JobQueue * queue, Job * job)
{

}

void Scheduler::jobQueueRemovedJobEvent(JobQueue * queue, Job * job)
{

}

void Scheduler::jobChangedEvent(Job * job, Job::Status status)
{
    updateQueue( job->jobQueue() );
}

void Scheduler::jobChangedEvent(Job * job, Job::Policy policy)
{
    updateQueue( job->jobQueue() );
}

void Scheduler::updateQueue( JobQueue * queue )
{
    int jobs = 0;

    JobQueue::iterator it = queue->begin();
    JobQueue::iterator itEnd = queue->end();

    for( ; it!=itEnd ; ++it )
    {
        if( jobs <= queue->maxSimultaneousJobs() )
        {
            if( (*it)->status() == Job::Running )
            {
                if( !shouldBeRunning(*it) )
                {
                    (*it)->stop();
                }
            }
            else             // != Job::Running
            {
                if( shouldBeRunning(*it) )
                {
                    (*it)->start();
                    jobs++;
                }
            }
        }
        else
        {
            //Stop all the other running downloads
            (*it)->stop();
        }
    }
}

bool Scheduler::shouldBeRunning( Job * job )
{
    Job::Policy policy = job->policy();
    Job::Status status = job->status();

    if( job->jobQueue()->status() == JobQueue::Stopped )
    {
        return ( (policy == Job::Start)   &&
                 (status != Job::Delayed) &&
                 (status != Job::Finished) );
    }
    else                           //JobQueue::Running
    {
        return ( (policy != Job::Stop)    &&
                 (status != Job::Delayed) &&
                 (status != Job::Finished) );
    }
}
