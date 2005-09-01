/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QList>
#include <QTimerEvent>

#include <kdebug.h>

#include "scheduler.h"
#include "model.h"
#include "job.h"
#include "jobqueue.h"

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
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

int Scheduler::countRunningJobs()
{
    int count = 0;

    foreach(JobQueue * queue, m_queues)
    {
        JobQueue::iterator it = queue->begin();
        JobQueue::iterator itEnd = queue->end();

        for( ; it!=itEnd ; ++it )
        {
            if((*it)->status() == Job::Running)
                count++;
        }
    }

    return count;
}

void Scheduler::jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status)
{
    if( status == JobQueue::Stopped )
    {
        JobQueue::iterator it = queue->begin();
        JobQueue::iterator itEnd = queue->end();

        for( ; it!=itEnd ; ++it)
        {
            if((*it)->status() != Job::Stopped)
                (*it)->stop();
        }
    }
    else
        updateQueue(queue);
}

void Scheduler::jobQueueMovedJobEvent(JobQueue * queue, Job * job)
{
    updateQueue(queue);
}

void Scheduler::jobQueueAddedJobEvent(JobQueue * queue, Job * job)
{

}

void Scheduler::jobQueueRemovedJobEvent(JobQueue * queue, Job * job)
{

}

void Scheduler::jobChangedEvent(Job * job, Job::Status status)
{
    kdDebug() << "Scheduler::jobChangedEvent (" << status <<  ")" << endl;

    //If the Job changed its status to Aborted, set a delay.
    if (status == Job::Aborted)
    {
        job->setDelay(10);
        //Here it's not necessary to call updateQueue since the setDelay()
        //function will generate another jobChangedEvent. We will call 
        //updateQueue then.
        return;
    }

    if (status != Job::Running)
        updateQueue( job->jobQueue() );
}

void Scheduler::jobChangedEvent(Job * job, Job::Policy policy)
{
    updateQueue( job->jobQueue() );
}

void Scheduler::startDelayTimer(Job * job, int seconds)
{
    stopDelayTimer(job);

    int index = startTimer(seconds * 1000);
    if(index == 0)
        return;
    m_activeTimers[index] = job;
}

void Scheduler::stopDelayTimer(Job * job)
{
    QMap<int, Job *>::iterator it = m_activeTimers.begin();
    QMap<int, Job *>::iterator itEnd = m_activeTimers.end();

    for( ; it!=itEnd ; ++it )
    {
        if(it.data() == job)
        {
            //A timer for this job has been found. Let's stop it.
            killTimer(it.key());
            m_activeTimers.remove(it);
        }
    }
}

void Scheduler::start()
{
    QList<JobQueue *>::iterator it = m_queues.begin();
    QList<JobQueue *>::iterator itEnd = m_queues.end();

    for( ; it!=itEnd ; ++it )
    {
        (*it)->setStatus(JobQueue::Running);
    }
}

void Scheduler::stop()
{
    QList<JobQueue *>::iterator it = m_queues.begin();
    QList<JobQueue *>::iterator itEnd = m_queues.end();

    for( ; it!=itEnd ; ++it )
    {
        (*it)->setStatus(JobQueue::Stopped);
    }
}

void Scheduler::updateQueue( JobQueue * queue )
{
    int runningJobs = 0;

    JobQueue::iterator it = queue->begin();
    JobQueue::iterator itEnd = queue->end();

    for( int job=0 ; it!=itEnd ; ++it, ++job)
    {
        //kdDebug() << "MaxSimJobs " << queue->maxSimultaneousJobs() << endl;
        kdDebug() << "Scheduler: Evaluating job " << job << endl;
        if( runningJobs < queue->maxSimultaneousJobs() )
        {
            if( (*it)->status() == Job::Running )
            {
                if( !shouldBeRunning(*it) )
                {
                    kdDebug() << "Scheduler:    stopping job" << endl;
                    (*it)->stop();
                }
                else
                    runningJobs++;
            }
            else             // != Job::Running
            {
                if( shouldBeRunning(*it) )
                {
                    kdDebug() << "Scheduler:    starting job" << endl;
                    (*it)->start();
                    runningJobs++;
                }
                else if( ((*it)->status() == Job::Delayed )
                      && ((*it)->policy() == Job::Stop ) )
                {
                    kdDebug() << "Scheduler:     Delayed transfer that should be stopped" << endl;
                    //This is a special case that we have to handle separately:
                    //if the download status is Delayed, but the current policy
                    //is Stopped, we must stop immediately the transfer.
                    stopDelayTimer(*it);
                    (*it)->stop();
                }
            }
        }
        else
        {
            //Stop all the other running downloads
            kdDebug() << "Scheduler:    stopping job over maxSimJobs limit" << endl;
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

void Scheduler::timerEvent( QTimerEvent * event )
{
    Job * job = m_activeTimers[event->timerId()];
    stopDelayTimer( job );

    job->delayTimerEvent();
}

#include "scheduler.moc"
