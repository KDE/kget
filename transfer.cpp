/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <qdom.h>

#include "transfer.h"
#include "interrogator.h"
#include "scheduler.h"
#include "viewinterface.h"


Transfer::Transfer(Scheduler * _scheduler, KURL _src, KURL _dest)
        : sched(_scheduler)
{
    tInfo.src=_src;
    tInfo.dest=_dest;
    tInfo.status=St_Stopped;
    tInfo.priority=3;
    tInfo.totalSize=0;
    tInfo.processedSize=0;
    tInfo.percent=0;
    tInfo.speed=0;
    tInfo.group="None";
        
    connect( this, 
             SIGNAL(statusChanged(Transfer *, Transfer::TransferStatus)),
             sched,
             SLOT(slotTransferStatusChanged(Transfer *,
             Transfer::TransferStatus)) );
 
    connect( this, 
             SIGNAL(transferChanged(Transfer *, Transfer::TransferChanges)),
             sched,
             SLOT(slotTransferChanged(Transfer *, Transfer::TransferChanges)) );
}

Transfer::Transfer(Scheduler * _scheduler, QDomNode * n)
        : sched(_scheduler)

{
    tInfo.speed=0;
    
    read(n);
    
    connect( this, 
             SIGNAL(statusChanged(Transfer *, Transfer::TransferStatus)),
             sched,
             SLOT(slotTransferStatusChanged(Transfer *,
             Transfer::TransferStatus)) );

    connect( this, 
             SIGNAL(transferChanged(Transfer *, Transfer::TransferChanges)),
             sched,
             SLOT(slotTransferChanged(Transfer *, Transfer::TransferChanges)) );

}
             
const Transfer::Info& Transfer::info() const
{    
    return tInfo;
}
    
Transfer::TransferChanges Transfer::changesFlags(const TransferInterrogator * ti)
{
    if( transferChanges.find(ti) != transferChanges.end() )
        return transferChanges[ti];    
    else
    {
        //In this case the local transferChanges map does not contain 
        //any information related to the given view. So we add it.
        transferChanges[ti]=0xFFFFFFFF;
        return 0xFFFFFFFF;
    }
}

void Transfer::resetChangesFlags(const TransferInterrogator * ti)
{
    transferChanges[ti]=0;
}


void Transfer::setPriority(int p)
{
    tInfo.priority = p;
    setTransferChange(Tc_Priority);
}

void Transfer::setGroup(const QString& group)
{
    tInfo.group = group;
}

void Transfer::about() const
{
    kdDebug() << "TRANSFER: (" << this << ") " << tInfo.src.fileName() << endl;
}

void Transfer::setTransferChange(TransferChange p)
{
    QMap<const TransferInterrogator *, TransferChanges>::Iterator it = transferChanges.begin();    
    
    QMap<const TransferInterrogator *, TransferChanges>::Iterator itEnd = transferChanges.end();    
    
    for( ; it!=itEnd; ++it )
        *it |= p;
}

bool Transfer::read(QDomNode * n)
{
    sDebugIn << endl;

    QDomElement e = n->toElement();
    
    tInfo.group = e.attribute("Group");
    tInfo.priority = e.attribute("Priority").toInt();
    tInfo.status = (TransferStatus) e.attribute("Status").toInt();
    tInfo.src = KURL::fromPathOrURL(e.attribute("Source"));
    tInfo.dest = KURL::fromPathOrURL(e.attribute("Dest"));
    tInfo.totalSize = e.attribute("TotalSize").toInt();
    tInfo.processedSize = e.attribute("ProcessedSize").toInt();
    tInfo.percent = e.attribute("Percent").toULong();
    
    sDebugOut << endl;
    return true;
}


void Transfer::write(QDomNode * n)
{
    sDebugIn << endl;
    
    QDomElement t = n->ownerDocument().createElement("Transfer");
    n->appendChild(t);
    
    t.setAttribute("Group", tInfo.group);
    t.setAttribute("Priority", tInfo.priority);
    t.setAttribute("Status", tInfo.status);
    t.setAttribute("Source", tInfo.src.url());
    t.setAttribute("Dest", tInfo.dest.url());
    t.setAttribute("TotalSize", tInfo.totalSize);
    t.setAttribute("ProcessedSize", tInfo.processedSize);
    t.setAttribute("Percent", tInfo.percent);
    
    sDebugOut << endl;
}
