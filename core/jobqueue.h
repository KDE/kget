/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _JOBQUEUE_H
#define _JOBQUEUE_H

/**
 * @brief Class JobQueue:
 *
 * This class abstracts the concept of queue. A queue is, basically, a 
 * group of jobs that should be executed by the scheduler (if the queue
 * is marked as active). The scheduler will execute a maximum of n jobs
 * belonging to this queue at a time, where n can be set calling the 
 * setMaxSimultaneousJobs(int n)
 *
 */

#include <qvaluelist.h>

class Job;

class JobQueue
{
    public:
        JobQueue() {}

        /**
         * Sets the maximum number of jobs belonging to this queue that 
         * should executed simultaneously by the scheduler
         *
         * @param n The maximum number of jobs
         */
        void setMaxSimultaneousJobs(int n);

        /**
         * Returns a const reference to the list of jobs belonging to the queue
         */
        const QValueList<Job *> & jobs() const;

    private:
        int m_maxSimultaneousJobs;

        QValueList<Job *> m_jobs;
};

#endif
