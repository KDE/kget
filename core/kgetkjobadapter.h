/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday @ gmail.com>
   Idea by Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGETKJOBADAPTER_H
#define KGETKJOBADAPTER_H

#include "transferhandler.h"

#include <kio/job.h>

/**
* Allows kget to register all transfers in kuiserver as kjobs
*/
class KGetKJobAdapter : public KJob
{
    Q_OBJECT
public:
    KGetKJobAdapter(QObject *parent, TransferHandler *transfer);
    ~KGetKJobAdapter();

    void start() {};

    qulonglong processedAmount(Unit unit) const;
    qulonglong totalAmount(Unit unit) const;
    unsigned long percent() const;

public slots:
    void slotUpdateDescription();

signals:
    /**
     * Emitted when doKill is called, e.g. when the gui is closed.
     * Not handling this signal might lead to a crash if something tries to
     * access the then non-existing gui.
     */
    void requestStop(KJob *job, TransferHandler *handler);
    void requestSuspend(KJob *job, TransferHandler *handler);
    void requestResume(KJob *job, TransferHandler *handler);

protected:
    virtual bool doKill();
    virtual bool doSuspend();
    virtual bool doResume();

private:
    TransferHandler *m_transferHandler;
};
#endif
