/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef SPEEDLIMITER_H
#define SPEEDLIMITER_H

#include <kio/job.h>

#include <QtCore/QTime>

class KJobSpeedLimiter;

/**
 *  The KSpeedLimiter is designed to throtle the speed in kio jobs
 */

class KSpeedLimiter
{

public:
    KSpeedLimiter(unsigned long speed);
    ~KSpeedLimiter();

    void setSpeedLimit(unsigned long speed);
    unsigned long speedLimit();

    void addJob(KJob *job);

private:
    unsigned long m_speedLimit;
    QList<KJobSpeedLimiter *> m_JobSpeedLimiters;

};

class KJobSpeedLimiter : public QObject
{
Q_OBJECT

public:
    KJobSpeedLimiter(KJob *job, unsigned long speedLimit);
    ~KJobSpeedLimiter();

private Q_SLOTS:
    void slotOnTimer();
    void slotOffTimer();
    void slotResult(KJob *job);

private:
    bool isSuspended();
    void suspend();
    void resume();

private:
    unsigned long m_speedLimit;
    unsigned long m_channelSpeed;
    KJob *m_job;
    qulonglong m_bytes;
    QTime m_time;
    bool m_isSuspended;
    int m_OnTime;
    int m_OffTime;
};

#endif
