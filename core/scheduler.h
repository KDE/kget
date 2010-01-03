/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QMap>
#include <QTimerEvent>

#include "core/job.h"
#include "core/jobqueue.h"

/**
 * @brief Scheduler class: what handle all the jobs in kget.
 *
 * This class handles all the jobs in kget. See job.h for further details.
 * When we want a job to be executed in kget, we have to add the queue
 * that owns the job in the scheduler calling the addQueue(JobQueue *) function.
 *
 */

class Scheduler : public QObject
{
    Q_OBJECT
    public:

        enum FailureStatus {
            None         = 0,
            AboutToStall = 1,
            Stall        = 2,
            StallTimeout = 3,
            Abort        = 4,
            AbortTimeout = 5
        };
        
        class JobFailure {
            public:
            JobFailure()
                : status(None), time(-1), count(0)
            {}
            
            bool isValid()                  {return ((status != None) && (time != -1));}
            
            FailureStatus status;
            int time;
            int count;
            
            bool operator==(JobFailure f)   {return ((status == f.status) && (time == f.time));}
            bool operator!=(JobFailure f)   {return ((status != f.status) || (time != f.time));}            
        };
        
        Scheduler(QObject * parent = 0);
        ~Scheduler();

        /**
         * Starts globally the execution of the jobs
         *
         * @see stop()
         */
        void start();

        /**
         * Stops globally the execution of the jobs
         *
         * @see start()
         */
        void stop();

        /**
         * Adds a queue to the scheduler.
         *
         * @param queue The queue that should be added
         */
        void addQueue(JobQueue * queue);

        /**
         * Deletes a queue from the scheduler.
         * If some jobs in the given queue are being executed, they are
         * first stopped, then removed from the scheduler.
         *
         * @param queue The queue that should be removed
         */
        void delQueue(JobQueue * queue);

        /**
         * @returns the number of jobs that are currently in a Running state
         */
        int countRunningJobs();
        
        /**
         * This function gets called by the KGet class whenever the settings
         * have changed.
         */
        void settingsChanged();

        //JobQueue notifications
        virtual void jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status);
        virtual void jobQueueMovedJobEvent(JobQueue * queue, Job * job);
        virtual void jobQueueAddedJobEvent(JobQueue * queue, Job * job);
        virtual void jobQueueRemovedJobEvent(JobQueue * queue, Job * job);

        //Job notifications
        virtual void jobChangedEvent(Job * job, Job::Status status);
        virtual void jobChangedEvent(Job * job, Job::Policy status);
        virtual void jobChangedEvent(Job * job, JobFailure failure);

    protected:
        /**
         * Updates the given queue, starting the jobs that come first in the queue
         * and stopping all the other
         *
         * @param queue the queue to update
         */
        void updateQueue( JobQueue * queue );

        /**
         * @return true if the given job should be running (and this depends
         * on the job policy and on its jobQueue status)
         *
         * @param job the job to evaluate
         */
        bool shouldBeRunning( Job * job );
        
    private:
        //Virtual QObject method
        void timerEvent(QTimerEvent * event);

        QList<JobQueue *> m_queues;
        QMap<Job *, JobFailure> m_failedJobs;

        int m_failureCheckTimer;
        
        const int m_stallTime;
        int m_stallTimeout;
        int m_abortTimeout;
};

#endif
