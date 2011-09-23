/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Coypright (C) 2010 Matthias Fuchs <mat69@gmx.net>

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
#include "../kget_export.h"

/**
 * @brief Scheduler class: what handle all the jobs in kget.
 *
 * This class handles all the jobs in kget. See job.h for further details.
 * When we want a job to be executed in kget, we have to add the queue
 * that owns the job in the scheduler calling the addQueue(JobQueue *) function.
 *
 */

class KGET_EXPORT Scheduler : public QObject
{
    Q_OBJECT

    friend class SchedulerTest;

    public:

        enum FailureStatus {
            None         = 0,
            AboutToStall = 1,
            Stall        = 2,
            StallTimeout = 3,
            Abort        = 4,
            AbortTimeout = 5,
            Error = 6
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
            
            bool operator==(JobFailure f) const {return ((status == f.status) && (time == f.time));}
            bool operator!=(JobFailure f) const {return ((status != f.status) || (time != f.time));}            
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
         * Can be used to suspend the scheduler before doing lenghty operations
         * and activating it later again
         *
         * NOTE does not stop running jobs, just prevents changes to jobs
         * HACK this is needed since the scheduler would constantly update the queue
         * when stopping starting multiple transfers, this slows down that operation a lot
         * and could result in transfers finishing before they are stopped etc.
         */
        void setIsSuspended(bool isSuspended);

        /**
         * The JobQueues will be informed of changes in the network connection
         * If there is no network connection then the Scheduler won't act on
         * the timerEvent or updateQueue
         */
        void setHasNetworkConnection(bool hasConnection);

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
         * @returns true if there is at least one Job in the Running state
         */
        bool hasRunningJobs() const;

        /**
         * @returns the number of jobs that are currently in a Running state
         */
        int countRunningJobs() const;
        
        /**
         * This function gets called by the KGet class whenever the settings
         * have changed.
         */
        void settingsChanged();

        //JobQueue notifications
        virtual void jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status);
        virtual void jobQueueMovedJobEvent(JobQueue * queue, Job * job);
        virtual void jobQueueAddedJobEvent(JobQueue * queue, Job * job);
        virtual void jobQueueAddedJobsEvent(JobQueue *queue, const QList<Job*> jobs);
        virtual void jobQueueRemovedJobEvent(JobQueue * queue, Job * job);
        virtual void jobQueueRemovedJobsEvent(JobQueue *queue, const QList<Job*> jobs);

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

        /**
         * Calls updateQueue for all queues
         * @see updateQueue
         */
        void updateAllQueues();

        bool shouldUpdate() const;

    private:
        QList<JobQueue *> m_queues;
        QMap<Job *, JobFailure> m_failedJobs;

        int m_failureCheckTimer;

        const int m_stallTime;
        int m_stallTimeout;
        int m_abortTimeout;
        bool m_isSuspended;
        bool m_hasConnection;
};

inline bool Scheduler::shouldUpdate() const
{
    return !m_isSuspended && m_hasConnection;
}
#endif
