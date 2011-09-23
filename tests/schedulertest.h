/**************************************************************************
*   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#ifndef KGET_SCHEDULER_TEST_H
#define KGET_SCHEDULER_TEST_H

#include "../core/job.h"
#include "../core/jobqueue.h"

class Scheduler;

/**
 * Makes sure settings are changed on construction and then restored 
 * once the destructor is called
 */
class SettingsHelper //TODO better name
{
    public:
        SettingsHelper(bool useLimit, int limit);
        ~SettingsHelper();

    private:
        bool m_oldUseLimit;
        int m_oldLimit;
};

/**
 * There to make it easy to create jobs with which to fill the test schedule queue
 */
class TestJob : public Job
{
    Q_OBJECT

    public:
        TestJob(Scheduler *scheduler, JobQueue *parent);

        virtual void start();
        virtual void stop();
        virtual int elapsedTime() const;
        virtual int remainingTime() const;
        virtual bool isStalled() const;
        virtual bool isWorking() const;

    signals:
        void statusChanged();
};

/**
 * Adds a public append method
 */
class TestQueue : public JobQueue
{
    Q_OBJECT

    public:
        TestQueue(Scheduler *scheduler);
        void appendPub(Job *job);
};

class SchedulerTest : public QObject
{
    Q_OBJECT

    private slots:
        /**
         * Tests if the scheduler reacts correctly on appending jobs, i.e.
         * start/stop them
         */
        void testAppendJobs();
        void testAppendJobs_data();

        /**
         * Tests Scheduler::countRunningJobs and Scheduler::hasRunningJobs
         */
        void testCountRunningJobs();
        void testCountRunningJobs_data();

        /**
         * Tests if after stopping the scheduler all jobs are not running
         */
        void testStopScheduler();
        void testStopScheduler_data();

        /**
         * Stops the scheduler and then starts it again to see if jobs
         * are correctly started
         */
        void testSchedulerStartStop();
        void testSchedulerStartStop_data();

        void testSuspendScheduler();
        void testSuspendScheduler_data();

    private:
        void createJobs();
};

#endif
