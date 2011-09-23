/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Coypright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/scheduler.h"

#include "core/transferhandler.h"
#include "settings.h"

#include <algorithm>
#include <boost/bind.hpp>

#include <KDebug>

Scheduler::Scheduler(QObject * parent)
  : QObject(parent),
    m_failureCheckTimer(0),
    m_stallTime(5),
    m_stallTimeout(Settings::reconnectDelay()),
    m_abortTimeout(Settings::reconnectDelay()),
    m_isSuspended(false),
    m_hasConnection(true)
{

}

Scheduler::~Scheduler()
{

}

void Scheduler::setIsSuspended(bool isSuspended)
{
    const bool changed = (isSuspended != m_isSuspended);
    m_isSuspended = isSuspended;

    //update all the queues
    if (changed && shouldUpdate()) {
        updateAllQueues();
    }
}

void Scheduler::setHasNetworkConnection(bool hasConnection)
{
    const bool changed = (hasConnection != m_hasConnection);
    m_hasConnection = hasConnection;

    if (changed) {
        if (hasConnection) {
            if (!m_failureCheckTimer) {
                m_failureCheckTimer = startTimer(1000);
            }
            updateAllQueues();
        } else {
            if (m_failureCheckTimer) {
                killTimer(m_failureCheckTimer);
                m_failureCheckTimer = 0;
            }
            foreach (JobQueue *queue, m_queues) {
                std::for_each(queue->begin(), queue->end(), boost::bind(&Job::stop, _1));
            }
        }
    }
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

struct IsRunningJob
{
    bool operator()(Job *job) const {return (job->status() == Job::Running);}
};

bool Scheduler::hasRunningJobs() const
{
    foreach (JobQueue *queue, m_queues) {
        if (std::find_if(queue->begin(), queue->end(), IsRunningJob()) != queue->end()) {
            return true;
        }
    }
    return false;
}

int Scheduler::countRunningJobs() const
{
    int count = 0;
    foreach(JobQueue * queue, m_queues) {
        count += std::count_if(queue->begin(), queue->end(), IsRunningJob());
    }

    return count;
}

void Scheduler::settingsChanged()
{
    m_stallTimeout = Settings::reconnectDelay();
    m_abortTimeout = Settings::reconnectDelay();
    
    updateAllQueues();
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

void Scheduler::jobQueueAddedJobsEvent(JobQueue *queue, const QList<Job*> jobs)
{
    Q_UNUSED(jobs)

    updateQueue(queue);
}


void Scheduler::jobQueueRemovedJobEvent(JobQueue * queue, Job * job)
{
    Q_UNUSED(job)

    updateQueue(queue);
}

void Scheduler::jobQueueRemovedJobsEvent(JobQueue *queue, const QList<Job*> jobs)
{
    Q_UNUSED(jobs)

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
        case Error:
            kDebug(5001) << "job = " << job << " failure (#" << failure.count << ") = Error ";
            break;
    }
    
    if (failure.status == Error) {
        static_cast<Transfer*>(job)->handler()->stop();
    } else if (//If this happens the job just gets stopped
        // Second condition: if count >  reconnectRetries and Timeout happened trigger a stop/start BUT only if
        // 10 timeouts have happened (9 of them without taking any action). This means every 10*Settings::reconnectDelay() (ex. 15s -> 150s)
       (failure.count >  Settings::reconnectRetries() && (failure.status == StallTimeout || failure.status == AbortTimeout) 
                                                      && !((failure.count - Settings::reconnectRetries()) % 10)) )
    {
        //FIXME reenable once a connection limit per mirror is in place BUG:262098
        //static_cast<Transfer*>(job)->handler()->stop();// This will trigger the changedEvent which will trigger an updateQueue call
        job->stop();//FIXME remove once a connection limit per mirror is in place
    } else if (failure.count <= Settings::reconnectRetries() && (failure.status == StallTimeout || failure.status == AbortTimeout)){
        // First  condition: if count <= reconnectRetries and Timeout happened trigger a stop/start
        job->stop();//stops the job, it will be later restarted by updateQueue
    }
    else
        updateQueue( job->jobQueue() );  
}

void Scheduler::start()
{
    std::for_each(m_queues.begin(), m_queues.end(), boost::bind(&JobQueue::setStatus, _1, JobQueue::Running));
}

void Scheduler::stop()
{
    std::for_each(m_queues.begin(), m_queues.end(), boost::bind(&JobQueue::setStatus, _1, JobQueue::Stopped));
}

void Scheduler::updateQueue( JobQueue * queue )
{
    static bool updatingQueue = false;
    
    if (!shouldUpdate() || updatingQueue)
        return;

    updatingQueue = true;
       
    int runningJobs = 0;    //Jobs that are running (and not in the stallTimeout)
    int waitingJobs = 0;    //Jobs that we leave running but are in stallTimeout. We wait for them to start downloading, while we start other ones

    /**
     * Implemented behaviour
     * 
     * The scheduler allows a maximum number of runningJobs equal to the queue->maxSimultaneousJobs() setting.
     * If that number is not reached because of stallTimeout transfers, the scheduler allows that:
     *     (runningJobs + waitingJobs) < 2 * queue->maxSimultaneousJobs()
     * Examples (with maxSimultaneousJobs = 2):
     *        These are if the running jobs come first in the queue
     *     1) 2 runningJobs - 0 waitingJobs
     *     2) 1 runningJobs - up to 3 waitingJobs
     *     3) 0 runningJobs - up to 4 waitingJobs
     *        These are if the waiting jobs come first in the queue
     *     1) 1 waitingJobs - 2 runningJobs
     *     2) 2 waitingJobs - 2 runningJobs     
     *     3) 3 waitingJobs - 1 runningJobs
     *     4) 4 waitingJobs - 0 runningJobs
     **/

    JobQueue::iterator it = queue->begin();
    JobQueue::iterator itEnd = queue->end();

    for( int job=0 ; it!=itEnd ; ++it, ++job)
    {
        //kDebug(5001) << "MaxSimJobs " << queue->maxSimultaneousJobs();
        kDebug(5001) << "Scheduler: Evaluating job " << job;
        
        JobFailure failure = m_failedJobs.value(*it);
        
        if( runningJobs < queue->maxSimultaneousJobs() && ((runningJobs + waitingJobs) < 2 * queue->maxSimultaneousJobs()) )
        {
            if( (*it)->status() == Job::Running || (*it)->status() == Job::FinishedKeepAlive )
            {
                if( !shouldBeRunning(*it) )
                {
                    kDebug(5001) << "Scheduler:    stopping job";
                    (*it)->stop();
                }
                else if(failure.status == None || failure.status == AboutToStall)
                    runningJobs++;
                else
                    waitingJobs++;
            }
            else             // != Job::Running
            {
                if( shouldBeRunning(*it) )
                {
                    kDebug(5001) << "Scheduler:    starting job";
                    (*it)->start();
                    if((failure.status == None || failure.status == AboutToStall) && (*it)->status() != Job::FinishedKeepAlive)
                        runningJobs++;
                    else
                        waitingJobs++;
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

void Scheduler::updateAllQueues()
{
    foreach (JobQueue *queue, m_queues) {
        updateQueue(queue);
    }
}

bool Scheduler::shouldBeRunning( Job * job )
{
    Job::Policy policy = job->policy();
    Job::Status status = job->status();

    if( job->jobQueue()->status() == JobQueue::Stopped )
    {
        return ( (policy == Job::Start)   &&
                 ((status != Job::Finished) &&
                 (status != Job::Aborted || job->error().type == Job::AutomaticRetry)));
    }
    else                           //JobQueue::Running
    {
        return ( (policy != Job::Stop)    &&
                 ((status != Job::Finished) &&
                 (status != Job::Aborted || job->error().type == Job::AutomaticRetry)));
    }
}

void Scheduler::timerEvent( QTimerEvent * event )
{
    Q_UNUSED(event)
//     kDebug(5001);

    if (!shouldUpdate()) {
        return;
    }

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
                if ((*it)->error().type != Job::AutomaticRetry) {
                    failure.status = Error;
                } else {
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
