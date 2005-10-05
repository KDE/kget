/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _JOB_H
#define _JOB_H

/**
 *  @brief Job class
 *
 *  We want to abstract this common interface in order to simplify the 
 *  Scheduler code. A Job can be either a Transfer or a search through the net.
 *  It is basically something you execute in background and that the scheduler 
 *  can decide to start, stop or cancel. In this way we don't expose the complex 
 *  API of a Transfer (or a Search), to the scheduler.
 *  By definition a job must always belong to a JobQueue (see jobqueue.h).
 **/

class QDomNode;

class Scheduler;
class JobQueue;

class Job
{
    public:
        /**
         * The status property describes the current job status
         *
         * @param Running The job is being executed
         * @param Delayed The job is delayed. This means that the scheduler should
         *                not start it until it exits from the delayed state
         * @param Stopped The job is stopped
         * @param Aborted The job is stopped, but this also indicates that it
         *                stopped becouse an error occoured
         * @param Finished The job exited from its Running state successfully
         */
        enum Status {Running, Delayed, Stopped, Aborted, Finished};

        /**
         * The policy property describes how the scheduler should manage this job.
         *
         * @param Start The scheduler should start this job even if its queue 
         *              isn't in a Running status
         * @param Stop The scheduler shouldn't never start this job, even if
         *             if its queue is in a Running status
         * @param None The scheduler should start this job depending on its
         *             queue status
         */
        enum Policy {Start, Stop, None};

        Job(JobQueue * parent, Scheduler * scheduler);
        virtual ~Job();

        //Job commands
        virtual void start()=0;
        virtual void stop()=0;

        virtual void setDelay(int seconds)=0;
        virtual void delayTimerEvent()=0;

        JobQueue * jobQueue() {return m_jobQueue;}

        //Job properties
        void setStatus(Status jobStatus);
        void setPolicy(Policy jobPolicy);

        Status status() const {return m_status;}
        Policy policy() const {return m_policy;}

        virtual int elapsedTime() const =0;
        virtual int remainingTime() const =0;

        //Job capabilities
        virtual bool isResumable() const =0;

    protected:
        Scheduler * scheduler() const {return m_scheduler;}

        void read(QDomNode * n);
        void write(QDomNode * n);

        /**
         * This one posts a job event to the scheduler
         */
        void postJobEvent(Status); //do we need a JobEvent instead of JobStatus?

        JobQueue *  m_jobQueue;
        Scheduler * m_scheduler;

    private:
        Status m_status;
        Policy m_policy;
};

#endif
