/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/scheduler.h"

#include "core/kget.h"
#include "core/job.h"
#include "core/jobqueue.h"
#include "settings.h"

#include <kdebug.h>

Scheduler::Scheduler(QObject * parent)
  : QObject(parent),
    m_failureCheckTimer(0),
    m_stallTime(5),
    m_stallTimeout(Settings::reconnectDelay()),
    m_abortTimeout(Settings::reconnectDelay()) 
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
    m_queues.removeAll(queue);
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

void Scheduler::settingsChanged()
{
    m_stallTimeout = Settings::reconnectDelay();
    m_abortTimeout = Settings::reconnectDelay();
    
    foreach(JobQueue * queue, m_queues)
    {
        updateQueue(queue);
    }
}

void Scheduler::jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status)
{
    if( status == JobQueue::Stopped )
    {
        JobQueue::iterator it = queue->begin();
        JobQueue::iterator itEnd = queue->end();

        for ( ; it!=itEnd ; ++it)
        {
            if ((*it)->status() != Job::Stopped)
                (*it)->stop();
        }
    }
    else
        updateQueue(queue);
}

void Scheduler::jobQueueMovedJobEvent(JobQueue * queue, Job * job)
{
    Q_UNUSED(job)

    updateQueue(queue);
}

void Scheduler::jobQueueAddedJobEvent(JobQueue * queue, Job * job)
{
    Q_UNUSED(job)

    updateQueue(queue);
}

void Scheduler::jobQueueRemovedJobEvent(JobQueue * queue, Job * job)
{
    Q_UNUSED(job)

    updateQueue(queue);
}

void Scheduler::jobChangedEvent(Job * job, Job::Status status)
{
    kDebug(5001) << "Scheduler::jobChangedEvent  (job=" << job << " status=" << status <<  ")";

    if (!m_failureCheckTimer)
        m_failureCheckTimer = startTimer(1000);

    if (status != Job::Running)
        updateQueue( job->jobQueue() );
}

void Scheduler::jobChangedEvent(Job * job, Job::Policy policy)
{
    Q_UNUSED(policy)

    updateQueue( job->jobQueue() );
}

void Scheduler::jobChangedEvent(Job * job, JobFailure failure)
{
    switch(failure.status)
    {
        case None:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = None ";
            break;
        case AboutToStall:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = AboutToStall ";
            break;
        case Stall:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = Stall ";
            break;
        case StallTimeout:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = StallTimeout ";
            break;
        case Abort:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = Abort ";
            break;
        case AbortTimeout:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = AbortTimeout ";
            break;
    }
    
    if( // First  condition: if count <= reconnectRetries and Timeout happened trigger a stop/start
       (failure.count <= Settings::reconnectRetries() && (failure.status == StallTimeout || failure.status == AbortTimeout)) ||
        // Second condition: if count >  reconnectRetries and Timeout happened trigger a stop/start BUT only if
        // 10 timeouts have happened (9 of them without taking any action). This means every 10*Settings::reconnectDelay() (ex. 15s -> 150s)
       (failure.count >  Settings::reconnectRetries() && (failure.status == StallTimeout || failure.status == AbortTimeout) 
                                                      && !((failure.count - Settings::reconnectRetries()) % 10)) )
    {
        job->stop();        // This will trigger the changedEvent which will trigger an updateQueue call
    }
    else
        updateQueue( job->jobQueue() );  
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
    static bool updatingQueue = false;
    
    if(updatingQueue)
        return;

    updatingQueue = true;
       
    int runningJobs = 0;

    JobQueue::iterator it = queue->begin();
    JobQueue::iterator itEnd = queue->end();

    for( int job=0 ; it!=itEnd ; ++it, ++job)
    {
        //kDebug(5001) << "MaxSimJobs " << queue->maxSimultaneousJobs();
        kDebug(5001) << "Scheduler: Evaluating job " << job;
        
        JobFailure failure = m_failedJobs.value(*it);
        
        if( runningJobs < queue->maxSimultaneousJobs() )
        {
            if( (*it)->status() == Job::Running )
            {
                if( !shouldBeRunning(*it) )
                {
                    kDebug(5001) << "Scheduler:    stopping job";
                    (*it)->stop();
                }
                else if(failure.status == None || failure.status == AboutToStall)
                    runningJobs++;
            }
            else             // != Job::Running
            {
                if( shouldBeRunning(*it) )
                {
                    kDebug(5001) << "Scheduler:    starting job";
                    (*it)->start();
                    if(failure.status == None || failure.status == AboutToStall)
                        runningJobs++;
                }
            }
        }
        else
        {
            //Stop all the other running downloads
            kDebug(5001) << "Scheduler:    stopping job over maxSimJobs limit";
            (*it)->stop();
        }
    }
    
    updatingQueue = false;
}

bool Scheduler::shouldBeRunning( Job * job )
{
    Job::Policy policy = job->policy();
    Job::Status status = job->status();

    if( job->jobQueue()->status() == JobQueue::Stopped )
    {
        return ( (policy == Job::Start)   &&
                 ((status != Job::Finished) || job->isResumable()) );
    }
    else                           //JobQueue::Running
    {
        return ( (policy != Job::Stop)    &&
                 ((status != Job::Finished) || job->isResumable()) );
    }
}

void Scheduler::timerEvent( QTimerEvent * event )
{
    Q_UNUSED(event)
//     kDebug(5001);
    
    foreach(JobQueue * queue, m_queues)
    {
        JobQueue::iterator it = queue->begin();
        JobQueue::iterator itEnd = queue->end();

        for( int job=0 ; it!=itEnd ; ++it, ++job)
        {
            JobFailure failure = m_failedJobs[*it];
            JobFailure prevFailure = failure;
            
            if((*it)->isStalled())                              // Stall status initialization
            {
                if(failure.status!=AboutToStall && failure.status!=Stall && failure.status!=StallTimeout)
                {
                    failure.status = AboutToStall;
                    failure.time = 0;
                    failure.count = 0;                    
                }
                else
                {
                    failure.time++;
                    
                    if(failure.time >= m_stallTime + m_stallTimeout)
                    {
                        failure.status = StallTimeout;
                        failure.count++;                    

                    }
                    else if(failure.time >= m_stallTime)
                        failure.status = Stall;
                    else
                        failure.status = AboutToStall;
                    
                    if(failure.status == StallTimeout)
                        failure.time = m_stallTime;
                }
            } 
            else if((*it)->status() == Job::Aborted)            // Abort status initialization
            {
                if(failure.status!=Abort)
                {
                    failure.status = Abort;
                    failure.time = 0;
                    failure.count = 0;
                }
                else
                {
                    failure.time++;
                    failure.count++;                    
                    
                    if(failure.time >= m_abortTimeout)
                    {
                        failure.status = AbortTimeout;
                        failure.count++;                    
                    }
                    
                    if(failure.status == AbortTimeout)
                        failure.time = 0;
                }
            }
            else if ((*it)->isWorking())
            {
                failure = JobFailure();
            }
               
            if(failure.isValid())                                   // A failure has been detected
                m_failedJobs[*it] = failure;
            else                                                    // No failure detected, remove it
                m_failedJobs.remove(*it);
                           
//             if(failure.isValid() || prevFailure.isValid())
//                 kDebug(5001) << "failure = " << failure.status << " T=" << failure.time << " prevFailure = " << prevFailure.status;
            
            if(failure.status != prevFailure.status)
                jobChangedEvent(*it, failure);                      // Notify the scheduler
        }
    }
}

#include "scheduler.moc"
