/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KUISERVERJOBS_H
#define KUISERVERJOBS_H

#include "kgetglobaljob.h"

#include <kio/job.h>
#include <kio/filejob.h>

#include <QObject>
#include <QList>

class KUiServerJobs : public QObject
{
    Q_OBJECT
public:
    KUiServerJobs(QObject *parent=0);
    ~KUiServerJobs();

    void registerJob(KJob *job);
    void unregisterJob(KJob *job);
    void reload();

private:
    KGetGlobalJob *globalJob();

    QList <KJob *> m_jobs;
    KGetGlobalJob *m_globalJob;
};

#endif
