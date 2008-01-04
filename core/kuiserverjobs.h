/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
