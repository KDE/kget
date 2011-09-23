/***************************************************************************
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

#include "schedulertest.h"
#include "../core/scheduler.h"
#include "../settings.h"

#include <QtCore/QVariant>
#include <QtTest/QtTest>

Q_DECLARE_METATYPE(Job::ErrorType)
Q_DECLARE_METATYPE(Job::Status)
Q_DECLARE_METATYPE(Job::Policy)
Q_DECLARE_METATYPE(QList<Job::Status>)
Q_DECLARE_METATYPE(QList<Job::Policy>)

const int SchedulerTest::NO_LIMIT = 0;

SettingsHelper::SettingsHelper(int limit)
  : m_oldLimit(Settings::maxConnections())
{
    Settings::setMaxConnections(limit);
}

SettingsHelper::~SettingsHelper()
{
    Settings::setMaxConnections(m_oldLimit);
}

TestJob::TestJob(Scheduler *scheduler, JobQueue *parent)
  : Job(scheduler, parent)
{
}

void TestJob::start()
{
    if (status() == Aborted || status() == Stopped) {
        setStatus(Running);
    }
}

void TestJob::stop()
{
    if (status() == Running || status() == Aborted || status() == Moving) {
        setStatus(Stopped);
    }
}

int TestJob::elapsedTime() const
{
    return 0;
}

int TestJob::remainingTime() const
{
    return 0;
}

bool TestJob::isStalled() const
{
    return false;
}

bool TestJob::isWorking() const
{
    return true;
}

TestQueue::TestQueue(Scheduler *scheduler)
  : JobQueue(scheduler)
{
}

void TestQueue::appendPub(Job *job)
{
    append(job);
}

void SchedulerTest::testAppendJobs()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);

    //uses an own list instead of the iterators to make sure that the order stays the same
    QList<TestJob*> jobs;
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        queue->appendPub(job);
        jobs << job;
    }

    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), finalStatus[i]);
    }
}

void SchedulerTest::testAppendJobs_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    QTest::newRow("limit 2, two finished, will third be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, two finished, will third aborted be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Aborted) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will be stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("no limit, two finished, will third be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit, will all three be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);
    QTest::newRow("no limit, will all three be started and one remain running?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Running << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running << Job::Running);
} 

void SchedulerTest::testCountRunningJobs()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(int, numRunningJobs);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);

    //uses an own list instead of the iterators to make sure that the order stays the same
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        queue->appendPub(job);
    }

    QCOMPARE(scheduler.countRunningJobs(), numRunningJobs);

    const bool hasRunningJobs = numRunningJobs;
    QCOMPARE(scheduler.hasRunningJobs(), hasRunningJobs);
}

void SchedulerTest::testCountRunningJobs_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<int>("numRunningJobs");

    QTest::newRow("limit 2, two finished, will third be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << 1;
    QTest::newRow("limit 2, two finished, will none be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished) << 0;
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << 2;
    QTest::newRow("limit 2, will first two start while last will be stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << 2;
    QTest::newRow("no limit, two finished, will third be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << 1;
    QTest::newRow("no limit, two finished, will third be started and fourth stay running?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped << Job::Running) << 2;
    QTest::newRow("no limit, will all three be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << 3;
}

void SchedulerTest::testStopScheduler()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);

    //uses an own list instead of the iterators to make sure that the order stays the same
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        queue->appendPub(job);
    }

    scheduler.stop();

    QCOMPARE(scheduler.countRunningJobs(), 0);

}

void SchedulerTest::testStopScheduler_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");

    QTest::newRow("limit 2, two finished one stopped") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped);
    QTest::newRow("limit 2, two finished one running") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped);
    QTest::newRow("limit 2, three stopped") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped);
    QTest::newRow("limit 2, two stopped one running") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running);
    QTest::newRow("no limit, two finished one stopped") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped);
    QTest::newRow("no limit, three stopped") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped);
    QTest::newRow("no limit, one running, three stopped") << NO_LIMIT << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Stopped << Job::Stopped);
}

void SchedulerTest::testSchedulerStopStart()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);

    //uses an own list instead of the iterators to make sure that the order stays the same
    QList<TestJob*> jobs;
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        queue->appendPub(job);
        jobs << job;
    }

    scheduler.stop();
    scheduler.start();

    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), finalStatus[i]);
    }
}

void SchedulerTest::testSchedulerStopStart_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    QTest::newRow("limit 2, two finished, will third be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will be stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("no limit, two finished, will third be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit, will all three be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);
    QTest::newRow("limit 2, two finished, will third stay running?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
}


void SchedulerTest::testSuspendScheduler()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);
    scheduler.setIsSuspended(true);

    //uses an own list instead of the iterators to make sure that the order stays the same
    QList<TestJob*> jobs;
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        queue->appendPub(job);
        jobs << job;
    }

    //scheduler is suspended thus the status has to be same as start status
    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), status[i]);
    }
    scheduler.setIsSuspended(false);

    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), finalStatus[i]);
    }
}

void SchedulerTest::testSuspendScheduler_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    //NOTE Scheduler does not stop jobs, it just prevents new ones from being started
    QTest::newRow("limit 2, two finished, will third be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("limit 2, will first start and second not while last will stay running?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("no limit, two finished, will third be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit, will all three be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);
    QTest::newRow("limit 2, two finished, will third stay running?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);

}

void SchedulerTest::testJobQueueStopPolicy()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);
    QFETCH(QList<Job::Policy>, policy);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    queue->setStatus(JobQueue::Stopped);
    scheduler.addQueue(queue);

    //uses an own list instead of the iterators to make sure that the order stays the same
    QList<TestJob*> jobs;
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        job->setPolicy(policy[i]);
        queue->appendPub(job);
        jobs << job;
    }

    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), finalStatus[i]);
    }
}

void SchedulerTest::testJobQueueStopPolicy_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");
    QTest::addColumn<QList<Job::Policy> >("policy");

    QTest::newRow("limit 2, two finished, will third not be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Policy>() << Job::None << Job::None << Job::None);
    QTest::newRow("limit 2, will first start while rest will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Stopped) << (QList<Job::Policy>() << Job::Start << Job::Stop << Job::None);
    QTest::newRow("limit 2, will first and third start while rest will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Running << Job::Stopped) << (QList<Job::Policy>() << Job::Start << Job::Stop << Job::Start << Job::None);
    QTest::newRow("no limit, two finished, will third be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running) << (QList<Job::Policy>() << Job::Start << Job::None << Job::Start);
    QTest::newRow("no limit, will all three be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running) << (QList<Job::Policy>() << Job::Start << Job::Start << Job::Start);
}

void SchedulerTest::testJobQueueStopStartPolicy()
{
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, intermediateStatus);
    QFETCH(QList<Job::Policy>, policy);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(limit);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    queue->setStatus(JobQueue::Stopped);
    scheduler.addQueue(queue);

    //uses an own list instead of the iterators to make sure that the order stays the same
    QList<TestJob*> jobs;
    for (int i = 0; i < status.size(); ++i) {
        TestJob *job = new TestJob(&scheduler, queue);
        job->setStatus(status[i]);
        job->setPolicy(policy[i]);
        queue->appendPub(job);
        jobs << job;
    }

    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), intermediateStatus[i]);
    }

    queue->setStatus(JobQueue::Running);

    for (int i = 0; i < status.size(); ++i) {
        QCOMPARE(jobs[i]->status(), finalStatus[i]);

    }
}

void SchedulerTest::testJobQueueStopStartPolicy_data()
{
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("intermediateStatus");
    QTest::addColumn<QList<Job::Policy> >("policy");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    QTest::newRow("limit 2, two finished, will third be started?") << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Policy>() << Job::None << Job::None << Job::None) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first and last start while rest will stay stopped?") << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Stopped) << (QList<Job::Policy>() << Job::Start << Job::Stop << Job::None) << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Running);
    QTest::newRow("limit 3, will first, third and last start while rest will stay stopped?") << 3 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Running << Job::Stopped) << (QList<Job::Policy>() << Job::Start << Job::Stop << Job::Start << Job::None) << (QList<Job::Status>() << Job::Running << Job::Stopped << Job::Running << Job::Running);
    QTest::newRow("no limit, two finished, will third be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running) << (QList<Job::Policy>() << Job::Start << Job::Start << Job::Start) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit, will all three be started?") << NO_LIMIT << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running) << (QList<Job::Policy>() << Job::Start << Job::Start << Job::Start) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);
}

void SchedulerTest::testJobErrorType()
{
    QFETCH(bool, jobQueueRunning);
    QFETCH(Job::Policy, policy);
    QFETCH(Job::ErrorType, errorType);
    QFETCH(Job::Status, finalStatus);

    SettingsHelper helper(NO_LIMIT);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    queue->setStatus(jobQueueRunning ? JobQueue::Running : JobQueue::Stopped);
    scheduler.addQueue(queue);

    TestJob *job = new TestJob(&scheduler, queue);
    job->setPolicy(policy);
    job->setError(QString(), QPixmap(), errorType);
    queue->appendPub(job);

    QCOMPARE(job->status(), finalStatus);
}

void SchedulerTest::testJobErrorType_data()
{
    QTest::addColumn<bool>("jobQueueRunning");
    QTest::addColumn<Job::Policy>("policy");
    QTest::addColumn<Job::ErrorType>("errorType");
    QTest::addColumn<Job::Status>("finalStatus");

    QTest::newRow("queue running, job::none, autoretry, running") << true << Job::None << Job::AutomaticRetry << Job::Running;
    QTest::newRow("queue running, job::start, autoretry, running") << true << Job::Start << Job::AutomaticRetry << Job::Running;
    QTest::newRow("queue running, job::stop, autoretry, aborted") << true << Job::Stop << Job::AutomaticRetry << Job::Aborted;
    QTest::newRow("queue running, job::none, manualsolve, aborted") << true << Job::None << Job::ManualSolve << Job::Aborted;
    QTest::newRow("queue running, job::start, manualsolve, aborted") << true << Job::Start << Job::ManualSolve << Job::Aborted;
    QTest::newRow("queue running, job::stop, manualsolve, aborted") << true << Job::Stop << Job::ManualSolve << Job::Aborted;
    QTest::newRow("queue running, job::none, notsolveable, aborted") << true << Job::None << Job::NotSolveable << Job::Aborted;
    QTest::newRow("queue running, job::start, notsolveable, aborted") << true << Job::Start << Job::NotSolveable << Job::Aborted;
    QTest::newRow("queue running, job::stop, notsolveable, aborted") << true << Job::Stop << Job::NotSolveable << Job::Aborted;
    QTest::newRow("queue stopped, job::none, autoretry, aborted") << false << Job::None << Job::AutomaticRetry << Job::Aborted;
    QTest::newRow("queue stopped, job::start, autoretry, running") << false << Job::Start << Job::AutomaticRetry << Job::Running;
    QTest::newRow("queue stopped, job::stop, autoretry, aborted") << false << Job::Stop << Job::AutomaticRetry << Job::Aborted;
    QTest::newRow("queue stopped, job::none, manualsolve, aborted") << false << Job::None << Job::ManualSolve << Job::Aborted;
    QTest::newRow("queue stopped, job::start, manualsolve, aborted") << false << Job::Start << Job::ManualSolve << Job::Aborted;
    QTest::newRow("queue stopped, job::stop, manualsolve, aborted") << false << Job::Stop << Job::ManualSolve << Job::Aborted;
    QTest::newRow("queue stopped, job::none, notsolveable, aborted") << false << Job::None << Job::NotSolveable << Job::Aborted;
    QTest::newRow("queue stopped, job::start, notsolveable, aborted") << false << Job::Start << Job::NotSolveable << Job::Aborted;
    QTest::newRow("queue stopped, job::stop, notsolveable, aborted") << false << Job::Stop << Job::NotSolveable << Job::Aborted;
}

void SchedulerTest::testGettingNetworkConnection()
{
    QFETCH(Job::Policy, policy);
    QFETCH(Job::Status, finalStatus);

    SettingsHelper helper(NO_LIMIT);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);
    scheduler.setHasNetworkConnection(false);

    TestJob *job = new TestJob(&scheduler, queue);
    job->setPolicy(policy);
    queue->appendPub(job);

    scheduler.setHasNetworkConnection(true);

    QCOMPARE(job->policy(), policy);//the policy should remain unchanged
    QCOMPARE(job->status(), finalStatus);
}

void SchedulerTest::testGettingNetworkConnection_data()
{
    QTest::addColumn<Job::Policy>("policy");
    QTest::addColumn<Job::Status>("finalStatus");

    QTest::newRow("Start, Running") << Job::Start << Job::Running;
    QTest::newRow("Stop, Stopped") << Job::Stop << Job::Stopped;
    QTest::newRow("None, Running") << Job::None << Job::Running;
}

void SchedulerTest::testLosingNetworkConnection()
{
    QFETCH(Job::Policy, policy);

    SettingsHelper helper(NO_LIMIT);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);
    scheduler.setHasNetworkConnection(true);

    TestJob *job = new TestJob(&scheduler, queue);
    job->setPolicy(policy);
    queue->appendPub(job);

    scheduler.setHasNetworkConnection(false);

    QCOMPARE(job->policy(), policy);//the policy should remain unchanged
    QCOMPARE(job->status(), Job::Stopped);
}

void SchedulerTest::testLosingNetworkConnection_data()
{
    QTest::addColumn<Job::Policy>("policy");

    QTest::newRow("Start") << Job::Start;
    QTest::newRow("Stop") << Job::Stop;
    QTest::newRow("None") << Job::None;
}

void SchedulerTest::testShouldUpdate()
{
    QFETCH(bool, isSuspended);
    QFETCH(bool, hasNetworkConnection);
    QFETCH(bool, shouldUpdate);

    SettingsHelper helper(NO_LIMIT);

    Scheduler scheduler;
    TestQueue *queue = new TestQueue(&scheduler);
    scheduler.addQueue(queue);

    QVERIFY(scheduler.shouldUpdate());//should be true by default

    scheduler.setIsSuspended(isSuspended);
    scheduler.setHasNetworkConnection(hasNetworkConnection);

    QCOMPARE(scheduler.shouldUpdate(), shouldUpdate);
}

void SchedulerTest::testShouldUpdate_data()
{
    QTest::addColumn<bool>("isSuspended");
    QTest::addColumn<bool>("hasNetworkConnection");
    QTest::addColumn<bool>("shouldUpdate");

    QTest::newRow("false, true, true") << false << true << true;
    QTest::newRow("true, true, false") << true << true << false;
    QTest::newRow("false, false, false") << false << false << false;
}

QTEST_MAIN(SchedulerTest)

#include "schedulertest.moc"
