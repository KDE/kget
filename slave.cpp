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

#include "slave.h"
#include "transfer.h"

#include <assert.h>

/*
 * GetFileJob
 */

namespace KIO
{
    GetFileJob::GetFileJob(const KURL & m_src, const KURL & m_dest)
	: FileCopyJob(m_src, m_dest,-1, false, false, false, false) {}
	
    GetFileJob::~GetFileJob() {}
	
    /** Return true if the file has been resumed */
    bool GetFileJob::getCanResume()const
    {
	return m_canResume;
    }
}

/*
 * SlaveEvent
 */

#define EVENT_TYPE (QEvent::User + 252)

/*SlaveEvent::SlaveEvent(Transfer * _item, unsigned int _event, unsigned long _ldata)
    : QCustomEvent(EVENT_TYPE),
      m_event(_event),
      m_item(_item),
      m_ldate(_ldata),
      m_msg(QString(""))
{

}


SlaveEvent::SlaveEvent(Transfer * _item, unsigned int _event, const QString & _msg):QCustomEvent(EVENT_TYPE)
{
    m_event = _event;
    m_item = _item;
    m_ldata = 0L;
    m_msg = _msg;
}

unsigned int
SlaveEvent::getEvent() const
{
    return m_event;
}

Transfer *SlaveEvent::getItem() const
{
    return m_item;
}

unsigned long SlaveEvent::getData() const
{
    return m_ldata;
}

const QString & SlaveEvent::getMsg() const
{
    return m_msg;
}
*/


/*
 * Slave
 */

Slave::Slave(Transfer * _transfer, const KURL & _src, const KURL & _dest)
    : QObject(), QThread(),
      transfer(_transfer),
      m_src(_src), m_dest(_dest),
      nPendingCommand(0), copyjob(0)
      
{
    mDebug << ">>>>Entering" << endl;


    mDebug << ">>>>Leaving" << endl;
}

Slave::~Slave()
{}

void Slave::Op(SlaveCommand _cmd)
{
    mDebugIn << " _cmd = " << _cmd << endl;

    if ( !running() ) // start on demand
        start();

    mDebug << "AA" << endl;
                
    mutex.lock();
    mDebug << "BB" << endl;
    stack.push(_cmd);
    mDebug << "CC" << endl;
    nPendingCommand++;
    mDebug << "DD" << endl;
    worker.wakeOne();
    mDebug << "EE" << endl;
    mutex.unlock();

    mDebugOut << endl;
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
                mDebug << " FETCHED COMMAND      RESTART" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                // fall through
            case RETR:
                mDebug << " FETCHED COMMAND       RETR" << endl;
                KIO::Scheduler::checkSlaveOnHold( true );
                copyjob = new KIO::GetFileJob(m_src, m_dest);
                Connect();
                transfer->slavePostMessage(SLV_RESUMED);
                break;

            case PAUSE:
                mDebug << " FETCHED COMMAND       PAUSE" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                transfer->slavePostMessage(SLV_PAUSED);
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
                transfer->slavePostMessage(SLV_REMOVED);
                break;

            case SCHEDULE:
                mDebug << " FETCHED COMMAND       SCHEDULE" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                transfer->slavePostMessage(SLV_SCHEDULED);
                break;

            case DELAY:
                mDebug << " FETCHED COMMAND       DELAY" << endl;
                if (copyjob) {
                    copyjob->kill(true);
                    copyjob = 0L;
                }
                transfer->slavePostMessage(SLV_DELAYED);
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


    connect(copyjob, SIGNAL(connected(KIO::Job *)), SLOT(slotConnected(KIO::Job *)));
    connect(copyjob, SIGNAL(result(KIO::Job *)), SLOT(slotResult(KIO::Job *)));

    connect(copyjob, SIGNAL(totalSize(KIO::Job *, KIO::filesize_t)), SLOT(slotTotalSize(KIO::Job *, KIO::filesize_t)));

    connect(copyjob, SIGNAL(processedSize(KIO::Job *, KIO::filesize_t)), SLOT(slotProcessedSize(KIO::Job *, KIO::filesize_t)));

    connect(copyjob, SIGNAL(speed(KIO::Job *, unsigned long)), SLOT(slotSpeed(KIO::Job *, unsigned long)));

    connect(copyjob, SIGNAL(infoMessage(KIO::Job *, const QString &)), SLOT(slotInfoMessage(KIO::Job *, const QString &)));

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
    
    //assert(copyjob == job);
    copyjob=0L;
    KIO::Error error=KIO::Error(job->error());
    if (!error) {
        transfer->slavePostMessage(SLV_FINISHED);
    }
    else {
        QString tmsg="<font color=\"red\"> <b>" + job->errorString() + \
                     "</font></b>";
        transfer->slaveInfoMessage(tmsg);
        switch (error)
            {
            case KIO::ERR_COULD_NOT_LOGIN:
            case KIO::ERR_SERVER_TIMEOUT:
                //Timeout or login error
                transfer->slavePostMessage(SLV_ERROR);
                break;
            case KIO::ERR_CONNECTION_BROKEN:
                // Connection Broken
                transfer->slavePostMessage(SLV_BROKEN);
                break;            
            default:
                job->showErrorDialog();
                transfer->slavePostMessage(SLV_DELAYED);
                
                
        }
    }
    mDebugOut << endl;
}


void Slave::slotSpeed(KIO::Job *, unsigned long lSpeed)
{
    // mDebugIn<<endl;
    transfer->slavePostMessage(SLV_PROGRESS_SPEED, lSpeed);
    // mDebugOut<<endl;

}

void Slave::slotTotalSize(KIO::Job *, KIO::filesize_t _total_size)
{
    mDebugIn << "= " << (unsigned long) _total_size << endl;
    transfer->slavePostMessage(SLV_TOTAL_SIZE, _total_size);

    transfer->slavePostMessage(SLV_CAN_RESUME, copyjob->getCanResume());
    transfer->slavePostMessage(SLV_CONNECTED);
    mDebugOut << endl;
}

void Slave::slotProcessedSize(KIO::Job *, KIO::filesize_t _processed_size)
{
    // mDebugIn<<endl;
    transfer->slavePostMessage(SLV_PROGRESS_SIZE, _processed_size);

    // mDebugOut<<endl;
}

void Slave::slotInfoMessage(KIO::Job *, const QString & _msg)
{
    mDebugIn << "MSG=" << _msg << endl;
    transfer->slaveInfoMessage(_msg);
    mDebugOut << endl;
}

#include "slave.moc"
