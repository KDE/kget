/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2009 by Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef KGETGLOBALJOB_H
#define KGETGLOBALJOB_H

#include <kio/job.h>

class TransferHandler;

class KGetGlobalJob : public KJob
{
    Q_OBJECT
public:
    KGetGlobalJob(QObject *parent=0);
    ~KGetGlobalJob();

    void update();    
    
    void start() {};

signals:
    /**
     * Emitted when doKill is called, e.g. when the gui is closed.
     * Not handling this signal might lead to a crash if something tries to
     * access the then non-existing gui.
     * @param job is this
     * @param handler is always 0 suggesting that all TransferHandlers should be stopped
     */
    void requestStop(KJob *job, TransferHandler *handler);

protected:
    virtual bool doKill();
};

#endif
