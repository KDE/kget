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

#include <assert.h>
#include "settings.h"
#include "logwindow.h"
#include "kmainwidget.h"
#include "dlgIndividual.h"
#include "transferlist.h"
#include "transfer.h"

#include <kapplication.h>
#include <kio/passdlg.h>
#include <kio/global.h>


extern Settings ksettings;


Transfer::Transfer(TransferList * _view, const KURL & _src, const KURL & _dest):KListViewItem(_view)
{

    sDebugIn << endl;

    src = _src;
    dest = _dest;

    view = _view;
    setupFields();


    sDebugOut << endl;

}


Transfer::Transfer(TransferList * _view, Transfer * after, const KURL & _src, const KURL & _dest):KListViewItem(_view, (QListViewItem *) after)
{

    sDebugIn << endl;
    view = _view;
    src = _src;
    dest = _dest;
    setupFields();
    sDebugOut << endl;
}


Transfer::~Transfer()
{
    sDebugIn << endl;
    dlgIndividual->close();
    delete dlgIndividual;
    sDebugOut << endl;
}


void
Transfer::setupFields()
{
    sDebugIn << endl;


    totalSize = 0;
    processedSize = 0;
    percent = 0;
    id = 0;
    m_pSlave = new Slave(this, src, dest);
    m_pSlave->start();
    canResume = false;
    startTime = QDateTime::currentDateTime();
    speed = 0;
    // retryCount = ksettings.reconnectRetries-1;
    retryCount = 0;
    //first off all we need to know if resume is supported...

    status = ST_STOPPED;

    if (ksettings.b_addQueued)
        mode = MD_QUEUED;
    else
        mode = MD_DELAYED;



    connect(this, SIGNAL(statusChanged(Transfer *, int)), kmain, SLOT(slotStatusChanged(Transfer *, int)));
    connect(this, SIGNAL(statusChanged(Transfer *, int)), this, SLOT(slotUpdateActions()));

    connect(this, SIGNAL(log(uint, const QString &, const QString &)), kmain->logwin(), SLOT(logTransfer(uint, const QString &, const QString &)));

    // setup actions
    m_paResume = new KAction(i18n("&Resume"), QIconSet(QPixmap(locate("appdata", "pics/tool_resume.png"))), 0, this, SLOT(slotResume()), this, "resume");

    m_paPause = new KAction(i18n("&Pause"), QIconSet(QPixmap(locate("appdata", "pics/tool_pause.png"))), 0, this, SLOT(slotRequestPause()), this, "pause");

    m_paDelete = new KAction(i18n("&Delete"), QIconSet(QPixmap(locate("appdata", "pics/tool_delete.png"))), 0, this, SLOT(slotRequestRemove()), this, "delete");

    m_paRestart = new KAction(i18n("Re&start"), "tool_restart", 0, this, SLOT(slotRequestRestart()), this, "restart");

    m_paQueue = new KRadioAction(i18n("&Queue"), "tool_queue", 0, this, SLOT(slotQueue()), this, "queue");

    m_paTimer = new KRadioAction(i18n("&Timer"), "tool_timer", 0, this, SLOT(slotRequestSchedule()), this, "timer");

    m_paDelay = new KRadioAction(i18n("De&lay"), "tool_delay", 0, this, SLOT(slotRequestDelay()), this, "delay");

    m_paQueue->setExclusiveGroup("TransferMode");
    m_paTimer->setExclusiveGroup("TransferMode");
    m_paDelay->setExclusiveGroup("TransferMode");

    // Actions

    //        m_paDock = new KAction(i18n("&Dock"),"tool_dock.png", 0, this,SLOT(slotRequestDelay()), this, "dockIndividual");

    // setup individual transfer dialog
    dlgIndividual = new DlgIndividual(this);
    if (ksettings.b_iconifyIndividual) {
        // TODO : iconify in kwin
        // dlgIndividual->iconify( true );
    }

    sDebugOut << endl;
}



void Transfer::copy(Transfer * _orig)
{
    sDebugIn << endl;



    canResume = _orig->canResume;
    dest = _orig->dest;

    src = _orig->src;
    id = _orig->id;



    mode = _orig->mode;
    percent = _orig->percent;


    processedSize = _orig->processedSize;
    remainingTime = _orig->remainingTime;
    retryCount = _orig->retryCount;
    speed = _orig->speed;
    src = _orig->src;
    startTime = _orig->startTime;
    status = _orig->status;
    totalSize = _orig->totalSize;
    updateAll();

    sDebugOut << endl;
}



void Transfer::slotUpdateActions()
{
    sDebugIn << "the item Status is =" << status << "offline=" << ksettings.b_offlineMode << endl;
    //if we are in offlinemode just disable all actions  and return
    if (ksettings.b_offlineMode) {
        m_paResume->setEnabled(false);
        m_paPause->setEnabled(false);
        m_paRestart->setEnabled(false);
        dlgIndividual->update();
        return;
    }
    UpdateRetry();
    switch (status) {
    case ST_TRYING:
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

    if (isVisible())
        dlgIndividual->update();

    sDebugOut << endl;
}



void Transfer::setSpeed(unsigned long _speed)
{
    // sDebugIn <<endl;
    speed = _speed;

    remainingTime = KIO::calculateRemaining(totalSize, processedSize, speed);
    //sDebugOut <<endl;
}




void Transfer::updateAll()
{
    sDebugIn << endl;

    updateStatus(status);       // first phase of animation

    logMessage(i18n("Copy file from: %1").arg(src.url()));
    logMessage(i18n("To: %1").arg(dest.url()));

    // source
    setText(view->lv_url, src.prettyURL());

    // destination
    setText(view->lv_filename, dest.fileName());

    dlgIndividual->setCopying(src, dest);
    dlgIndividual->setCanResume(canResume);

    if (totalSize != 0) {
        //logMessage(i18n("Total size is %1 bytes").arg(totalSize));
        setText(view->lv_total, KIO::convertSize(totalSize));
    } else {
        //logMessage(i18n("Total size is unknown"));
        setText(view->lv_total, i18n("unknown"));
    }

    dlgIndividual->setTotalSize(totalSize);
    dlgIndividual->setPercent(0);
    dlgIndividual->setProcessedSize(0);

    sDebugOut << endl;
}


bool Transfer::updateStatus(int counter)
{
    //sDebug<< ">>>>Entering"<<endl;

    QPixmap *pix = 0L;
    bool isTransfer = false;

    if (status == ST_RUNNING) {
        pix = view->animConn.at(counter);
        isTransfer = true;
    } else if (status == ST_TRYING) {
        pix = view->animTry.at(counter);
        isTransfer = true;

    } else if (status == ST_STOPPED /*|| status==ST_PAUSED||status==ST_ABORTED */ ) {
        if (mode == MD_QUEUED) {
            pix = &view->pixQueued;
        } else if (mode == MD_SCHEDULED) {
            pix = &view->pixScheduled;
        } else {
            pix = &view->pixDelayed;
        }
    } else if (status == ST_FINISHED) {
        pix = &view->pixFinished;
    }

    setPixmap(view->lv_pixmap, *pix);

    //sDebug<< "<<<<Leaving"<<endl;
    return isTransfer;
}


void Transfer::UpdateRetry()
{
    QString retry;
    QString MaxRetry;

    retry.setNum(retryCount);
    MaxRetry.setNum(ksettings.reconnectRetries);
    retry += " / " + MaxRetry;

    setText(view->lv_count, retry);


}


void Transfer::slotResume()
{
    sDebugIn << " state =" << status << endl;

    retryCount++;
    if (retryCount > ksettings.reconnectRetries)
        ksettings.reconnectRetries = retryCount;
    UpdateRetry();
    assert(status == ST_STOPPED);

    sDebug << "src: " << src.url() << endl;
    sDebug << "dest " << dest.url() << endl;

    m_paResume->setEnabled(false);

    status = ST_TRYING;
    mode = MD_QUEUED;

    sDebug << "sending Resume to slave " << endl;
    m_pSlave->Op(Slave::RETR);

    sDebugOut << endl;
}




void Transfer::slotRequestPause()
{
    sDebugIn << endl;

    logMessage(i18n("Pausing"));

    assert(status <= ST_RUNNING);

    //stopping the thead

    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(false);



    m_pSlave->Op(Slave::PAUSE);
    sDebug << "Requesting Pause.." << endl;

    sDebugOut << endl;
}




void Transfer::slotRequestRestart()
{
    sDebugIn << endl;
    m_pSlave->Op(Slave::RESTART);
    slotSpeed(0);
    sDebugOut << endl;
}


void Transfer::slotRequestRemove()
{
    sDebugIn << endl;
    m_paDelete->setEnabled(false);
    m_paPause->setEnabled(false);
    dlgIndividual->close();

    if (status == ST_RUNNING) {
        m_pSlave->Op(Slave::REMOVE);
    } else
        emit statusChanged(this, OP_REMOVED);


    sDebugOut << endl;
}


void Transfer::slotQueue()
{
    sDebug << ">>>>Entering with mode = " << mode << endl;

    logMessage(i18n("Queueing"));

    assert(!(mode == MD_QUEUED));

    mode = MD_QUEUED;
    m_paQueue->setChecked(true);
    emit statusChanged(this, OP_QUEUED);
    sDebugOut << endl;
}


void Transfer::slotRequestSchedule()
{
    sDebugIn << endl;

    logMessage(i18n("Scheduling"));
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
    sDebugOut << endl;
}


void Transfer::slotRequestDelay()
{
    sDebugIn << endl;

    logMessage(i18n("Delaying"));

    assert(!(mode == MD_DELAYED));
    if (status == ST_RUNNING) {
        m_paPause->setEnabled(false);
        m_paRestart->setEnabled(false);
        m_pSlave->Op(Slave::DELAY);
    } else
        slotExecDelay();
    sDebugOut << endl;
}



/*
void Transfer::slotCanceled(KIO::Job *)
{
        sDebugIn  << endl;

        logMessage(i18n("Canceled by user"));
        emit statusChanged(this, OP_CANCELED);
        sDebugOut   << endl;
}

*/

void Transfer::slotFinished()
{
    sDebugIn << endl;

    logMessage(i18n("Download finished"));
    mode = MD_NONE;
    status = ST_FINISHED;
    slotProcessedSize(totalSize);

    slotSpeed(0);
    dlgIndividual->enableOpenFile();
    emit statusChanged(this, OP_FINISHED);
    sDebugOut << endl;
}




/*
void Transfer::slotRenaming(KIO::Job *, const KURL &, const KURL & to)
{
        sDebugIn  << endl;

        dest = to;

        logMessage(i18n("Renaming to %1").arg(dest.url().ascii()));

          // destination
          setText (view->lv_filename, dest.fileName ());

          dlgIndividual->setCopying (src, dest);

        sDebugOut   << endl;
}
        */




void Transfer::slotSpeed(unsigned long bytes_per_second)
{
    //sDebugIn <<endl;

    setSpeed(bytes_per_second);

    if (speed == 0 && status == ST_RUNNING) {
        setText(view->lv_speed, i18n("Stalled"));
        setText(view->lv_remaining, i18n("Stalled"));
    } else if (speed == 0 && status == ST_FINISHED) {

        setText(view->lv_progress, i18n("OK as in 'finished'","OK"));
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

    //sDebugOut<<endl;
}



void Transfer::slotTotalSize(unsigned long bytes)
{
#ifdef _DEBUG
    sDebugIn<<" totalSize is = "<<totalSize << endl;
#endif

    if (totalSize == 0) {
        totalSize = bytes;
        if (totalSize != 0) {
            logMessage(i18n("Total size is %1 bytes").arg(totalSize));
            setText(view->lv_total, KIO::convertSize(totalSize));
            dlgIndividual->setTotalSize(totalSize);
            dlgIndividual->setPercent(0);
            dlgIndividual->setProcessedSize(0);
        }
    } else {

#ifdef _DEBUG
        sDebug<<"totalSize="<<totalSize<<" bytes="<<bytes<<endl;
        assert(totalSize == bytes);
#endif
        if (totalSize != bytes)
            logMessage(i18n("The file size does not match!"));
        else
            logMessage(i18n("File Size checked"));
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}



void Transfer::slotProcessedSize(unsigned long bytes)
{
    //sDebug<< ">>>>Entering"<<endl;

    int old = percent;
    processedSize = bytes;

    if (totalSize == 0)
    {
        percent = 0;
    }
    else if ( totalSize < processedSize ) // bogus totalSize value
    {
        percent = 99; // what can we say?
        totalSize = processedSize;

        setText(view->lv_total, KIO::convertSize(totalSize));
        dlgIndividual->setTotalSize(totalSize);
    }
    else {
        percent = (int) (((float) processedSize / (float) totalSize) * 100.0);
    }
    dlgIndividual->setProcessedSize(processedSize);

    if (percent != old) {
        QString tmps;
        if (percent == 100) {
            tmps = i18n("OK as in 'finished'","OK");
        } else {
            tmps.setNum(percent);
        }

        setText(view->lv_progress, tmps);

        dlgIndividual->setPercent(percent);
    }
    //sDebug<< "<<<<Leaving"<<endl;
}




void Transfer::showIndividual()
{
    sDebugIn << endl;

    //      update the actions
    slotUpdateActions();
    //     then show the single dialog
    dlgIndividual->show();

    sDebugOut << endl;
}



void Transfer::logMessage(const QString & message)
{
    sDebugIn << message << endl;

    emit log(id, src.fileName(), message);
    dlgIndividual->addLog(message);

    sDebugOut << endl;
}



bool Transfer::read(KSimpleConfig * config, int id)
{
    sDebugIn << endl;


    QString str;
    str.sprintf("Item%d", id);
    config->setGroup(str);

    src = config->readEntry("Source", "");
    dest = config->readEntry("Dest", "");

    if (src.isEmpty() || dest.isEmpty()) {
        return false;
    }

    if (src.isMalformed() && !ksettings.b_expertMode) {
        KMessageBox::error(kmain, i18n("Malformed URL:\n") + src.url(), i18n("Error"));
        return false;
    }

    mode = (TransferMode) config->readNumEntry("Mode", MD_QUEUED);
    status = (TransferStatus) config->readNumEntry("Status", ST_RUNNING);
    startTime = config->readDateTimeEntry("ScheduledTime");
    canResume = config->readBoolEntry("CanResume", true);
    totalSize = config->readNumEntry("TotalSize", 0);
    processedSize = config->readNumEntry("ProcessedSize", 0);

    if (status != ST_FINISHED && totalSize != 0) {
        //TODO insert additional check
        status = ST_STOPPED;
    }

    updateAll();
    sDebugOut << endl;
    return true;
}


void Transfer::write(KSimpleConfig * config, int id)
{
    sDebugIn << endl;

    QString str;
    str.sprintf("Item%d", id);

    config->setGroup(str);
    config->writeEntry("Source", src.url());
    config->writeEntry("Dest", dest.url());
    config->writeEntry("Mode", mode);
    config->writeEntry("Status", status);
    config->writeEntry("CanResume", canResume);
    config->writeEntry("TotalSize", totalSize);
    config->writeEntry("ProcessedSize", processedSize);
    config->writeEntry("ScheduledTime", startTime);
    sDebugOut << endl;
}






/** No descriptions */
void Transfer::slotExecPause()
{

    sDebugIn << endl;
    slotSpeed(0);


    mode = MD_DELAYED;
    m_paDelay->setChecked(true);
    status = ST_STOPPED;

    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(true);
    m_paResume->setEnabled(true);
    slotUpdateActions();
    //TODO WE NEED TO UPDATE ACTIONS..
    kmain->slotUpdateActions();
    emit statusChanged(this, OP_PAUSED);
    sDebugOut << endl;

}

void Transfer::slotExecAbort(const QString & _msg)
{

    mode = MD_DELAYED;
    status = ST_STOPPED;
    slotSpeed(0);               //need???????
    QString tmps;

    tmps = "<code><font color=\"red\"> <strong>" + _msg + "</font> </strong></code><br/>";

    logMessage(tmps);

    if (ksettings.b_reconnectOnError) {
        retryCount++;
        if (retryCount == ksettings.reconnectRetries) { // no more retries
            if (retryCount > ksettings.reconnectRetries)
                ksettings.reconnectRetries = retryCount;
            status = ST_STOPPED;
            mode = MD_DELAYED;
        } else {
            status = ST_STOPPED;
            mode = MD_SCHEDULED;
            startTime = QDateTime::currentDateTime().addSecs(ksettings.reconnectTime * 60);
            logMessage(i18n("Attempt number %1").arg(retryCount));
        }
    } else {                    // if reconnecting is not enabled - simply set to delayed
        status = ST_STOPPED;
        mode = MD_DELAYED;
    }


    emit statusChanged(this, OP_ABORTED);

}

/** No descriptions */
void Transfer::slotExecRemove()
{
    sDebugIn << endl;

    //m_pFTP.wait();
    m_pSlave->wait();
    emit statusChanged(this, OP_REMOVED);
    sDebugOut << endl;

}


void Transfer::slotExecResume()
{
    sDebugIn << endl;
    emit statusChanged(this, OP_RESUMED);
    sDebugOut << endl;

}

void Transfer::slotExecConnected()
{
    sDebugIn << endl;
    status = ST_RUNNING;
    emit statusChanged(this, OP_CONNECTED);
    sDebugOut << endl;

}


void Transfer::slotCanResume(bool _bCanResume)
{
    sDebugIn << endl;


    canResume = _bCanResume;

    if (canResume) {
        logMessage(i18n("Download resumed"));
        setText(view->lv_resume, i18n("Yes"));
    } else {
        setText(view->lv_resume, i18n("No"));
    }

    dlgIndividual->setCanResume(canResume);

    sDebugOut << endl;

}


/** No descriptions */
void Transfer::slotExecDelay()
{
    sDebugIn << endl;

    mode = MD_DELAYED;
    status = ST_STOPPED;
    slotSpeed(0);               //need???????
    m_paDelay->setChecked(true);
    emit statusChanged(this, OP_DELAYED);

    sDebugOut << endl;

}

/** No descriptions */
void Transfer::slotExecSchedule()
{
    sDebugIn << endl;

    mode = MD_SCHEDULED;
    status = ST_STOPPED;
    m_paTimer->setChecked(true);
    emit statusChanged(this, OP_SCHEDULED);

    sDebugOut << endl;

}

/** No descriptions */
void Transfer::slotStartTime(const QDateTime & _startTime)
{
    sDebugIn << endl;

    setStartTime(_startTime);
    sDebugOut << endl;

}

/** return true if the dlgIndividual is Visible */
bool Transfer::isVisible()
{
    return dlgIndividual->isVisible();
}


#include "transfer.moc"

