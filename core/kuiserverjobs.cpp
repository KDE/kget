/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kuiserverjobs.h"

#include "kgetglobaljob.h"
#include "settings.h"

#include <kuiserverjobtracker.h>

KUiServerJobs::KUiServerJobs(QObject *parent)
    : QObject(parent), m_jobs()
{
}

KUiServerJobs::~KUiServerJobs()
{
}

void KUiServerJobs::registerJob(KJob *job)
{
    m_jobs.append(job);

    if(Settings::enableKUIServerIntegration()) {
        if(Settings::exportGlobalJob()) {
            globalJob()->registerJob(job);
        }
        else {
            KIO::getJobTracker()->registerJob(job);
        }
    }
}

void KUiServerJobs::unregisterJob(KJob *job)
{
    m_jobs.removeAll(job);

    if(Settings::enableKUIServerIntegration()) {
        if(Settings::exportGlobalJob()) {
            globalJob()->unregisterJob(job);
        }
        else {
            KIO::getJobTracker()->unregisterJob(job);
        }
    }
}

// every time the configuration changed, check the registered jobs and the state through the ui server
void KUiServerJobs::reload()
{
    if(m_globalJob && !Settings::exportGlobalJob()) {
        KIO::getJobTracker()->unregisterJob(globalJob());
        delete m_globalJob;
    }

    if(Settings::exportGlobalJob()) {
        KIO::getJobTracker()->registerJob(globalJob());
    }
    else {
        foreach(KJob *job, m_jobs) {
            if(Settings::enableKUIServerIntegration()) {
                KIO::getJobTracker()->registerJob(job);
            }
            else {
                KIO::getJobTracker()->unregisterJob(job);
            }
        }
    }
}

// get the kget global job
KGetGlobalJob *KUiServerJobs::globalJob()
{
    if(!m_globalJob) {
        m_globalJob = new KGetGlobalJob();
        KIO::getJobTracker()->registerJob(m_globalJob);
    }

    return m_globalJob;
}
