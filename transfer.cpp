#include <qdom.h>

#include "transfer.h"
#include "scheduler.h"
#include "viewinterface.h"

Transfer::Transfer(Scheduler * _scheduler, const KURL & _src, const KURL & _dest)
        : sched(_scheduler)
{
    info.src=_src;
    info.dest=_dest;
    info.status=St_Stopped;
    info.priority=3;
    info.totalSize=0;
    info.processedSize=0;
    info.percent=0;
    info.speed=0;
    info.group="None";
        
    connect(this, 
            SIGNAL(statusChanged(Transfer *, Transfer::TransferStatus)),
            sched,
            SLOT(slotTransferStatusChanged(Transfer *, Transfer::TransferStatus)));

    connect(this, 
            SIGNAL(transferChanged(Transfer *, Transfer::TransferChanges)),
            sched,
            SLOT(slotTransferChanged(Transfer *, Transfer::TransferChanges)));
}

Transfer::Transfer(Scheduler * _scheduler, QDomNode * n)
        : sched(_scheduler)

{
    info.speed=0;
    
    read(n);
    
    connect(this, 
            SIGNAL(statusChanged(Transfer *, Transfer::TransferStatus)),
            sched,
            SLOT(slotTransferStatusChanged(Transfer *, Transfer::TransferStatus)));

    connect(this, 
            SIGNAL(transferChanged(Transfer *, Transfer::TransferChanges)),
            sched,
            SLOT(slotTransferChanged(Transfer *, Transfer::TransferChanges)));

}
             
const Transfer::Info& Transfer::getInfo() const
{    
    return info;
}
    
Transfer::TransferChanges Transfer::getChangesFlags(ViewInterface * view)
{
    int id = view->getId();
    int s = transferChanges.size();
        
    if( s < id )
        //In this case the local transferChanges vector does not contain 
        //any information related to the given view. So we add a vector 
        //for the view. Since we know that to each views is assigned 
        //an increasing id number starting from 0, we create
        //a transferChanges entry for each view with an id number lower
        //than the given one.
        //transferChanges.resize(id+1, 10);
        transferChanges.resize(id+1, 0xFFFFFFFF);

    return transferChanges[id];
}

void Transfer::resetChangesFlags(ViewInterface * view)
{
    int id = view->getId();
    int s = transferChanges.size();

    QValueVector<int> p;
    p.resize(10, 1);
        
    if( id < s )
        //In this case the local transferChanges vector does not contain 
        //any information related to the given view. So we add a vector 
        //for the view. Since we know that to each views is assigned 
        //an increasing id number starting from 0, we create
        //a transferChanges entry for each view with an id number lower
        //than the given one.
        //transferChanges.resize(id+1, 10);
        transferChanges.resize(id+1, 0xFFFFFFFF);
    
    transferChanges[id] = 0;
}


void Transfer::setPriority(int p)
{
    info.priority = p;
    setTransferChange(Tc_Priority);
}

void Transfer::setGroup(const QString& group)
{
    info.group = group;
}

void Transfer::about() const
{
    kdDebug() << "TRANSFER: (" << this << ") " << info.src.fileName() << endl;
}

void Transfer::setTransferChange(TransferChange p)
{
    int vectorSize = transferChanges.size();
    for(int i=0; i<vectorSize; i++)
        transferChanges[i] |= p;
}

bool Transfer::read(QDomNode * n)
{
    sDebugIn << endl;

    QDomElement e = n->toElement();
    
    info.group = e.attribute("Group");
    info.priority = e.attribute("Priority").toInt();
    info.status = (TransferStatus) e.attribute("Status").toInt();
    info.src = KURL::fromPathOrURL(e.attribute("Source"));
    info.dest = KURL::fromPathOrURL(e.attribute("Dest"));
    info.totalSize = e.attribute("TotalSize").toInt();
    info.processedSize = e.attribute("ProcessedSize").toInt();
    info.percent = e.attribute("Percent").toULong();
    sDebugOut << endl;
    return true;
}


void Transfer::write(QDomNode * n)
{
    sDebugIn << endl;
    
    QDomElement t = n->ownerDocument().createElement("Transfer");
    n->appendChild(t);
    
    t.setAttribute("Group", info.group);
    t.setAttribute("Priority", info.priority);
    t.setAttribute("Status", info.status);
    t.setAttribute("Source", info.src.url());
    t.setAttribute("Dest", info.dest.url());
    t.setAttribute("TotalSize", info.totalSize);
    t.setAttribute("ProcessedSize", info.processedSize);
    t.setAttribute("Percent", info.percent);
    
    sDebugOut << endl;
}
