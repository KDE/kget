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
 *  -- Job class -- 
 *  We want to abstract this common interface in order to simplify the 
 *  Scheduler code. A Job can be either a Transfer or a search through the net.
 *  It is basically something you execute in background and that the scheduler 
 *  can decide to start, stop or cancel. In this way we don't expose the complex 
 *  API of a Transfer (or a Search), to the scheduler.
 *  By definition a job must always belong to a JobQueue (see jobqueue.h).
 **/

class Scheduler;
class QDomNode;

class Job
{
    public:
        enum JobStatus {Running, Delayed, Stopped, Aborted, Finished};

        Job(Scheduler * scheduler) {};

        //Job commands
        virtual void start()=0;
        virtual void stop()=0;
        void setDelay(int seconds);

        //Job status
        JobStatus jobStatus() const {return m_jobStatus;}
        virtual int elapsedTime() const =0;
        virtual int remainingTime() const =0;

        //Job capabilities
        virtual bool isResumable() const =0;

    protected:
        void read(QDomNode * n);
        void write(QDomNode * n);

        //This one posts a job event to the scheduler
        void postJobEvent(JobStatus); //do we need a JobEvent instead of JobStatus?

        Scheduler * m_scheduler;

        JobStatus m_jobStatus;
};

#endif
