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


#ifndef _TRANSFER_H
#define _TRANSFER_H

#include <qtimer.h>
#include <qdatetime.h>
#include <qguardedptr.h>
#include <qmap.h>

#include <kurl.h>
#include <kio/jobclasses.h>

#include "slave.h"
#include "globals.h"


class KSimpleConfig;
class KAction;
class KRadioAction;



class Scheduler;
class TransferList;


class Transfer : public QObject
{
Q_OBJECT

friend class Scheduler;
friend class Slave;
friend class TransferList;

public:

    //TransferStatus defined in globals.h
    //TransferMessage defined in globals.h
    //TransferCommand defined in globals.h

        
    Transfer(Scheduler * _scheduler, const KURL & _src, const KURL & _dest, uint _id=0);
    ~Transfer();

    /**
     * These functions are used to get informations about the transfer
     */
    KURL getSrc()const {return src;}
    KURL getDest()const {return dest;}
    unsigned long getTotalSize()const {return totalSize;}
    unsigned long getProcessedSize()const {return processedSize;}
    int getPercent()const {return percent;}
    int getPriority()const {return priority;}
    QString getGroup()const {return group;}
    QDateTime getStartTime()const {return startTime;}
    QTime getRemainingTime()const {return remainingTime;}
    int getSpeed()const {return speed;}
    TransferStatus getStatus()const {return status;}
    bool getCanResume()const {return canResume;}
    int getDelay()const {return delayTime;}
    
    /**
     * These functions are used to set the transfer properties
     */
    void setPriority(int _priority) {priority = _priority;}
    void setGroup(const QString & _group) {group = _group;}
    void setSpeed(unsigned long _speed);
    

private slots:
    bool slotResume();
    void slotStop();
    void slotRetransfer();
    void slotRemove();

    /**
     * Delays the transfer for "seconds" seconds
     */
    void slotDelay(int seconds = 60);

    
signals:
    void statusChanged(Transfer *, TransferMessage message);
    void log(uint, const QString &, const QString &);


private:
    bool read(KSimpleConfig * config, int id);
    void write(KSimpleConfig * config, int id);
    
    
    /**
     * This operator is used to determine the relationship of < or >
     * priority between two transfers.
     */
    bool operator<(Transfer *);
    

    //Slave Messages
    void slavePostMessage(Slave::SlaveResult event, unsigned long data = 0L);
    void slavePostMessage(Slave::SlaveResult event, const QString & msg);
    void slaveInfoMessage(const QString & msg);

    /**
     * Debug function. This functions prints on the screen useful
     * informations about the current transfer
     */
    void about();
    

    
    /**
     * INTERNAL FUNCTIONS
     */
     
    void slaveTotalSize(unsigned long bytes);
    void slaveProcessedSize(unsigned long);
    
    void logMessage(const QString & message);
    
    void synchronousAbort();
   
    private slots:
    void slotUpdateDelay();
    
    private:
        
    Transfer * transfer;
    Slave * slave;
    Scheduler * scheduler;
    
    KURL src;
    KURL dest;

    TransferStatus status;
    
    uint id;
    int priority;
    QString group;
    QString transferLog;
    
    QDateTime startTime;
    QTime remainingTime;
    
    int delayTime;
    QTimer timer;
        
    unsigned long totalSize;
    unsigned long processedSize;
    int percent;

    int speed;

    // how many times have we retried already
    unsigned int retryCount;
    
    bool canResume;
};


#endif                          // _Transfer_h
