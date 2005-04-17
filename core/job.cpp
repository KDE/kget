/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "core/job.h"
#include "core/jobqueue.h"
#include "core/scheduler.h"

Job::Job(JobQueue * parent, Scheduler * scheduler)
    : m_jobQueue(parent),
      m_scheduler(scheduler),
      m_status(Stopped),
      m_policy(None)
{

}


void Job::setStatus(Status jobStatus)
{
    m_status = jobStatus;
    m_scheduler->jobChangedEvent(this, m_status);
}

void Job::setPolicy(Policy jobPolicy)
{
    m_policy = jobPolicy;
    m_scheduler->jobChangedEvent(this, m_status);
}

