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
    : QObject(parent), m_jobs(), m_registeredJobs(), m_globalJob(0)
{
}

KUiServerJobs::~KUiServerJobs()
{
    foreach(KJob *job, m_jobs) {
        unregisterJob(job);
    }

    if(m_globalJob) {
        KIO::getJobTracker()->unregisterJob(globalJob());
        delete m_globalJob;
        m_globalJob = 0;
    }
}

void KUiServerJobs::registerJob(KJob *job)
{
    m_jobs.append(job);

    if(Settings::enableKUIServerIntegration()) {
        if(Settings::exportGlobalJob()) {
            globalJob()->registerJob(job);
        }
        else {
            m_registeredJobs.append(job);
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
            m_registeredJobs.removeAll(job);
            KIO::getJobTracker()->unregisterJob(job);
        }
    }
}

// every time the configuration changed, check the registered jobs and the state through the ui server
void KUiServerJobs::reload()
{
    if(Settings::exportGlobalJob() && Settings::enableKUIServerIntegration() && !m_globalJob) {
        foreach(KJob *job, m_registeredJobs) {
            KIO::getJobTracker()->unregisterJob(job);
            m_registeredJobs.removeAll(job);
        }
        KIO::getJobTracker()->registerJob(globalJob());
    }
    else {
        if((!Settings::exportGlobalJob() && Settings::enableKUIServerIntegration()) 
                                         || !Settings::enableKUIServerIntegration()) {
            if(m_globalJob) {
                KIO::getJobTracker()->unregisterJob(globalJob());
                delete m_globalJob;
                m_globalJob = 0;
            }
        }

        foreach(KJob *job, m_jobs) {
            if(!Settings::exportGlobalJob() && Settings::enableKUIServerIntegration() && job 
                                            && job->percent() < 100) {
                if(!m_registeredJobs.contains(job)) {
                    m_registeredJobs.append(job);
                    KIO::getJobTracker()->registerJob(job);
                }
            }
            else {
                m_registeredJobs.removeAll(job);
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

        foreach(KJob *job, m_jobs) {
            m_globalJob->registerJob(job);
        }

        if(Settings::exportGlobalJob() && Settings::enableKUIServerIntegration()) {
            KIO::getJobTracker()->registerJob(globalJob());
        }
    }

    return m_globalJob;
}
