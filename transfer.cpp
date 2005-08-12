/***************************************************************************
*                                transfer.cpp
*                             -------------------
*
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002, 2003, 2004, 2005 by Patrick Charbonnier
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

#include <qheader.h>

#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kaction.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <kmessagebox.h>

#include <assert.h>
#include "safedelete.h"
#include "settings.h"
#include "logwindow.h"
#include "kmainwidget.h"
#include "dlgIndividual.h"
#include "transferlist.h"
#include "transfer.h"

#include <kapplication.h>
#include <kio/passdlg.h>
#include <kio/global.h>
#include <kio/netaccess.h>


extern Settings ksettings;


Transfer::Transfer(TransferList * _view, const KURL & _src, const KURL & _dest, const uint _id)
    : QObject( _view ),
      KListViewItem(_view),
      dlgIndividual( 0 )
{
    sDebugIn << endl;

    src = _src;
    dest = _dest;

    view = _view;
    init(_id);

    sDebugOut << endl;
}


Transfer::Transfer(TransferList * _view, Transfer * after, const KURL & _src, const KURL & _dest, const uint _id)
    : QObject( _view ),
      KListViewItem(_view, (QListViewItem *) after),
      src(_src), dest(_dest), view(_view),
      dlgIndividual( 0 )
{
    sDebugIn << endl;

    view = _view;
    src = _src;
    dest = _dest;
    init(_id);

    sDebugOut << endl;
}


Transfer::~Transfer()
{
    sDebugIn << endl;

    synchronousAbort();
    delete dlgIndividual;

    sDebugOut << endl;
}


void
Transfer::init(const uint _id)
{
    sDebugIn << endl;
    remainingTimeSec = 0;
    totalSize = 0;
    processedSize = 0;
    percent = 0;
    id = _id;
    m_pSlave = new Slave(this, src, dest);
    canResume = false;
    startTime = QDateTime::currentDateTime();
    speed = 0;
    // retryCount = ksettings.reconnectRetries-1;
    retryCount = 0;
    //first off all we need to know if resume is supported...

    status = ST_STOPPED;


    connect(this, SIGNAL(statusChanged(Transfer *, int)), kmain, SLOT(slotStatusChanged(Transfer *, int)));
    connect(this, SIGNAL(statusChanged(Transfer *, int)), this, SLOT(slotUpdateActions()));

    connect(this, SIGNAL(log(uint, const QString &, const QString &)), kmain->logwin(), SLOT(logTransfer(uint, const QString &, const QString &)));

    // setup actions
    m_paResume = new KAction(i18n("&Resume"), "tool_resume", 0, this, SLOT(slotResume()), this, "resume");

    m_paPause = new KAction(i18n("&Pause"), "tool_pause", 0, this, SLOT(slotRequestPause()), this, "pause");

    m_paDelete = new KAction(i18n("&Delete"), "editdelete", 0, this, SLOT(slotRequestRemove()), this, "delete");

    m_paRestart = new KAction(i18n("Re&start"), "tool_restart", 0, this, SLOT(slotRequestRestart()), this, "restart");

    m_paQueue = new KRadioAction(i18n("&Queue"), "tool_queue", 0, this, SLOT(slotQueue()), this, "queue");

    m_paTimer = new KRadioAction(i18n("&Timer"), "tool_timer", 0, this, SLOT(slotRequestSchedule()), this, "timer");

    m_paDelay = new KRadioAction(i18n("De&lay"), "tool_delay", 0, this, SLOT(slotRequestDelay()), this, "delay");

    m_paQueue->setExclusiveGroup("TransferMode");
    m_paTimer->setExclusiveGroup("TransferMode");
    m_paDelay->setExclusiveGroup("TransferMode");

    // Actions

    // m_paDock = new KAction(i18n("&Dock"),"tool_dock", 0, this,SLOT(slotRequestDelay()), this, "dockIndividual");

    // setup individual transfer dialog

    mode = MD_NEW;

    sDebugOut << endl;
}


void Transfer::synchronousAbort()
{
    if ( m_pSlave )
    {
        if ( m_pSlave->running() )
        {
            m_pSlave->Op(Slave::KILL);
            m_pSlave->wait();
        }

        if ( m_pSlave->running() )
            m_pSlave->terminate();

        delete m_pSlave;
        m_pSlave = 0L;

        status = ST_STOPPED;
        slotUpdateActions();
    }

}

void Transfer::slotUpdateActions()
{
    sDebugIn << "the item Status is =" << status << "offline=" << ksettings.b_offline << endl;
     //if we are offline just disable Resume and Pause and return
    if (ksettings.b_offline) {
        m_paResume->setEnabled(false);
        m_paPause->setEnabled(false);
        m_paRestart->setEnabled(false);
        if(dlgIndividual)
			dlgIndividual->update();
        return;
    }

    UpdateRetry();

    switch (status) {

    case ST_TRYING://fall-through
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
    case MD_NEW: //fall through
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

    if (dlgIndividual)
        dlgIndividual->update();

    sDebugOut << endl;
}



void Transfer::setSpeed(unsigned long _speed)
{
    // sDebugIn <<endl;
    speed = _speed;

    remainingTimeSec = KIO::calculateRemainingSeconds(totalSize, processedSize, speed);
    remainingTime = KIO::convertSeconds(remainingTimeSec);
    //sDebugOut <<endl;
}



void Transfer::updateAll()
{
    sDebugIn << endl;

    updateStatus(status);       // first phase of animation

    logMessage(i18n("Copy file from: %1").arg(src.prettyURL()));
    logMessage(i18n("To: %1").arg(dest.prettyURL()));

    // source
    setText(view->lv_url, src.prettyURL());

    // destination
    setText(view->lv_filename, dest.fileName());

    if(dlgIndividual)
		{
		dlgIndividual->setCopying(src, dest);
    	dlgIndividual->setCanResume(canResume);
		dlgIndividual->setTotalSize(totalSize);
		dlgIndividual->setPercent(0);
		dlgIndividual->setProcessedSize(0);
	}

    if (totalSize != 0) {
        //logMessage(i18n("Total size is %1 bytes").arg((double)totalSize));
        setText(view->lv_total, KIO::convertSize(totalSize));
    } else {
        //logMessage(i18n("Total size is unknown"));
        setText(view->lv_total, i18n("unknown"));
    }


    sDebugOut << endl;
}


bool Transfer::updateStatus(int counter)
{
    //sDebug<< ">>>>Entering"<<endl;

    QPixmap *pix = 0L;
    bool isTransfer = false;

    view->setUpdatesEnabled(false);

    switch(status)
        {
        case ST_RUNNING:
            pix = view->animConn.at(counter);
            isTransfer = true;
            break;
        case ST_TRYING:
            pix = view->animTry.at(counter);
            isTransfer = true;
            break;
        case ST_STOPPED:
            if(mode == MD_QUEUED)
                pix = &view->pixQueued;
            else if(mode == MD_SCHEDULED)
                pix = &view->pixScheduled;
            else
                pix = &view->pixDelayed;
            break;
        case ST_FINISHED:
            pix = &view->pixFinished;
    }

    setPixmap(view->lv_pixmap, *pix);
    view->setUpdatesEnabled(true);

    if(prevStatus!=status || prevMode != mode || status==ST_RUNNING || status==ST_TRYING)
        {
        QRect rect = view->header()->sectionRect(view->lv_pixmap);

        int x = rect.x();
        int y = view->itemRect(this).y();
        int w = rect.width();
        int h = rect.height();

        view->QScrollView::updateContents(x,y,w,h);

        prevStatus = status;
       prevMode = mode;
    }

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
        retryCount = 1;
    UpdateRetry();
    assert(status == ST_STOPPED);

    sDebug << "src: " << src.prettyURL() << endl;
    sDebug << "dest " << dest.prettyURL() << endl;

    m_paResume->setEnabled(false);

    status = ST_TRYING;
    mode = MD_QUEUED;
    logMessage(i18n("Attempt number %1").arg(retryCount));

    sDebug << "sending Resume to slave " << endl;
    m_pSlave->Op(Slave::RETR);

    sDebugOut << endl;
}


 void Transfer::slotStop()
{
    sDebugIn << endl;

    logMessage(i18n("Stopping"));

    assert(status <= ST_RUNNING && ksettings.b_offline);

    m_pSlave->Op(Slave::KILL); // KILL doesn't post a Message
    sDebug << "Killing Slave" << endl;

    slotSpeed(0);
    mode = MD_QUEUED;
    status=ST_STOPPED;
    m_paQueue->setChecked(true);

    slotUpdateActions();

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
    if (dlgIndividual && !ksettings.b_expertMode)
    {
        if (KMessageBox::warningContinueCancel(0, i18n("Are you sure you want to delete this transfer?"),
                                           QString::null, KStdGuiItem::del(),
                                           QString("delete_transfer"))
            != KMessageBox::Continue)
            return;
    }
    m_paDelete->setEnabled(false);
    m_paPause->setEnabled(false);
    if (dlgIndividual)
        dlgIndividual->close();


    if ( status != ST_FINISHED )
    {
        KURL file = dest;
        // delete the partly downloaded file, if any
        file.setFileName( dest.fileName() + ".part" ); // ### get it from the job?
  
        if ( KIO::NetAccess::exists( file, false, view ) ) // don't pollute user with warnings
        {
            SafeDelete::deleteFile( file ); // ### messagebox on failure?
        }
    }
    if (status == ST_RUNNING)
        m_pSlave->Op(Slave::REMOVE);
    else
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
    if(dlgIndividual)
		dlgIndividual->enableOpenFile();
    emit statusChanged(this, OP_FINISHED);
    sDebugOut << endl;
}




/*
void Transfer::slotRenaming(KIO::Job *, const KURL &, const KURL & to)
{
        sDebugIn  << endl;

        dest = to;

        logMessage(i18n("Renaming to %1").arg(dest.prettyURL().ascii()));

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
        if(dlgIndividual)
            dlgIndividual->setSpeed(i18n("Stalled"));
    } else if (speed == 0 && status == ST_FINISHED) {

        setText(view->lv_progress, i18n("OK as in 'finished'","OK"));
        setText(view->lv_speed, i18n("Finished"));
        setText(view->lv_remaining, i18n("Finished"));
        if(dlgIndividual)
            dlgIndividual->setSpeed(i18n("Finished"));

    } else if (speed == 0 && status == ST_STOPPED) {


        setText(view->lv_speed, i18n("Stopped"));
        setText(view->lv_remaining, i18n("Stopped"));
        if(dlgIndividual)
            dlgIndividual->setSpeed(i18n("Stopped"));

    } else {
        QString tmps = i18n("%1/s").arg(KIO::convertSize(speed));
        setText(view->lv_speed, tmps);
        setText(view->lv_remaining, remainingTime);
        if(dlgIndividual)
            dlgIndividual->setSpeed(tmps + " ( " + remainingTime  + " ) ");
    }

    //sDebugOut<<endl;
}



void Transfer::slotTotalSize(KIO::filesize_t bytes)
{
#ifdef _DEBUG
    sDebugIn<<" totalSize is = "<<totalSize << endl;
#endif

    if (totalSize == 0) {
        totalSize = bytes;
        if (totalSize != 0) {
            logMessage(i18n("Total size is %1 bytes").arg((double)totalSize,0,'f',0));
            setText(view->lv_total, KIO::convertSize(totalSize));
			if(dlgIndividual)
				{
				dlgIndividual->setTotalSize(totalSize);
				dlgIndividual->setPercent(0);
				dlgIndividual->setProcessedSize(0);
			}
        }
    } else {

#ifdef _DEBUG
        sDebug<<"totalSize="<<totalSize<<" bytes="<<bytes<<endl;
        assert(totalSize == bytes);
#endif
        if (totalSize != bytes)
            logMessage(i18n("The file size does not match."));
        else
            logMessage(i18n("File Size checked"));
    }

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}



void Transfer::slotProcessedSize(KIO::filesize_t bytes)
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
		if(dlgIndividual)
        	dlgIndividual->setTotalSize(totalSize);
    }
    else {
        percent = (int) (((float) processedSize / (float) totalSize) * 100.0);
    }
	if(dlgIndividual)
    	dlgIndividual->setProcessedSize(processedSize);

    if (percent != old) {
        QString tmps;
        if (percent == 100) {
            tmps = i18n("OK as in 'finished'","OK");
        } else {
            tmps.setNum(percent);
        }

        setText(view->lv_progress, tmps);

		if(dlgIndividual)
        	dlgIndividual->setPercent(percent);
    }
    //sDebug<< "<<<<Leaving"<<endl;
}




void Transfer::showIndividual()
{
    sDebugIn << endl;

    //    create a DlgIndividual only if it hasn't been created yet
    if(!dlgIndividual)
    {
        dlgIndividual = new DlgIndividual(this);
        dlgIndividual->setLog(transferLog);
        dlgIndividual->setCopying(src, dest);
        dlgIndividual->setCanResume(canResume);
        dlgIndividual->setTotalSize(totalSize);
        dlgIndividual->setPercent(percent);
        dlgIndividual->setProcessedSize(processedSize);
    }

    dlgIndividual->raise();


    if (ksettings.b_iconifyIndividual) {
        KWin::iconifyWindow( dlgIndividual->winId() );
    }

    //      update the actions
    slotUpdateActions();
    //     then show the single dialog
    KWin::deIconifyWindow( dlgIndividual->winId() );
    dlgIndividual->show();

    sDebugOut << endl;
}


void Transfer::logMessage(const QString & message)
{
    sDebugIn << message << endl;

    emit log(id, src.fileName(), message);

    QString tmps = "<font color=\"blue\">" + QTime::currentTime().toString() + "</font> : " + message;

    transferLog.append(tmps + '\n');

    if(dlgIndividual)
        dlgIndividual->appendLog(tmps);

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

    if (!src.isValid() && !ksettings.b_expertMode) {
        KMessageBox::error(kmain, i18n("Malformed URL:\n") + src.prettyURL(), i18n("Error"));
        return false;
    }

    mode = (TransferMode) config->readNumEntry("Mode", MD_QUEUED);
    status = (TransferStatus) config->readNumEntry("Status", ST_RUNNING);
    startTime = config->readDateTimeEntry("ScheduledTime");
    canResume = config->readBoolEntry("CanResume", true);
    totalSize = config->readUnsignedNum64Entry("TotalSize", 0);
    processedSize = config->readUnsignedNum64Entry("ProcessedSize", 0);

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
    config->writePathEntry("Source", src.url());
    config->writePathEntry("Dest", dest.url());
    config->writeEntry("Mode", mode);
    config->writeEntry("Status", status);
    config->writeEntry("CanResume", canResume);
    config->writeEntry("TotalSize", totalSize );
    config->writeEntry("ProcessedSize", processedSize );
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

void Transfer::slotExecError()
{
    sDebugIn << endl;

    status = ST_STOPPED;
    mode = MD_SCHEDULED;
    startTime=QDateTime::currentDateTime().addSecs(ksettings.reconnectTime * 60);
    emit statusChanged(this, OP_SCHEDULED);

    sDebugOut << endl;
}

void Transfer::slotExecBroken()
{
    sDebugIn << endl;

    status = ST_STOPPED;
    mode = MD_QUEUED;
    emit statusChanged(this, OP_QUEUED);

    sDebugOut << endl;
}


void Transfer::slotExecRemove()
{
    sDebugIn << endl;

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
    if (mode == MD_NEW)
    {
        if (ksettings.b_offline)// when we're offline and arrive here, then the file is in cache
            return;             // Slave::slotResult will be called immediately, so we do nothing here
        status = ST_STOPPED;
        m_pSlave->Op(Slave::KILL);
        if (ksettings.b_addQueued)
        {
            mode = MD_QUEUED;
            emit statusChanged(this, OP_QUEUED);
        }
        else
        {
            mode = MD_DELAYED;
            emit statusChanged(this, OP_DELAYED);
        }
        return;
    }
    
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

    //dlgIndividual->setCanResume(canResume);

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
bool Transfer::isVisible() const
{
    return dlgIndividual ? dlgIndividual->isVisible() : false;
}

bool Transfer::keepDialogOpen() const
{
    return dlgIndividual ? dlgIndividual->keepDialogOpen() : false;
}

void Transfer::maybeShow()
{
    if ( ksettings.b_showIndividual && getStatus() != Transfer::ST_FINISHED )
    {
        if(dlgIndividual)
            dlgIndividual->show();
    }
}

bool Transfer::retryOnError()
{
    return (ksettings.b_reconnectOnError && (retryCount < ksettings.reconnectRetries));
}

bool Transfer::retryOnBroken()
{
    return (ksettings.b_reconnectOnBroken && (retryCount < ksettings.reconnectRetries));
}

void Transfer::checkCache()
{
    assert (mode == MD_NEW);

    if (src.protocol()=="http")
    {
        status = ST_TRYING;
        m_pSlave->Op(Slave::RETR_CACHE);
    }
    else
        NotInCache();
}

void Transfer::NotInCache()
{
    logMessage(i18n("checking if file is in cache...no"));
    if (ksettings.b_addQueued)
        mode = MD_QUEUED;
    else
        mode = MD_DELAYED;
    status = ST_STOPPED;
}
#include "transfer.moc"

