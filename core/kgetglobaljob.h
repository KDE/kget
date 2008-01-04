/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
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
