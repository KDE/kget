/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

class JobQueue;
class Job;

/**
 * class Scheduler:
 *
 * This class handles all the jobs in kget. See job.h for further details.
 * When we want a job to be executed in kget, we have to add the queue
 * that owns the job in the scheduler calling the addQueue(JobQueue *) function.
 *
 */

class Scheduler
{
    public:
        Scheduler();

        /**
         * Starts globally the execution of the jobs
         */
        void run();

        /**
         * Stops globally the execution of the jobs
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

    private:
        QValueList<JobQueue *> m_queues;
};

#endif
