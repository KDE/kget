/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef JOB_H
#define JOB_H

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

#include "../kget_export.h"

#include <QPixmap>

class QDomNode;

class Scheduler;
class JobQueue;

class KGET_EXPORT Job : public QObject
{
    Q_OBJECT
    public:
        /**
         * The status property describes the current job status
         */
        enum Status {
            Running            = 0, /// The job is being executed
            Stopped            = 2, /// The job is stopped
            Aborted            = 3, /// The job is stopped, but this also indicates that it
                                    /// stopped because an error occurred
            Finished           = 4, /// The job exited from its Running state successfully
            FinishedKeepAlive  = 5, /// The job exited from its Running state successfully
                                    /// but wants to be restarted by the scheduler
            Moving             = 6  /// The associated files to that job (e.g. Download) are
                                    /// moved to a different location
        };

        /**
         * The policy property describes how the scheduler should manage this job.
         */
        enum Policy {
            Start, /// The scheduler should start this job even if its queue
                   /// isn't in a Running status
            Stop,  /// The scheduler shouldn't never start this job, even if
                   /// if its queue is in a Running status
            None   /// The scheduler should start this job depending on its
                   /// queue status
        };
        /**
         * Describes different types of errors and how the scheduler should manage them.
         */
        enum ErrorType {
            AutomaticRetry,
            ManualSolve,
            NotSolveable
        };
        struct Error {
            int id;
            QString text;
            QPixmap pixmap;
            ErrorType type;
        };
        Job(Scheduler * scheduler, JobQueue * parent);
        virtual ~Job();

        //Job commands
        virtual void start()=0;
        virtual void stop()=0;

        JobQueue * jobQueue() {return m_jobQueue;}

        //Job properties
        void setStatus(Status jobStatus);
        void setPolicy(Policy jobPolicy);
        void setError(const QString &text, const QPixmap &pixmap, ErrorType type = AutomaticRetry, int errorId = -1);

        Status status() const {return m_status;}
        Status startStatus() const { return m_startStatus;}
        Policy policy() const {return m_policy;}
        Error error() const {return m_error;}

        virtual int elapsedTime() const =0;
        virtual int remainingTime() const =0;
        virtual bool isStalled() const =0;
        virtual bool isWorking() const =0;
        
        virtual void resolveError(int errorId);

    protected:
        Scheduler * scheduler() const {return m_scheduler;}

        void read(QDomNode * n);
        void write(QDomNode * n);

        void setStartStatus(Status jobStatus);

        /**
         * This one posts a job event to the scheduler
         */
        void postJobEvent(Status); //do we need a JobEvent instead of JobStatus?

        JobQueue *  m_jobQueue;
        Scheduler * m_scheduler;

    private:
        Status m_status;
        // our status when KGet is started
        Status m_startStatus;
        Policy m_policy;
        Error m_error;
};

#endif
