/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _TRANSFER_H
#define _TRANSFER_H

#include <qobject.h>
#include <qmap.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <kurl.h>

class QDomElement;
class QDomNode;
class QDomDocument;

class TransferList;
class Scheduler;
class ViewInterface;
class TransferInterrogator;

class Transfer : public QObject
{
Q_OBJECT
    friend class TransferList;

public:
    enum TransferStatus  
    { 
        St_Trying,
        St_Running,
        St_Delayed,
        St_Stopped,
        St_Aborted,
        St_Finished
    };  

    enum TransferChange  
    { 
        Tc_None          = 0x00000000,
        Tc_Priority      = 0x00000001,
        Tc_Status        = 0x00000002,
        Tc_CanResume     = 0x00000004,
        Tc_TotalSize     = 0x00000008,
        Tc_ProcessedSize = 0x00000010,
        Tc_Percent       = 0x00000020,
        Tc_Speed         = 0x00000040,
        Tc_Log           = 0x00000080 
    };
    
    typedef int TransferChanges;                         
    
    typedef struct Info
    {
        int priority;
        TransferStatus status;
        QValueList <QString *> log;
        
        KURL src;
        KURL dest;
        
        unsigned long totalSize;
        unsigned long processedSize;
        int percent;
    
        int speed;
    
        QString group;  
        QDateTime startTime;
        QTime remainingTime;
        int delayTime;
        
        bool canResume;
        
        /**
        * This field specifies whether the transfer can perform segmented 
        * downloading or not.
        */
        bool canSegment;
    };

    Transfer(Scheduler * _scheduler, KURL _src, KURL _dest);
    Transfer(Scheduler * _scheduler, QDomNode * n);
    
    const Info& info() const;
    
    TransferChanges changesFlags(const TransferInterrogator *);
    void resetChangesFlags(const TransferInterrogator *);
       
    void setPriority(int p);
    void setGroup(const QString& group);
    
    inline bool operator<(const Transfer& t2) const
        {return tInfo.priority < t2.tInfo.priority;}
    
    inline bool operator<=(const Transfer& t2) const
        {return tInfo.priority <= t2.tInfo.priority;}

    inline bool operator>(const Transfer& t2) const
        {return tInfo.priority > t2.tInfo.priority;}
    
    inline bool operator>=(const Transfer& t2) const
        {return tInfo.priority >= t2.tInfo.priority;}
        
    inline bool operator==(const Transfer& t2) const
        {return tInfo.priority == t2.tInfo.priority;}
        
    void about() const;
        
public slots:
    /**
     * These slots _MUST_ be reimplemented
     */
    virtual bool slotResume() = 0;
    virtual void slotStop() = 0;
    virtual void slotRetransfer() = 0;
    virtual void slotRemove() = 0;
    
    virtual void slotSetSpeed(int speed) = 0;
    virtual void slotSetDelay(int seconds) = 0;
    virtual void slotSetSegmented(int nSegments) = 0;
   
signals:
    void transferChanged(Transfer *);

protected:
    /**
     * These functions can be reimplemented if necessary
     */
    virtual bool read(QDomNode * n);
    virtual void write(QDomNode * n);

    void setTransferChange(TransferChange);
   
    Info tInfo;
        
private:
    Scheduler * sched;
    QMap<const TransferInterrogator *, TransferChanges> transferChanges;
};

#endif
