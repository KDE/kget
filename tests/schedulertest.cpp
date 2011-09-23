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

Q_DECLARE_METATYPE(QList<Job::Status>)

SettingsHelper::SettingsHelper(bool useLimit, int limit)
  : m_oldUseLimit(Settings::limitDownloads()),
    m_oldLimit(Settings::maxConnections())
{
    Settings::setLimitDownloads(useLimit);
    Settings::setMaxConnections(limit);
}

SettingsHelper::~SettingsHelper()
{
    Settings::setLimitDownloads(m_oldUseLimit);
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
    QFETCH(bool, useLimit);
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(useLimit, limit);

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
    QTest::addColumn<bool>("useLimit");
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    QTest::newRow("limit 2, two finished, will third be started?") << true << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will be stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("no limit though 1 set, two finished, will third be started?") << false << 1 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit though 1 set, will all three be started?") << false << 1 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);
} 

void SchedulerTest::testCountRunningJobs()
{
    QFETCH(bool, useLimit);
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(int, numRunningJobs);

    SettingsHelper helper(useLimit, limit);

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
    QTest::addColumn<bool>("useLimit");
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<int>("numRunningJobs");

    QTest::newRow("limit 2, two finished, will third be started?") << true << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << 1;
    QTest::newRow("limit 2, two finished, will none be started?") << true << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished) << 0;
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << 2;
    QTest::newRow("limit 2, will first two start while last will be stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << 2;
    QTest::newRow("no limit though 1 set, two finished, will third be started?") << false << 1 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << 1;
    QTest::newRow("no limit though 1 set, will all three be started?") << false << 1 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << 3;
}

void SchedulerTest::testStopScheduler()
{
    QFETCH(bool, useLimit);
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);

    SettingsHelper helper(useLimit, limit);

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
    QTest::addColumn<bool>("useLimit");
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");

    QTest::newRow("limit 2, two finished, will third be started?") << true << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will be stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running);
    QTest::newRow("no limit though 1 set, two finished, will third be started?") << false << 1 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped);
    QTest::newRow("no limit though 1 set, will all three be started?") << false << 1 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped);
}

void SchedulerTest::testSchedulerStartStop()
{
    QFETCH(bool, useLimit);
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(useLimit, limit);

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

void SchedulerTest::testSchedulerStartStop_data()
{
    QTest::addColumn<bool>("useLimit");
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    QTest::newRow("limit 2, two finished, will third be started?") << true << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will be stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("no limit though 1 set, two finished, will third be started?") << false << 1 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit though 1 set, will all three be started?") << false << 1 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);
}


void SchedulerTest::testSuspendScheduler()
{
    QFETCH(bool, useLimit);
    QFETCH(int, limit);
    QFETCH(QList<Job::Status>, status);
    QFETCH(QList<Job::Status>, finalStatus);

    SettingsHelper helper(useLimit, limit);

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
    QTest::addColumn<bool>("useLimit");
    QTest::addColumn<int>("limit");
    QTest::addColumn<QList<Job::Status> >("status");
    QTest::addColumn<QList<Job::Status> >("finalStatus");

    QTest::newRow("limit 2, two finished, will third be started?") << true << 2 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("limit 2, will first two start while last will stay stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("limit 2, will first two start while last will be stopped?") << true << 2 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Running) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Stopped);
    QTest::newRow("no limit though 1 set, two finished, will third be started?") << false << 1 << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Stopped) << (QList<Job::Status>() << Job::Finished << Job::Finished << Job::Running);
    QTest::newRow("no limit though 1 set, will all three be started?") << false << 1 << (QList<Job::Status>() << Job::Stopped << Job::Stopped << Job::Stopped) << (QList<Job::Status>() << Job::Running << Job::Running << Job::Running);

}

QTEST_MAIN(SchedulerTest)

#include "schedulertest.moc"
