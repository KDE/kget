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



#include "slave.h"


Slave::Slave(Transfer * _parent, const KURL & _src, const KURL & _dest)
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

        postEvent(kapp->mainWidget(), (QEvent *) e1);

}

void Slave::PostMessage(SlaveResult _event, const QString & _msg)
{

        SlaveEvent *e1 = new SlaveEvent(m_parent, _event, _msg);

        postEvent(kapp->mainWidget(), (QEvent *) e1);
        mDebug << "Msg:" << "_msg = " << _msg << endl;

}

void Slave::InfoMessage(const QString & _msg)
{
        SlaveEvent *e1 = new SlaveEvent(m_parent, SLV_INFO, _msg);

        postEvent(kapp->mainWidget(), (QEvent *) e1);
        mDebug << "Infor Msg:" << "_msg = " << _msg << endl;



}



void Slave::run()
{

        mDebugIn << endl;

        SlaveCommand cmd;
        bool running = true;

        while (running) {
                if (!nPendingCommand)
                        worker.wait();
                switch (cmd = fetch_cmd()) {
                case RESTART:
                        copyjob->kill(true);
                case RETR:
                        mDebug << " FETCHED COMMAND       RETR" << endl;
                        copyjob = new KIO::GetFileJob(m_src, m_dest);
                        copyjob->setAutoErrorHandlingEnabled(true);
                        Connect();
                        PostMessage(SLV_RESUMED);
                        break;

                case PAUSE:
                        mDebug << " FETCHED COMMAND       PAUSE" << endl;
                        copyjob->kill(true);
                        PostMessage(SLV_PAUSED);
                        break;

                case REMOVE:
                        mDebug << " FETCHED COMMAND       REMOVE" << endl;
                        running = false;
                        copyjob->kill(true);

                        copyjob = 0L;
                        PostMessage(SLV_REMOVED);
                        break;

                case SCHEDULE:
                        mDebug << " FETCHED COMMAND       SCHEDULE" << endl;
                        copyjob->kill(true);
                        copyjob = 0L;
                        PostMessage(SLV_SCHEDULED);
                        break;
                case DELAY:
                        mDebug << " FETCHED COMMAND       DELAY" << endl;
                        copyjob->kill(true);
                        copyjob = 0L;
                        PostMessage(SLV_DELAYED);
                        break;

                default: {
                                mDebug << " UNKNOW COMMAND DIE...." << endl;
                                assert(0);
                        }
                }


        }




        copyjob = NULL;
        mDebugOut << endl;



}





/** No descriptions */
Slave::SlaveCommand Slave::fetch_cmd()
{
        mutex.lock();
        nPendingCommand--;
        SlaveCommand cmd = stack.pop();

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
}/** No descriptions */


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

/** No descriptions */
void Slave::slotResult(KIO::Job * job)
{
        mDebugIn << endl;
        if (job->error()) {
                InfoMessage(job->errorString());
                PostMessage(SLV_DELAYED);
        } else
                PostMessage(SLV_FINISHED);
        mDebugOut << endl;

}


/** No descriptions */
void Slave::slotSpeed(KIO::Job *, unsigned long lSpeed)
{
        // mDebugIn<<endl;
        PostMessage(SLV_PROGRESS_SPEED, lSpeed);
        // mDebugOut<<endl;

}

/** No descriptions */
void Slave::slotTotalSize(KIO::Job *, KIO::filesize_t _total_size)
{
        mDebugIn << "= " << (unsigned long) _total_size << endl;
        PostMessage(SLV_TOTAL_SIZE, _total_size);

        PostMessage(SLV_CAN_RESUME, copyjob->getCanResume());
        PostMessage(SLV_CONNECTED);
        mDebugOut << endl;
}

/** No descriptions */
void Slave::slotProcessedSize(KIO::Job *, KIO::filesize_t _processed_size)
{
        // mDebugIn<<endl;
        PostMessage(SLV_PROGRESS_SIZE, _processed_size);

        // mDebugOut<<endl;
}

/** No descriptions */
void Slave::slotInfoMessage(KIO::Job *, const QString & _msg)
{
        mDebugIn << "MSG=" << _msg << endl;
        InfoMessage(_msg);
        mDebugOut << endl;
}


#include "slave.moc"
