/***************************************************************************
*                                slave.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/


#ifndef SLAVE_H
#define SLAVE_H

#include <qthread.h>
#include <kurl.h>
#include <qvaluestack.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qobject.h>

#include "common.h"

namespace KIO
{
    class GetFileJob;
};

class Transfer;

class Slave:public QObject, public QThread
{
Q_OBJECT public:
    enum SlaveCommand {
        RETR, PAUSE, RESTART, ABORT, DELAY,
        SCHEDULE, REMOVE, NOOP
    };

    enum SlaveResult {

        SLV_TOTAL_SIZE, SLV_PROGRESS_SIZE, SLV_PROGRESS_SPEED,
        SLV_CAN_RESUME, SLV_CONNECTED,

        SLV_RESUMED, SLV_PAUSED, SLV_ABORTED, SLV_SCHEDULED, SLV_DELAYED,
        SLV_FINISHED, SLV_INFO, SLV_REMOVED
    };

    // ### those don't seem to be used at all!!
    enum SlaveStatus {

        SLV_RUNNING, SLV_STOPPING, SLV_FINISHING, SLV_ABORTING
    };

public:
    Slave(Transfer * _parent, const KURL & _src, const KURL & _dest);
    ~Slave();
    void Op(SlaveCommand _cmd);

protected:
    virtual void run();

private slots:
    void slotCanceled(KIO::Job *);
    void slotConnected(KIO::Job *);
    void slotResult(KIO::Job *);
    void slotTotalSize(KIO::Job *, KIO::filesize_t);
    void slotProcessedSize(KIO::Job *, KIO::filesize_t);
    void slotSpeed(KIO::Job *, unsigned long);
    void slotInfoMessage(KIO::Job *, const QString &);

private:
    void Connect();

    void PostMessage(SlaveResult _event, unsigned long _data = 0L);
    void PostMessage(SlaveResult _event, const QString & _msg);
    void InfoMessage(const QString & _msg);

    Transfer * m_parent;

    KURL m_src;
    KURL m_dest;

    Slave::SlaveCommand fetch_cmd();
    int nPendingCommand;

    QValueStack < SlaveCommand > stack;
    QWaitCondition worker;
    QMutex mutex;
    KIO::GetFileJob * copyjob;

};

#endif
