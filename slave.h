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
#include <kio/job.h>
#include <qthread.h>
#include <kurl.h>
#include <kapp.h>
#include <qfile.h>
#include "slaveevent.h"

#include "common.h"
#include <assert.h>
#include <klocale.h>
#include <qvaluestack.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qobject.h>
#include "getfilejob.h"
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

        enum SlaveStatus {

                SLV_RUNNING, SLV_STOPPING, SLV_FINISHING, SLV_ABORTING
        };

public:
        Slave(Transfer * _parent, const KURL & _src, const KURL & _dest);
        ~Slave();
        void Op(SlaveCommand _cmd);

        /** No descriptions */
        void PostMessage(SlaveResult _event, unsigned long _data = 0L);
        void PostMessage(SlaveResult _event, const QString & _msg);
        void InfoMessage(const QString & _msg);


public slots:
        /** No descriptions */
        void slotCanceled(KIO::Job *);
        /** No descriptions */
        void slotConnected(KIO::Job *);
        /** No descriptions */
        void slotResult(KIO::Job *);
        /** No descriptions */
        void slotTotalSize(KIO::Job *, KIO::filesize_t);
        /** No descriptions */
        void slotProcessedSize(KIO::Job *, KIO::filesize_t);
        /** No descriptions */
        void slotSpeed(KIO::Job *, unsigned long);
        /** No descriptions */
        void slotInfoMessage(KIO::Job *, const QString &);

public:
        QValueStack < SlaveCommand > stack;
        QWaitCondition worker;
        QMutex mutex;
        KIO::GetFileJob * copyjob;

private:                     // Private attributes
        virtual void run();
        void Connect();


protected:

        Transfer * m_parent;

public:                      // Public attributes
        KURL m_src;
        KURL m_dest;
private:                     // Private methods

        /** No descriptions */
        Slave::SlaveCommand fetch_cmd();
        int nPendingCommand;
};

#endif
