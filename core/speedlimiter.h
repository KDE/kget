/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef SPEEDLIMITER_H
#define SPEEDLIMITER_H

#include <kio/job.h>

class QTimer;

namespace KIO {

/**
 *  The KSpeedLimiter is designed to throtle the speed in kio jobs
 */

class KSpeedLimiter : public QObject
{
Q_OBJECT

public:
    KSpeedLimiter();
    ~KSpeedLimiter();

    void setSpeedLimit(unsigned long speed);
    unsigned long speedLimit();

    void addJob(KJob *job);
    void start();
    void stop();

private Q_SLOTS:
    void slotTimer();
    void slotResult(KJob *job);

private:
    unsigned long m_speedLimit;
    QTimer *m_timer;
    QList<KJob *> m_jobs;

};

} // namespace

#endif
