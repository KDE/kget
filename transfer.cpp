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

#include <ftpsearch.h>
#include <kping.h>
#include <assert.h>
#include "settings.h"
#include "logwindow.h"
#include "kmainwidget.h"
#include "dlgIndividual.h"
#include "transferlist.h"
#include "transfer.h"

#include <kapp.h>
#include <kio/passdlg.h>
#include <kio/global.h>

//#include "main.h"


uint Transfer::idcount = 0;	// each new transfer will increase it

Transfer::Transfer(TransferList * _view, const KURL & _src,
                   const KURL & _dest):QListViewItem(_view)
{

        sDebug << ">>>>Entering" << endl;

        src = _src;
        dest = _dest;

        view = _view;
        setupFields();


        sDebug << "<<<<Leaving" << endl;

}


Transfer::Transfer(TransferList * _view, Transfer * after,
                   const KURL & _src,
                   const KURL & _dest):QListViewItem(_view,
                                                                     (QListViewItem *)
                                                                     after)
{

        sDebug << ">>>>Entering" << endl;
        view = _view;
        src = _src;
        dest = _dest;
        setupFields();
        sDebug << "<<<<Leaving" << endl;
}


Transfer::~Transfer()
{
        sDebug << ">>>>Entering" << endl;
        //delete dlgIndividual;
        sDebug << "<<<<Leaving" << endl;
}


void
Transfer::setupFields()
{
        sDebug << ">>>>Entering" << endl;

        if (src.protocol() == "ftp")
                m_pSlave = (Slave *) new SlaveFTP(this, src, dest);
        else
                if (src.protocol() == "http")
                        m_pSlave = (Slave *) new SlaveHTTP(this, src, dest);
                else			//protocol not supported
                        assert(0);

        totalSize = 0;
        processedSize = 0;
        percent = 0;

        totalFiles = 1;
        processedFiles = 0;

        canResume = false;
        startTime = QDateTime::currentDateTime();
        speed = 0;
        retryCount = ksettings.reconnectRetries-1;

        //first off all we need to know if resume is supported...

        status = ST_STOPPED;
        m_NextCmd = Slave::CHECK_RESUME;

        if (ksettings.b_addQueued)
                mode = MD_QUEUED;
        else
                mode = MD_DELAYED;

        id = ++idcount;

        connect(this, SIGNAL(statusChanged(Transfer *, int)), kmain,
                SLOT(slotStatusChanged(Transfer *, int)));
        connect(this, SIGNAL(statusChanged(Transfer *, int)), this,
                SLOT(slotUpdateActions(Transfer *, int)));

        connect(this, SIGNAL(log(uint, const QString &, const QString &)),
                kmain->logwin(),
                SLOT(logTransfer(uint, const QString &, const QString &)));

        // setup actions
        m_paResume =
                new KAction(i18n("&Resume"), "tool_resume", 0, this,
                            SLOT(slotResume()), this, "resume");

        m_paPause =
                new KAction(i18n("&Pause"), "tool_pause", 0, this,
                            SLOT(slotPause()), this, "pause");

        m_paDelete =
                new KAction(i18n("&Delete"), "tool_delete", 0, this,
                            SLOT(slotRemove()), this, "delete");

        m_paRestart =
                new KAction(i18n("Re&start"), "tool_restart", 0, this,
                            SLOT(slotRestart()), this, "restart");

        m_paQueue =
                new KRadioAction(i18n("&Queue"), "tool_queue", 0, this,
                                 SLOT(slotQueue()), this, "queue");

        m_paTimer =
                new KRadioAction(i18n("&Timer"), "tool_timer", 0, this,
                                 SLOT(slotSchedule()), this, "timer");

        m_paDelay =
                new KRadioAction(i18n("De&lay"), "tool_delay", 0, this,
                                 SLOT(slotDelay()), this, "delay");

        m_paQueue->setExclusiveGroup("TransferMode");
        m_paTimer->setExclusiveGroup("TransferMode");
        m_paDelay->setExclusiveGroup("TransferMode");

        // setup individual transfer dialog
        dlgIndividual = new DlgIndividual(this);
        if (ksettings.b_iconifyIndividual) {
                // TODO : iconify in kwin
                // dlgIndividual->iconify( true );
        }




        sDebug << "<<<<Leaving" << endl;
}



void Transfer::copy(Transfer * _orig)
{
        sDebug << ">>>>Entering" << endl;



        canResume = _orig->canResume;
        dest = _orig->dest;

        src = _orig->src;
        id = _orig->id;
        id = _orig->id;

        idcount = _orig->idcount;
        m_NextCmd = _orig->m_NextCmd;
        mode = _orig->mode;
        percent = _orig->percent;


        processedFiles = _orig->processedFiles;
        processedSize = _orig->processedSize;
        remainingTime = _orig->remainingTime;
        retryCount = _orig->retryCount;
        speed = _orig->speed;
        src = _orig->src;
        startTime = _orig->startTime;
        status = _orig->status;
        totalFiles = _orig->totalFiles;
        totalSize = _orig->totalSize;
        updateAll();

        sDebug << "<<<<Leaving" << endl;
}



void Transfer::slotUpdateActions()
{
        sDebug << ">>>>Entering" << endl;

	UpdateRetry();
        switch (status) {
        case ST_RUNNING:		
                m_paResume->setEnabled(false);
                m_paPause->setEnabled(true);
                m_paRestart->setEnabled(true);
                break;

        case ST_STOPPED:
                m_paResume->setEnabled(true);
                m_paPause->setEnabled(false);
                m_paRestart->setEnabled(false);
                break;
        case ST_FINISHED:
                m_paResume->setEnabled(false);
                m_paPause->setEnabled(false);
                m_paRestart->setEnabled(false);

                break;



        }

        // disable all signals
        m_paQueue->blockSignals(true);
        m_paTimer->blockSignals(true);
        m_paDelay->blockSignals(true);

        switch (mode) {
        case MD_QUEUED:
                m_paQueue->setChecked(true);
                break;
        case MD_SCHEDULED:
                m_paTimer->setChecked(true);
                break;
        case MD_DELAYED:
                m_paDelay->setChecked(true);
                break;
        case MD_NONE:
                m_paQueue->setChecked(false);
                m_paTimer->setChecked(false);
                m_paDelay->setChecked(false);

                m_paQueue->setEnabled(false);
                m_paTimer->setEnabled(false);
                m_paDelay->setEnabled(false);
                break;

        }
      

        // enable all signals
        m_paQueue->blockSignals(false);
        m_paTimer->blockSignals(false);
        m_paDelay->blockSignals(false);
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::setStartTime(QDateTime _startTime)
{
        sDebug << ">>>>Entering" << endl;
        startTime = _startTime;
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::setSpeed(unsigned long _speed)
{
        // sDebug<< ">>>>Entering"<<endl;
        speed = _speed;

        remainingTime =
                KIO::calculateRemaining(totalSize, processedSize, speed);
        //sDebug<< "<<<<Leaving"<<endl;
}



void Transfer::updateAll()
{
        sDebug << ">>>>Entering" << endl;


        updateStatus(status);	// first phase of animation

        slotCopying(src, dest);	//set destination and source

        if (canResume) {
                logMessage(i18n("Download can be resumed"));
                setText(view->lv_resume, i18n("Yes"));
        } else {
                logMessage(i18n("Download can not be resumed"));
                setText(view->lv_resume, i18n("No"));
        }



        dlgIndividual->setCanResume(canResume);



        if (totalSize != 0) {
                logMessage(i18n("Total size is %1 bytes").arg(totalSize));
                setText(view->lv_total, KIO::convertSize(totalSize));
                dlgIndividual->setTotalSize(totalSize);
                dlgIndividual->setPercent(0);
                dlgIndividual->setProcessedSize(0);
        } else {
                logMessage(i18n("Total size is unknown"));
                setText(view->lv_total, "unknow");
                dlgIndividual->setTotalSize(totalSize);
                dlgIndividual->setPercent(0);
                dlgIndividual->setProcessedSize(0);


        }


        sDebug << "<<<<Leaving" << endl;
}


bool Transfer::updateStatus(int counter)
{
        //sDebug<< ">>>>Entering"<<endl;

        QPixmap *pix = 0L;
        bool isTransfer = false;

        if (status <= ST_RUNNING) {
                pix = view->animConn->at(counter);
                isTransfer = true;
        }

        else if (status ==
                        ST_STOPPED /*|| status==ST_PAUSED||status==ST_ABORTED */ ) {
                if (mode == MD_QUEUED) {
                        pix = view->pixQueued;
                } else if (mode == MD_SCHEDULED) {
                        pix = view->pixScheduled;
                } else {
                        pix = view->pixDelayed;
                }
        } else if (status == ST_FINISHED) {
                pix = view->pixFinished;
        }

        setPixmap(view->lv_pixmap, *pix);

        //sDebug<< "<<<<Leaving"<<endl;
        return isTransfer;
}


void Transfer::slotSearch()
{
        sDebug << ">>>>Entering" << endl;
	//TODO remove not used
        sDebug << "<<<<Leaving" << endl;
}

void Transfer::UpdateRetry()
{
	QString retry;
	QString MaxRetry;
	retry.setNum(ksettings.reconnectRetries-retryCount);
	MaxRetry.setNum(ksettings.reconnectRetries);
	retry+=" / "+MaxRetry;
                      
	setText(view->lv_count,retry);


}


void Transfer::slotResume()
{
        sDebug << ">>>>Entering with state =" << status << endl;
        logMessage(i18n("Resuming"));
        //wait until the slave is ready....

        m_pSlave->wait();

	// check if the Max Count is reached
        if((retryCount<=0)||(retryCount>ksettings.reconnectRetries))
	  {
            retryCount=1;
	    ksettings.reconnectRetries++;
	    UpdateRetry(); 
 
	  }
        assert (status == ST_STOPPED);

        sDebug << "src: " << src.url() << endl;
        sDebug << "dest " << dest.url() << endl;
        m_paResume->setEnabled(false);

        status = ST_RUNNING;
        mode = MD_QUEUED;
        sDebug << "sending slave cmd= " << m_NextCmd << endl;
        m_pSlave->Op(m_NextCmd);
        m_pSlave->start();

        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotPause()
{
       sDebug << ">>>>Entering" << endl;

        logMessage(i18n("Pausing"));

        assert(status <= ST_RUNNING);

        //stopping the thead

        m_paPause->setEnabled(false);
        m_paRestart->setEnabled(false);



        m_pSlave->Op(Slave::PAUSE);
        sDebug << "Requesting Pause.." << endl;

        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotPauseOffline()
{
        sDebug << ">>>>Entering" << endl;
        //TODO remove not used
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotRestart()
{
        sDebug << ">>>>Entering" << endl;
	//TODO remove not used
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotRemove()
{
        sDebug << ">>>>Entering" << endl;
        m_paDelete->setEnabled(false);
        m_paPause->setEnabled(false);
        dlgIndividual->close();

        if (status == ST_RUNNING) {
                m_pSlave->Op(Slave::REMOVE);
             //TODO remove wait
                m_pSlave->wait();
        } else
                emit statusChanged(this, OP_REMOVED);


        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotQueue()
{
        sDebug << ">>>>Entering with mode = " << mode << endl;

        logMessage(i18n("Queueing"));

        assert(!(mode == MD_QUEUED));

        mode = MD_QUEUED;
        emit statusChanged(this, OP_QUEUED);
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotSchedule()
{
        sDebug << ">>>>Entering" << endl;

        logMessage(i18n("Scheduling"));
        sDebug << "...........the STATUS IS  " << status << endl;

        assert(!(mode == MD_SCHEDULED));

        // if the time was already set somewhere in the future, keep it
        // otherwise set it to the current time + 60 seconds
        if (startTime < QDateTime::currentDateTime()) {
                QDateTime dt = QDateTime::currentDateTime();
                startTime = dt.addSecs(60);
        }
        if (status == ST_RUNNING) {
                m_paPause->setEnabled(false);
                m_paRestart->setEnabled(false);

                m_pSlave->Op(Slave::SCHEDULE);

        } else
                slotExecSchedule();
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotDelay()
{
        sDebug << ">>>>Entering" << endl;

        logMessage(i18n("Delaying"));

        assert(!(mode == MD_DELAYED));
        if (status == ST_RUNNING) {
                m_paPause->setEnabled(false);
                m_paRestart->setEnabled(false);
                m_pSlave->Op(Slave::DELAY);
        } else
                slotExecDelay();
        sDebug << "<<<<Leaving" << endl;
}




void Transfer::slotCanceled(KIO::Job *)
{
        sDebug << ">>>>Entering" << endl;

        logMessage(i18n("Canceled by user"));
        emit statusChanged(this, OP_CANCELED);
        sDebug << "<<<<Leaving" << endl;
}



void Transfer::slotFinished()
{
        sDebug << ">>>>Entering" << endl;

        logMessage(i18n("Download finished"));
        mode = MD_NONE;
        if (ksettings.b_removeOnSuccess) {
                emit statusChanged(this, OP_FINISHED);
        } else {
                status = ST_FINISHED;
                slotProcessedSize(totalSize);
                slotSpeed(0);
                emit statusChanged(this, OP_FINISHED_KEEP);
        }


        sDebug << "<<<<Leaving" << endl;
}



void Transfer::slotCopying(const KURL & from, const KURL & to)
{
        sDebug << ">>>>Entering" << endl;

        src = from;
        dest = to;

        logMessage(i18n("Copying %1 to %2").arg(src.url().ascii()).
                   arg(dest.url().ascii()));

        // source
        setText(view->lv_url, src.url());

        // destination
        setText(view->lv_filename, dest.fileName());

        dlgIndividual->setCopying(src, dest);
        sDebug << "<<<<Leaving" << endl;
}




//TODO remove not used
void Transfer::slotRenaming(KIO::Job *, const KURL &, const KURL & to)
{
        sDebug << ">>>>Entering" << endl;

        dest = to;

        logMessage(i18n("Renaming to %1").arg(dest.url().ascii()));
        /*
          // destination
          setText (view->lv_filename, dest.fileName ());
         
          dlgIndividual->setCopying (src, dest);
        */
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::slotCanResume()
{
        sDebug << ">>>>Entering" << endl;
	//TODO remove not used
    sDebug << "<<<<Leaving" << endl;
}



void Transfer::slotSpeed(unsigned long bytes_per_second)
{
        //sDebug<< ">>>>Entering"<<endl;

        setSpeed(bytes_per_second);

        if (speed == 0 && status == ST_RUNNING) {
                setText(view->lv_speed, i18n("Stalled"));
                setText(view->lv_remaining, i18n("Stalled"));
        }
        
        else if (speed == 0 && status == ST_FINISHED) {

                setText(view->lv_progress, i18n("OK"));
                setText(view->lv_speed, i18n("0 MB/s"));
                setText(view->lv_remaining, i18n("00:00:00"));

        } else if (speed == 0 && status == ST_STOPPED) {

        
                setText(view->lv_speed, i18n("0 MB/s"));
                setText(view->lv_remaining, i18n("00:00:00"));

        } else {
                QString tmps = i18n("%1/s").arg(KIO::convertSize(speed));
                setText(view->lv_speed, tmps);
                setText(view->lv_remaining, remainingTime.toString());
        }

        dlgIndividual->setSpeed(speed, remainingTime);
        //sDebug<< "<<<<Leaving"<<endl;
}


void Transfer::slotTotalSize(unsigned long bytes)
{
        sDebug << ">>>>Entering" << endl;

        totalSize = bytes;
        if (totalSize != 0) {
                logMessage(i18n("Total size is %1 bytes").arg(totalSize));
                setText(view->lv_total, KIO::convertSize(totalSize));
                dlgIndividual->setTotalSize(totalSize);
                dlgIndividual->setPercent(0);
                dlgIndividual->setProcessedSize(0);
        } else {
                logMessage(i18n("Total size is unknow"));
                setText(view->lv_total, "unknow");
                dlgIndividual->setTotalSize(totalSize);
                dlgIndividual->setPercent(0);
                dlgIndividual->setProcessedSize(0);


        }


        //now we can download
        status = ST_STOPPED;
        m_NextCmd = Slave::RETR;

        emit statusChanged(this, OP_SIZE_CHECKED);

        sDebug << "<<<<Leaving" << endl;
}



void Transfer::slotProcessedSize(unsigned long bytes)
{
        //sDebug<< ">>>>Entering"<<endl;

        int old = percent;
        processedSize = bytes;

        if (totalSize == 0) {
                percent = 0;
        } else {
                percent =
                        (int) (((float) processedSize / (float) totalSize) * 100.0);
        }
        dlgIndividual->setProcessedSize(processedSize);

        if (percent != old) {
                QString tmps;
                if (percent == 100) {
                        tmps = i18n("OK");
                } else {
                        tmps.setNum(percent);
                }

                setText(view->lv_progress, tmps);

                dlgIndividual->setPercent(percent);
        }
        //sDebug<< "<<<<Leaving"<<endl;
}


void Transfer::slotTotalFiles(unsigned long files)
{
        sDebug << ">>>>Entering" << endl;

        totalFiles = files;

        logMessage(i18n("Total number of files is: %1").arg(totalFiles));

        QString tmps;
        tmps.sprintf("%d / %d", processedFiles, totalFiles);
        setText(view->lv_count, tmps);

        dlgIndividual->setTotalFiles(totalFiles);
        sDebug << "<<<<Leaving" << endl;

}


void Transfer::slotProcessedFiles(unsigned long files)
{
        sDebug << ">>>>Entering" << endl;

        processedFiles = files;

        logMessage(i18n("Processed number of files is: %1").
                   arg(processedFiles));

        QString tmps;
        tmps.sprintf("%d / %d", processedFiles, totalFiles);
        setText(view->lv_count, tmps);

        dlgIndividual->setProcessedFiles(processedFiles);
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::showIndividual()
{
        sDebug << ">>>>Entering" << endl;

        // show the single dialog

        dlgIndividual->show();
        sDebug << "<<<<Leaving" << endl;
}


void Transfer::logMessage(const QString & message)
{
        sDebug << ">>>>Entering " << message << endl;


        emit log(id, src.fileName(), message);
        dlgIndividual->addLog(message);

        sDebug << "<<<<Leaving" << endl;
}


bool Transfer::read(KSimpleConfig * config, int id)
{
        sDebug << ">>>>Entering" << endl;


        QString str;
        str.sprintf("Item%d", id);
        config->setGroup(str);

        src = config->readEntry
              ("Source", "");
        dest = config->readEntry
               ("Dest", "");

        if (src.isEmpty() || dest.isEmpty()) {
                return false;
        }

        if (src.isMalformed() && !ksettings.b_expertMode) {
                KMessageBox::error(kmain, i18n("Malformed URL:\n") + src.url(),
                                   i18n("Error"));
                return false;
        }

        mode = (TransferMode) config->readNumEntry
               ("Mode", MD_QUEUED);
        status = (TransferStatus) config->readNumEntry
                 ("Status", ST_RUNNING);
        startTime = config->readDateTimeEntry
                    ("ScheduledTime");
        canResume = config->readBoolEntry
                    ("CanResume", true);
        totalSize = config->readNumEntry
                    ("TotalSize", 0);
        totalFiles = config->readNumEntry
                     ("TotalFiles", 1);
        processedSize = config->readNumEntry
                        ("ProcessedSize", 0);
        processedFiles = config->readNumEntry
                         ("ProcessedFiles", 0);

        if (status != ST_FINISHED && totalSize != 0) {
                //TODO insert additional check
                status = ST_STOPPED;
        }

        updateAll();
        sDebug << "<<<<Leaving" << endl;
        return true;
}


void Transfer::write(KSimpleConfig * config, int id)
{
        sDebug << ">>>>Entering" << endl;

        QString str;
        str.sprintf("Item%d", id);

        config->setGroup(str);
        config->writeEntry
        ("Source", src.url());
        config->writeEntry
        ("Dest", dest.url());
        config->writeEntry
        ("Mode", mode);
        config->writeEntry
        ("Status", status);
        config->writeEntry
        ("CanResume", canResume);
        config->writeEntry
        ("TotalSize", totalSize);
        config->writeEntry
        ("ProcessedSize", processedSize);
        config->writeEntry
        ("TotalFiles", totalFiles);
        config->writeEntry
        ("ProcessedFiles", processedFiles);
        config->writeEntry
        ("ScheduledTime", startTime);
        sDebug << "<<<<Leaving" << endl;
}

int Transfer::getStatus()
{
        //sDebug<< ">>>>Entering"<< status << endl;
        return status;
}

void Transfer::setStatus(TransferStatus _status)
{
        //sDebug<< ">>>>Entering"<< status << endl;
        status = _status;
}





#include "transfer.moc"

/** No descriptions */
void Transfer::slotExecPause()
{

        sDebug << ">>>>Entering" << endl;
        //just to be sure that the thead as finished..
        m_pSlave->wait();
        slotSpeed(0);


        mode = MD_DELAYED;
        status = ST_STOPPED;

        m_paPause->setEnabled(false);
        m_paRestart->setEnabled(true);
        m_paResume->setEnabled(true);
        slotUpdateActions();
        //TODO WE NEED TO UPDATE ACTIONS..
        kmain->slotUpdateActions();
        emit statusChanged(this, OP_PAUSED);
        sDebug << "<<<<Leaving" << endl;

}

void Transfer::slotExecAbort(const QString & _msg)
{

        mode = MD_DELAYED;
        status = ST_STOPPED;
        slotSpeed(0);		//need???????
        QString tmps;

        tmps =
                "<code><font color=\"red\"> <strong>" + _msg +
                "</font> </strong></code><br/>";

        logMessage(tmps);

	if (ksettings.b_reconnectOnError)
	  {
	    --retryCount;
	    if (retryCount == 0)
	      {		// no more retries
		status = ST_STOPPED;
		mode = MD_DELAYED;
	      }
	    else
                         
	      {
		status = ST_STOPPED;
		mode = MD_SCHEDULED;
		startTime =QDateTime::currentDateTime ().addSecs (ksettings.reconnectTime* 60);
		logMessage (i18n ("Attempt number %1").arg (retryCount));
	      }
	  }
	else
	  {			// if reconnecting is not enabled - simply set to delayed
	    status = ST_STOPPED;
	    mode = MD_DELAYED;
	  }


        emit statusChanged(this, OP_ABORTED);

}

/** No descriptions */
void Transfer::slotExecRemove()
{
        sDebug << ">>>>Entering" << endl;

        //m_pFTP.wait();
        m_pSlave->wait();
        emit statusChanged(this, OP_REMOVED);
        sDebug << "<<<<Leaving" << endl;

}

void Transfer::slotExecResume()
{
        sDebug << ">>>>Entering" << endl;
        status = ST_RUNNING;
        emit statusChanged(this, OP_RESUMED);
        sDebug << "<<<<Leaving" << endl;

}

void Transfer::slotExecCanResume(bool _bCanResume)
{
        sDebug << ">>>>Entering" << endl;


        canResume = _bCanResume;

        if (canResume) {
                logMessage(i18n("Download can be resumed"));
                setText(view->lv_resume, i18n("Yes"));
        } else {
                logMessage(i18n("Download can not be resumed"));
                setText(view->lv_resume, i18n("No"));
        }

        dlgIndividual->setCanResume(canResume);

        //now we now if the server support resuming
        //then we can check the file size..
        if (ksettings.b_getSizes)
                m_NextCmd = Slave::CHECK_SIZE;
        else {

                m_NextCmd = Slave::RETR;
                slotTotalSize(0);

        }

        status = ST_STOPPED;
        emit statusChanged(this, OP_CAN_RESUME_CHECKED);
        sDebug << "<<<<Leaving" << endl;

}


/** No descriptions */
void Transfer::slotExecDelay()
{
        sDebug << ">>>>Entering" << endl;
        //m_pFTP.wait();
        mode = MD_DELAYED;
        status = ST_STOPPED;
        slotSpeed(0);		//need???????

        emit statusChanged(this, OP_DELAYED);
        sDebug << "<<<<Leaving" << endl;

}

/** No descriptions */
void Transfer::slotExecSchedule()
{
        sDebug << ">>>>Entering" << endl;
        //m_pFTP.wait();
        mode = MD_SCHEDULED;
        status = ST_STOPPED;
        emit statusChanged(this, OP_SCHEDULED);
        sDebug << "<<<<Leaving" << endl;

}

/** No descriptions */
void Transfer::slotStartTime(const QDateTime & _startTime)
{
        sDebug << ">>>>Entering" << endl;

        setStartTime(_startTime);
        sDebug << "<<<<Leaving" << endl;

}

/** No descriptions */
void Transfer::SlotExecLoginInfo()
{

        QString user = getUser();
        QString pass = getPass();
        m_pSlave->wait();

        setStatus(ST_STOPPED);
        setMode(MD_NONE);

        KIO::PasswordDialog * passDialog = new KIO::PasswordDialog( "Please Enter the login and Password:",user,kmain);
        bool * bVal=false;
        int retry
	  = passDialog->getNameAndPassword(user, pass,bVal,
					   "Please Enter the login and Password:");

        if (retry
           ) {
                setNameAndPasswork(user, pass);

                slotResume();
        }
        else
                slotDelay();

        sDebug << "new Login =" << user << " new passw=" << pass << endl;

}

/** No descriptions */
void Transfer::ResumeStatus()
{
  //TODO remove not used
}
