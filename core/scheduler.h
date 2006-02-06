/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
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
        Scheduler();
        ~Scheduler();

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

        //JobQueue notifications
        void jobQueueChangedEvent(JobQueue * queue, JobQueue::Status status);
        void jobQueueMovedJobEvent(JobQueue * queue, Job * job);
        void jobQueueAddedJobEvent(JobQueue * queue, Job * job);
        void jobQueueRemovedJobEvent(JobQueue * queue, Job * job);

        //Job notifications
        void jobChangedEvent(Job * job, Job::Status status);
        void jobChangedEvent(Job * job, Job::Policy status);

        //Accessors methods
        void startDelayTimer(Job * job, int seconds);
        void stopDelayTimer(Job * job);

    public slots:
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

    private:
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

        //Virtual QObject method
        void timerEvent ( QTimerEvent * event);

        QList<JobQueue *> m_queues;
        QMap<int, Job *> m_activeTimers;
};

#endif
