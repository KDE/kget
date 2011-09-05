/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/job.h"

#include "core/jobqueue.h"
#include "core/scheduler.h"

#include <kdebug.h>

Job::Job(Scheduler * scheduler, JobQueue * parent)
    : QObject(parent),
      m_jobQueue(parent),
      m_scheduler(scheduler),
      m_status(Stopped),
      m_policy(None)
{
    m_error.id = -1;
    m_error.type = AutomaticRetry;
}

Job::~Job()
{
}

void Job::setStatus(Status jobStatus)
{
    if(jobStatus == m_status)
        return;
    if (m_status == Aborted) {
        m_error.id = -1;
        m_error.text.clear();
        m_error.pixmap = QPixmap();
        m_error.type = AutomaticRetry;
    }
    m_status = jobStatus;
    m_scheduler->jobChangedEvent(this, m_status);
}

void Job::setStartStatus(Status jobStatus)
{
    kDebug(5001) << "Setting start status to " << jobStatus;
    m_startStatus = jobStatus;
}

void Job::setPolicy(Policy jobPolicy)
{
    if(jobPolicy == m_policy)
        return;

    kDebug(5001) << "Job::setPolicy(" << jobPolicy << ")";

    m_policy = jobPolicy;
    m_scheduler->jobChangedEvent(this, m_policy);
}

void Job::setError(const QString &text, const QPixmap &pixmap, ErrorType type, int errorId)
{
    setStatus(Job::Aborted);
    m_error.id = errorId;
    m_error.text = text;
    m_error.pixmap = pixmap;
    m_error.type = type;
}

void Job::resolveError(int errorId)
{
    Q_UNUSED(errorId)
}
