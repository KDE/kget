/***************************************************************************
*                                slave.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
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


#include <kapplication.h>
#include <kio/scheduler.h>

#include "getfilejob.h"
#include "slave.h"
#include "slaveevent.h"
#include "transfer.h"

#include <assert.h>

Slave::Slave(Transfer * _parent, const KURL & _src, const KURL & _dest)
    : QObject()
{
    sDebug << ">>>>Entering" << endl;
    copyjob = NULL;
    m_src = _src;
    m_dest = _dest;
    m_parent = _parent;

    nPendingCommand = 0;

    sDebug << ">>>>Leaving" << endl;
}

Slave::~Slave()
{}

void Slave::Op(SlaveCommand _cmd)
{
    sDebugIn << " _cmd = " << _cmd << endl;

    stack.push(_cmd);
    nPendingCommand++;
    run();

    sDebugOut << endl;
}

/** No descriptions */
void Slave::PostMessage(SlaveResult _event, unsigned long _data)
{
    SlaveEvent *e1 = new SlaveEvent(m_parent, _event, _data);

    QApplication::postEvent(kapp->mainWidget(), (QEvent *) e1);
}

void Slave::PostMessage(SlaveResult _event, const QString & _msg)
{
    SlaveEvent *e1 = new SlaveEvent(m_parent, _event, _msg);

    QApplication::postEvent(kapp->mainWidget(), (QEvent *) e1);
    sDebug << "Msg:" << "_msg = " << _msg << endl;
}

void Slave::InfoMessage(const QString & _msg)
{
    SlaveEvent *e1 = new SlaveEvent(m_parent, SLV_INFO, _msg);

    QApplication::postEvent(kapp->mainWidget(), (QEvent *) e1);
    sDebug << "Infor Msg:" << "_msg = " << _msg << endl;
}



void Slave::run()
{
    sDebugIn << endl;

    SlaveCommand cmd;

        switch (cmd = fetch_cmd()) 
        {
            case RESTART:
                copyjob->kill(true);
                // fall through
            case RETR:
                sDebug << " FETCHED COMMAND       RETR" << endl;
                KIO::Scheduler::checkSlaveOnHold( true );
                copyjob = new KIO::GetFileJob(m_src, m_dest);
                copyjob->setAutoErrorHandlingEnabled(true);
                Connect();
                PostMessage(SLV_RESUMED);
                break;

            case PAUSE:
                sDebug << " FETCHED COMMAND       PAUSE" << endl;
                copyjob->kill(true);
                copyjob = 0L;
                PostMessage(SLV_PAUSED);
                break;

            case KILL:
                sDebug << " FETCHED COMMAND      KILL" << endl;
                if (copyjob)
                copyjob->kill(true);
                copyjob = 0L;
                // no message posted
                break;
            
            case REMOVE:
                sDebug << " FETCHED COMMAND       REMOVE" << endl;
                copyjob->kill(true);
                copyjob = 0L;
                PostMessage(SLV_REMOVED);
                break;

            case SCHEDULE:
                sDebug << " FETCHED COMMAND       SCHEDULE" << endl;
                copyjob->kill(true);
                copyjob = 0L;
                PostMessage(SLV_SCHEDULED);
                break;

            case DELAY:
                sDebug << " FETCHED COMMAND       DELAY" << endl;
                copyjob->kill(true);
                copyjob = 0L;
                PostMessage(SLV_DELAYED);
                break;

            case NOOP:
                sDebug << "FETCHED COMMAND        NOOP, i.e. empty stack" << endl;
	         if ( copyjob )
                    copyjob->kill(true);
                    copyjob = 0L;
                break;

            default: {
                sDebug << " UNKNOWN COMMAND DIE...." << endl;
                assert(0);
            }
        }

    sDebugOut << endl;
}


Slave::SlaveCommand Slave::fetch_cmd()
{
    SlaveCommand cmd = NOOP;
    if ( !stack.isEmpty() )
    {
        nPendingCommand--;
        cmd = stack.pop();
    }
    return cmd;
}


void Slave::Connect()
{
    sDebugIn << endl;


    connect(copyjob, SIGNAL(canceled(KIO::Job *)), SLOT(slotCanceled(KIO::Job *)));
    connect(copyjob, SIGNAL(connected(KIO::Job *)), SLOT(slotConnected(KIO::Job *)));
    connect(copyjob, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));

    connect(copyjob, SIGNAL(totalSize(KIO::Job *, KIO::filesize_t)), SLOT(slotTotalSize(KIO::Job *, KIO::filesize_t)));

    connect(copyjob, SIGNAL(processedSize(KIO::Job *, KIO::filesize_t)), SLOT(slotProcessedSize(KIO::Job *, KIO::filesize_t)));

    connect(copyjob, SIGNAL(speed(KIO::Job *, unsigned long)), SLOT(slotSpeed(KIO::Job *, unsigned long)));

    connect(copyjob, SIGNAL(infoMessage(KIO::Job *, const QString &)), SLOT(slotInfoMessage(KIO::Job *, const QString &)));

    sDebugOut << endl;
}


void Slave::slotCanceled(KIO::Job *)
{
    sDebugIn << endl;


    sDebugOut << endl;
}

void Slave::slotConnected(KIO::Job *)
{
    sDebugIn << endl;


    sDebugOut << endl;
}

void Slave::slotResult(KIO::Job * job)
{
    sDebugIn << endl;
    if (job->error()) {
        InfoMessage(job->errorString());
        PostMessage(SLV_DELAYED);
    } else
    {
        PostMessage(SLV_FINISHED);
    }
    copyjob=0L;
    sDebugOut << endl;
}


void Slave::slotSpeed(KIO::Job *, unsigned long lSpeed)
{
    // sDebugIn<<endl;
    PostMessage(SLV_PROGRESS_SPEED, lSpeed);
    // sDebugOut<<endl;

}

void Slave::slotTotalSize(KIO::Job *, KIO::filesize_t _total_size)
{
    sDebugIn << "= " << (unsigned long) _total_size << endl;
    PostMessage(SLV_TOTAL_SIZE, _total_size);

    PostMessage(SLV_CAN_RESUME, copyjob->getCanResume());
    PostMessage(SLV_CONNECTED);
    sDebugOut << endl;
}

void Slave::slotProcessedSize(KIO::Job *, KIO::filesize_t _processed_size)
{
    // sDebugIn<<endl;
    PostMessage(SLV_PROGRESS_SIZE, _processed_size);

    // sDebugOut<<endl;
}

void Slave::slotInfoMessage(KIO::Job *, const QString & _msg)
{
    sDebugIn << "MSG=" << _msg << endl;
    InfoMessage(_msg);
    sDebugOut << endl;
}

#include "slave.moc"
