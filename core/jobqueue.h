/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef JOBQUEUE_H
#define JOBQUEUE_H

/**
 * @brief JobQueue class
 *
 * This class abstracts the concept of queue. A queue is, basically, a 
 * group of jobs that should be executed by the scheduler (if the queue
 * is marked as active). The scheduler will execute a maximum of n jobs
 * belonging to this queue at a time, where n can be set calling the 
 * setMaxSimultaneousJobs(int n)
 *
 */

#include <QList>
#include "../kget_export.h"

class Job;
class Scheduler;

class KGET_EXPORT JobQueue : public QObject
{
    Q_OBJECT
    public:
        enum Status {Running, Stopped};
        typedef QList<Job *>::iterator iterator;

        JobQueue(Scheduler * parent);
        virtual ~JobQueue();

        /**
         * Sets the JobQueue status
         *
         * @param queueStatus the new JobQueue status
         */
        virtual void setStatus(Status queueStatus);

        /**
         * @return the jobQueue status
         */
        Status status() const   {return m_status;}

        /**
         * @return the begin of the job's list
         */
        iterator begin()    {return m_jobs.begin();}

        /**
         * @return the end of the job's list
         */
        iterator end()      {return m_jobs.end();}

        /**
         * @return the last job in the job's list
         */
        Job * last()        {return m_jobs.last();}

        /**
         * @return the number of jobs in the queue
         */
        int size() const    {return m_jobs.size();}

        /**
         * @param job The job for which we want to find the index
         *
         * @return the job index for the given job. If the given
         *         job can't be found, it returns -1
         */
        int indexOf(Job * job) const    {return m_jobs.indexOf(job);}

        /**
         * @returns the Job in the queue at the given index i
         */
        Job * operator[] (int i) const  {return m_jobs[i];}

        /**
         * @return a list with the running Jobs
         */
        const QList<Job *> runningJobs();

        /**
         * FIXME not implemented
         * Sets the maximum number of jobs belonging to this queue that 
         * should executed simultaneously by the scheduler
         *
         * @param n The maximum number of jobs
         */
        void setMaxSimultaneousJobs(int n);

        /**
         * @return the maximum number of jobs the scheduler should ever
         * execute simultaneously (in this queue).
         */
        int maxSimultaneousJobs() const;

    protected:
        /**
         * appends a job to the current queue
         *
         * @param job The job to append to the current queue
         */
        void append(Job * job);

        /**
         * appends jobs to the current queue
         *
         * @param jobs to append to the current queue
         */
        void append(const QList<Job*> &jobs);

        /**
         * prepends a job to the current queue
         *
         * @param job The job to prepend to the current queue
         */
        void prepend(Job * job);

        /**
         * inserts a job to the current queue after the given job
         *
         * @param job The job to add in the current queue
         * @param after The job after which to add the job
         */
        void insert(Job * job, Job * after);

        /**
         * removes a job from the current queue
         *
         * @param job The job to remove from the current queue
         */
        void remove(Job * job);

        /**
         * removes jobs from the current queue
         *
         * @param jobs The jobs to remove from the current queue
         */
        void remove(const QList<Job*> jobs);

        /**
         * Moves a job in the queue. Both the given jobs must belong to this queue
         *
         * @param job The job to move
         * @param position The job after which we have to move the given job
         */
        void move(Job * job, Job * after);

        Scheduler * scheduler()     {return m_scheduler;}

    private:
        QList<Job *> m_jobs;

        int m_maxSimultaneousJobs;

        Scheduler * m_scheduler;
        Status m_status;
};

#endif
