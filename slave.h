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
#include <qevent.h>

#include "globals.h"

#include <kio/jobclasses.h>

namespace KIO
{
    class GetFileJob:public FileCopyJob
    {
    public:
	GetFileJob(const KURL & m_src, const KURL & m_dest);
	~GetFileJob();
	bool getCanResume() const;
    };
}

/**
  *@author Patrick Charbonnier
  */

class SlaveEvent:public QCustomEvent
{
public:
    SlaveEvent(Transfer * _item, unsigned int _event, unsigned long _ldata = 0L);
    SlaveEvent(Transfer * _item, unsigned int _event, const QString & _msg);

    unsigned int getEvent() const;
    Transfer *getItem() const;
    unsigned long getData() const;
    const QString & getMsg() const;


private:
    unsigned int m_event;
    Transfer *m_item;
    unsigned long m_ldata;
    QString m_msg;

};


class Slave:public QObject, public QThread
{
    Q_OBJECT 

public:
    enum SlaveCommand {
        RETR, PAUSE, RESTART, ABORT, DELAY,
        SCHEDULE, REMOVE, KILL, NOOP
    };

    enum SlaveResult {

        SLV_TOTAL_SIZE, SLV_PROGRESS_SIZE, SLV_PROGRESS_SPEED,
        SLV_CAN_RESUME, SLV_CONNECTED,

        SLV_RESUMED, SLV_PAUSED, SLV_ERROR, SLV_BROKEN, SLV_SCHEDULED, SLV_DELAYED,
        SLV_FINISHED, SLV_INFO, SLV_REMOVED, SLV_KILLED
    };

public:
    Slave(Transfer * _transfer, const KURL & _src, const KURL & _dest);
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

    Transfer * transfer;

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
