/***************************************************************************
*                                transfer.cpp
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
 *   it under the terms of the GNU General Public Lkio/global.h:icense as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/

#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kwin.h>

#include <assert.h>
#include "safedelete.h"
#include "settings.h"
#include "logwindow.h"
#include "transferlist.h"
#include "transfer.h"
#include "scheduler.h"

#include <kapplication.h>
#include <kio/passdlg.h>
#include <kio/global.h>
#include <kio/netaccess.h>


Transfer::Transfer(Scheduler * _scheduler, const KURL & _src, const KURL & _dest, uint _id)
    : scheduler(_scheduler), slave(0), id(_id),
      src(_src), dest(_dest),
      totalSize(0), processedSize(0), percent(0), speed(0),  
      status(ST_STOPPED), priority(3), retryCount(0), canResume(false),
      delayTime(0)
{
    sDebugIn << endl;

    startTime = QDateTime::currentDateTime();
    
    logMessage(i18n("Copy file from: %1").arg(src.url()));
    logMessage(i18n("To: %1").arg(dest.url()));
    
    connect(this, SIGNAL(statusChanged(Transfer *, TransferMessage)), scheduler, SLOT(slotTransferMessage(Transfer *, TransferMessage)));
    connect(&timer, SIGNAL(timeout()), this, SLOT(slotUpdateDelay()));
    
    sDebugOut << endl;
}

Transfer::~Transfer()
{
    sDebugIn << endl;

    synchronousAbort();
    
    sDebugOut << endl;
}

void Transfer::setSpeed(unsigned long _speed)
{
    //FIXME: Implement me!!
}

bool Transfer::slotResume()
{
    sDebugIn << " state =" << status << endl;

    if(delayTime != 0)
        return false;
    
    if(!slave)
        slave = new Slave(this, src, dest);

    retryCount++;
/* FIXME reconnectRetries is only used here.
   This code modifies a config. variable!?, please check.
    if (retryCount > Settings::reconnectRetries())
        Settings::setReconnectRetries( retryCount );
*/
    assert(status == ST_STOPPED);

    sDebug << "src: " << src.url() << endl;
    sDebug << "dest " << dest.url() << endl;

    status = ST_TRYING;
    logMessage(i18n("Attempt number %1").arg(retryCount));

    sDebug << "sending Resume to slave " << endl;
    slave->Op(Slave::RETR);

    return true;
    
    sDebugOut << endl;
}

void Transfer::slotStop()
{
    sDebugIn << endl;

    logMessage(i18n("Pausing"));

    if(status != ST_TRYING && status != ST_RUNNING)
        return;
    
    //stopping the thread
    if(slave)
        {
        slave->Op(Slave::REMOVE);
        slave = 0;
    }
        
    
        
    sDebug << "Requesting Pause.." << endl;
    sDebugOut << endl;
}

void Transfer::slotRetransfer()
{
    sDebugIn << endl;
    if(!slave)
        slave = new Slave(this, src, dest);
    slave->Op(Slave::RESTART);
    speed = 0;
    sDebugOut << endl;
}


void Transfer::slotRemove()
{
    sDebugIn << endl;
 
    if ( status != ST_FINISHED )
    {
        // delete the partly downloaded file, if any
        KURL file = dest;
        file.setFileName( dest.fileName() + ".part" ); // ### get it from the job?
        if ( KIO::NetAccess::exists( file, false, 0 /*FIXME use a valid QWidget*/) )
        {
            SafeDelete::deleteFile( file ); // ### messagebox on failure?
        }
    }
    if (status == ST_RUNNING)
        slave->Op(Slave::REMOVE);
    else
        emit statusChanged(this, MSG_REMOVED);

    sDebugOut << endl;
}

void Transfer::slotDelay(int seconds)
{
    delayTime = seconds;
    timer.start(1000);
}


void Transfer::logMessage(const QString & message)
{
    sDebugIn << message << endl;

    emit log(id, src.fileName(), message);
    
    QString tmps = "<font color=\"blue\">" + QTime::currentTime().toString() + "</font> : " + message;

    transferLog.append(tmps + '\n');
    

    sDebugOut << endl;
}



bool Transfer::read(KSimpleConfig * config, int id)
{
    sDebugIn << endl;


    QString str;
    str.sprintf("Item%d", id);
    config->setGroup(str);

    if (src.isEmpty() || dest.isEmpty()) {
        return false;
    }

    if (!src.isValid() && !Settings::expertMode()) {
        //FIXME remove kmain! KMessageBox::error(kmain, i18n("Malformed URL:\n") + src.url(), i18n("Error"));
        return false;
    }

    status = (TransferStatus) config->readNumEntry("Status", ST_RUNNING);
    priority = config->readNumEntry("Priority", 3);
    group = config->readEntry("Group", "none");
    startTime = config->readDateTimeEntry("ScheduledTime");
    canResume = config->readBoolEntry("CanResume", true);
    totalSize = config->readNumEntry("TotalSize", 0);
    processedSize = config->readNumEntry("ProcessedSize", 0);

    if (status != ST_FINISHED && totalSize != 0) {
        //TODO insert additional check
        status = ST_STOPPED;
    }
    logMessage(i18n("Copy file from: %1").arg(src.url()));
    logMessage(i18n("To: %1").arg(dest.url()));
    
    sDebugOut << endl;
    return true;
}


void Transfer::write(KSimpleConfig * config, int id)
{
    sDebugIn << endl;

    QString str;
    str.sprintf("Item%d", id);

    config->setGroup(str);
    config->writePathEntry("Source", src.url());
    config->writePathEntry("Dest", dest.url());
    config->writeEntry("Priority", priority);
    config->writeEntry("Group", group);
    config->writeEntry("Status", status);
    config->writeEntry("CanResume", canResume);
    config->writeEntry("TotalSize", totalSize);
    config->writeEntry("ProcessedSize", processedSize);
    config->writeEntry("ScheduledTime", startTime);
    sDebugOut << endl;
}

bool Transfer::operator<(Transfer * transfer2)
{
    if(getPriority() < transfer2->getPriority())
        return true;
    return false;
}

void Transfer::about()
{
    kdDebug(DKGET) << "Transfer item: (prior= "<< getPriority() << ") "<< endl;
    kdDebug(DKGET) << "src  = "<< src.url() << endl;
    kdDebug(DKGET) << "dest = "<< dest.url() << endl;
}

void Transfer::slavePostMessage(Slave::SlaveResult event, unsigned long data)
{

    switch(event)
        {
        case Slave::SLV_PROGRESS_SIZE:
            slaveProcessedSize(data);
            emit statusChanged(this, MSG_UPD_PROGRESS);
            break;
        case Slave::SLV_PROGRESS_SPEED:
            speed = data;
            remainingTime = KIO::calculateRemaining(totalSize, processedSize, speed);
            emit statusChanged(this, MSG_UPD_SPEED);
            break;
        case Slave::SLV_RESUMED:
            emit statusChanged(this, MSG_RESUMED);
            break;
        case Slave::SLV_FINISHED:
            logMessage(i18n("Download finished"));
            status = ST_FINISHED;
            slaveProcessedSize(totalSize);
            speed = 0;
            sDebug << "DOWNLOAD FINISHED" << endl;
            emit statusChanged(this, MSG_FINISHED);
            break;
        case Slave::SLV_PAUSED:
            speed = 0;
            status = ST_STOPPED;
            emit statusChanged(this, MSG_PAUSED);
            break;
        case Slave::SLV_SCHEDULED:
            status = ST_STOPPED;
            emit statusChanged(this, MSG_SCHEDULED);
            break;
        case Slave::SLV_DELAYED:
            status = ST_STOPPED;
            speed = 0;
            emit statusChanged(this, MSG_DELAYED);
            break;
        case Slave::SLV_CONNECTED:
            status = ST_RUNNING;
            emit statusChanged(this, MSG_CONNECTED);
            break;
        case Slave::SLV_CAN_RESUME:
            canResume = (bool) data;
            emit statusChanged(this, MSG_CAN_RESUME);
            break;
        case Slave::SLV_TOTAL_SIZE:
            slaveTotalSize(data);
            emit statusChanged(this, MSG_TOTSIZE);
            break;
        case Slave::SLV_ERROR:
            status = ST_STOPPED;
            //startTime=QDateTime::currentDateTime().addSecs(Settings::reconnectTime() * 60); //FIXME give a look!
            emit statusChanged(this, MSG_ABORTED);
            break;
        case Slave::SLV_BROKEN:
            status = ST_STOPPED;
            emit statusChanged(this, MSG_ABORTED);
            break;
        case Slave::SLV_REMOVED:
            slave->wait();
            emit statusChanged(this, MSG_REMOVED);
            break;
        case Slave::SLV_KILLED:
            //FIXME IMPLEMENT ME!
            break;
        case Slave::SLV_INFO:
            //FIXME IMPLEMENT ME!
            break;
    }
}

void Transfer::slavePostMessage(Slave::SlaveResult /*event*/, const QString & /*msg*/)
{
    
}

void Transfer::slaveInfoMessage(const QString & msg)
{
    logMessage(msg);
}

void Transfer::slaveTotalSize(unsigned long bytes)
{
    sDebugIn<<" totalSize is = "<<totalSize << endl;

    if (totalSize == 0) 
        {
        totalSize = bytes;
        if (totalSize != 0) 
            logMessage(i18n("Total size is %1 bytes").arg(totalSize));
    } 
    else 
        {
        sDebug<<"totalSize="<<totalSize<<" bytes="<<bytes<<endl;
        
        if (totalSize != bytes)
            logMessage(i18n("The file size does not match."));
        else
            logMessage(i18n("File Size checked"));
    }
    sDebugOut << endl;
}



void Transfer::slaveProcessedSize(unsigned long bytes)
{
    if (totalSize == 0)
        percent = 0;
    else 
        if ( totalSize < bytes ) // bogus totalSize value
            {
            percent = 99; // what can we say?
            totalSize = bytes;
        }
        else 
            percent = (int) (((float) bytes / (float) totalSize) * 100.0);
}

void Transfer::synchronousAbort()
{
    if ( slave )
    {
        if ( slave->running() )
        {
            slave->Op(Slave::KILL);
            slave->wait();
        }

        if ( slave->running() )
            slave->terminate();

        delete slave;
        slave = 0L;

        status = ST_STOPPED;
    }
}

void Transfer::slotUpdateDelay()
{
    if(--delayTime == 0)
        {
        timer.stop();
        emit statusChanged(this, MSG_DELAY_FINISHED);
    }
    //sDebug << "tempo rimanente = " << delayTime << endl;
}

#include "transfer.moc"

