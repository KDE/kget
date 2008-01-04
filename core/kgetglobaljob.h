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
#ifndef KGETGLOBALJOB_H
#define KGETGLOBALJOB_H

#include <kio/job.h>
#include <kio/filejob.h>

#include <QList>

#define DEFAULT_UPDATE_TIME 5000

class QTimer;

class KGetGlobalJob : public KJob
{
    Q_OBJECT
public:
    KGetGlobalJob(QObject *parent=0);
    ~KGetGlobalJob();

    void registerJob(KJob *);
    void unregisterJob(KJob *);

    void start() {};

    // reimplement this functions from KJob to query all the child jobs
    qulonglong processedAmount(Unit unit) const;
    qulonglong totalAmount(Unit unit) const;
    unsigned long percent() const;

private slots:
    void update();

private:
    QList <KJob *> m_jobs;

    QTimer *m_timer;
};

#endif
