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
    : QObject(),
      QThread()
{
    mDebug << ">>>>Entering" << endl;
    copyjob = NULL;
    m_src = _src;
    m_dest = _dest;
    m_parent = _parent;

    nPendingCommand = 0;

    mDebug << ">>>>Leaving" << endl;
}

Slave::~Slave()
{}

void Slave::Op(SlaveCommand _cmd)
{
    mDebugIn << " _cmd = " << _cmd << endl;

    if ( !running() ) // start on demand
        start();

    mutex.lock();
    stack.push(_cmd);
    nPendingCommand++;
    worker.wakeOne();
    mutex.unlock();

    mDebugOut << endl;
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
    mDebug << "Msg:" << "_msg = " << _msg << endl;
}

void Slave::InfoMessage(const QString & _msg)
{
    SlaveEvent *e1 = new SlaveEvent(m_parent, SLV_INFO, _msg);

    QApplication::postEvent(kapp->mainWidget(), (QEvent *) e1);
    mDebug << "Infor Msg:" << "_msg = " << _msg << endl;
}



void Slave::run()
{
    mDebugIn << endl;

    SlaveCommand cmd;
    bool running = true;

    while (running) 
    {
        if (!nPendingCommand)
            worker.wait();
        switch (cmd = fetch_cmd()) 
        {
            case RESTART:
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                // fall through
            case RETR:
                mDebug << " FETCHED COMMAND       RETR" << endl;
                assert(!copyjob);
                KIO::Scheduler::checkSlaveOnHold( true );
                copyjob = new KIO::GetFileJob(m_src, m_dest);
                Connect();
                PostMessage(SLV_RESUMED);
                break;

            case PAUSE:
                mDebug << " FETCHED COMMAND       PAUSE" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                PostMessage(SLV_PAUSED);
                break;

            case KILL:
                mDebug << " FETCHED COMMAND      KILL" << endl;
                running = false;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                 }
                // no message posted
                break;
            
            case REMOVE:
                mDebug << " FETCHED COMMAND       REMOVE" << endl;
                running = false;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                PostMessage(SLV_REMOVED);
                break;

            case SCHEDULE:
                mDebug << " FETCHED COMMAND       SCHEDULE" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                PostMessage(SLV_SCHEDULED);
                break;

            case DELAY:
                mDebug << " FETCHED COMMAND       DELAY" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                PostMessage(SLV_DELAYED);
                break;

            case NOOP:
                mDebug << "FETCHED COMMAND        NOOP, i.e. empty stack" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                running = false;
                break;

            default:
                mDebug << " UNKNOWN COMMAND DIE...." << endl;
                assert(0);
        }
    }

    assert(!copyjob);
    mDebugOut << endl;
}


Slave::SlaveCommand Slave::fetch_cmd()
{
    mutex.lock();
    SlaveCommand cmd = NOOP;
    if ( !stack.isEmpty() )
    {
        nPendingCommand--;
        cmd = stack.pop();
    }
    mutex.unlock();
    return cmd;
}


void Slave::Connect()
{
    mDebugIn << endl;


    connect(copyjob, SIGNAL(canceled(KIO::Job *)), SLOT(slotCanceled(KIO::Job *)));
    connect(copyjob, SIGNAL(connected(KIO::Job *)), SLOT(slotConnected(KIO::Job *)));
    connect(copyjob, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));

    connect(copyjob, SIGNAL(totalSize(KIO::Job *, KIO::filesize_t)), SLOT(slotTotalSize(KIO::Job *, KIO::filesize_t)));

    connect(copyjob, SIGNAL(processedSize(KIO::Job *, KIO::filesize_t)), SLOT(slotProcessedSize(KIO::Job *, KIO::filesize_t)));

    connect(copyjob, SIGNAL(speed(KIO::Job *, unsigned long)), SLOT(slotSpeed(KIO::Job *, unsigned long)));

    connect(copyjob, SIGNAL(infoMessage(KIO::Job *, const QString &)), SLOT(slotInfoMessage(KIO::Job *, const QString &)));

    mDebugOut << endl;
}


void Slave::slotCanceled(KIO::Job *)
{
    mDebugIn << endl;


    mDebugOut << endl;
}

void Slave::slotConnected(KIO::Job *)
{
    mDebugIn << endl;


    mDebugOut << endl;
}

void Slave::slotResult(KIO::Job * job)
{
    mDebugIn << endl;
    
    assert(copyjob == job);
    copyjob=0L;
    KIO::Error error=KIO::Error(job->error());
    if (!error) {
        PostMessage(SLV_FINISHED);
    }
    else {
        QString tmsg="<font color=\"red\"> <b>" + job->errorString() + \
                "</b></font>";
        InfoMessage(tmsg);
        if (m_parent->retryOnError() && \
            ((error==KIO::ERR_COULD_NOT_LOGIN) || (error==KIO::ERR_SERVER_TIMEOUT))) {
            //Timeout or login error
            PostMessage(SLV_ERROR);
        }
        else if (m_parent->retryOnBroken() && (error==KIO::ERR_CONNECTION_BROKEN)) {
            // Connection Broken
            PostMessage(SLV_BROKEN);
        }
        else {
            job->showErrorDialog();
            PostMessage(SLV_DELAYED);
        }
    }
    mDebugOut << endl;
}


void Slave::slotSpeed(KIO::Job *, unsigned long lSpeed)
{
    // mDebugIn<<endl;
    PostMessage(SLV_PROGRESS_SPEED, lSpeed);
    // mDebugOut<<endl;

}

void Slave::slotTotalSize(KIO::Job *, KIO::filesize_t _total_size)
{
    mDebugIn << "= " << (unsigned long) _total_size << endl;
    PostMessage(SLV_TOTAL_SIZE, _total_size);

    PostMessage(SLV_CAN_RESUME, copyjob->getCanResume());
    PostMessage(SLV_CONNECTED);
    mDebugOut << endl;
}

void Slave::slotProcessedSize(KIO::Job *, KIO::filesize_t _processed_size)
{
    // mDebugIn<<endl;
    PostMessage(SLV_PROGRESS_SIZE, _processed_size);

    // mDebugOut<<endl;
}

void Slave::slotInfoMessage(KIO::Job *, const QString & _msg)
{
    mDebugIn << "MSG=" << _msg << endl;
    InfoMessage(_msg);
    mDebugOut << endl;
}

#include "slave.moc"
