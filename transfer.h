/***************************************************************************
*                                transfer.h
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


#ifndef _Transfer_h
#define _Transfer_h

#include <qlistview.h>
#include <qdatetime.h>
#include <qguardedptr.h>
#include <qmap.h>

#include <kurl.h>
#include <kio/jobclasses.h>

#include "slave.h"



class QTimer;


class KSimpleConfig;
class KAction;
class KRadioAction;



class DlgIndividual;
class TransferList;


class Transfer:public QObject, public QListViewItem
{


Q_OBJECT public:

    enum TransferMode { MD_QUEUED, MD_DELAYED, MD_SCHEDULED, MD_NONE };

    enum TransferStatus { ST_TRYING, ST_RUNNING, ST_STOPPED, ST_FINISHED };

    enum TransferOperation {
        OP_FINISHED, OP_RESUMED, OP_PAUSED, OP_REMOVED, OP_ABORTED,
        OP_QUEUED, OP_SCHEDULED, OP_DELAYED, OP_CONNECTED
    };



    Transfer(TransferList * view, const KURL & _src, const KURL & _dest);
    Transfer(TransferList * view, Transfer * after, const KURL & _src, const KURL & _dest);
    ~Transfer();

    Slave *m_pSlave;
    void copy(Transfer *);

    bool read(KSimpleConfig * config, int id);
    void write(KSimpleConfig * config, int id);
    void logMessage(const QString & message);


    QDateTime getStartTime()
    {
        return startTime;
    }
    QTime getRemainingTime()
    {
        return remainingTime;
    }

    unsigned long getTotalSize()
    {
        return totalSize;
    }
    unsigned long getProcessedSize()
    {
        return processedSize;
    }

    KURL getSrc()
    {
        return src;
    }
    KURL getDest()
    {
        return dest;
    }
    int getPercent()
    {
        return percent;
    }

    int getSpeed()
    {
        return speed;
    }
    TransferStatus getStatus()
    {
        return status;
    }
    int getMode()
    {
        return mode;
    }

    void setMode(TransferMode _mode)
    {
        mode = _mode;
    }
    void setStatus(TransferStatus _status)
    {
        status = _status;
    };
    void setStartTime(QDateTime _startTime)
    {
        startTime = _startTime;
    };
    void setSpeed(unsigned long _speed);

    // update methods
    void updateAll();
    bool updateStatus(int counter);

    void showIndividual();
    void UpdateRetry();


    // actions
    KAction *m_paResume, *m_paPause, *m_paDelete, *m_paRestart;
    //KAction *m_paDock;
    KRadioAction *m_paQueue, *m_paTimer, *m_paDelay;

    /**  */
public:
    void slotExecPause();
    void slotExecResume();
    void slotExecRemove();
    void slotExecDelay();
    void slotExecSchedule();
    void slotExecConnected();
    void slotExecAbort(const QString &);
    void slotCanResume(bool _bCanResume);
    void slotSpeed(unsigned long);
    /** No descriptions */
    bool isVisible();

public slots:
    // operation methods
    void slotResume();
    void slotRequestPause();
    void slotRequestRemove();
    void slotRequestSchedule();
    void slotRequestDelay();

    void slotRequestRestart();

    void slotUpdateActions();

    void slotQueue();
    void slotFinished();

    void slotTotalSize(unsigned long bytes);
    void slotProcessedSize(unsigned long);




    void slotStartTime(const QDateTime &);


signals:
    void statusChanged(Transfer *, int _operation);
    void log(uint, const QString &, const QString &);


private:
    void setupFields();

    KURL src;
    KURL dest;

    /* the tranfer id number */
    uint id;

    static uint idcount;

    // schedule time
    QDateTime startTime;

    unsigned long totalSize;
    unsigned long processedSize;
    int percent;


    int speed;
    QTime remainingTime;

    TransferStatus status;
    TransferMode mode;

    // how many times have we retried already
    unsigned int retryCount;

    bool canResume;

    TransferList *view;

    // individual download window
    DlgIndividual *dlgIndividual;



}

;


#endif                          // _Transfer_h
