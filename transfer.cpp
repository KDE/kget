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
    
    connect(this, 
            SIGNAL(statusChanged(Transfer *, Transfer::TransferStatus)),
            sched,
            SLOT(slotTransferStatusChanged(Transfer *, Transfer::TransferStatus)));

    connect(this, 
            SIGNAL(progressChanged(Transfer *, Transfer::ProgressChange)),
            sched,
            SLOT(slotTransferProgressChanged(Transfer *, Transfer::ProgressChange)));
}

Transfer::Transfer(Scheduler * _scheduler, QDomDocument * doc, QDomNode * e)
        : sched(_scheduler)

{
    info.speed=0;
    
    read(doc, e);
    
    connect(this, 
            SIGNAL(statusChanged(Transfer *, Transfer::TransferStatus)),
            sched,
            SLOT(slotTransferStatusChanged(Transfer *, Transfer::TransferStatus)));

    connect(this, 
            SIGNAL(progressChanged(Transfer *, Transfer::ProgressChange)),
            sched,
            SLOT(slotTransferProgressChanged(Transfer *, Transfer::ProgressChange)));

}
             
const Transfer::Info& Transfer::getInfo() const
{    
    return info;
}
    
Transfer::TransferProgress Transfer::getProgressFlags(ViewInterface * view)
{
    int id = view->getId();
    int s = progressChanges.size();
        
    if( s < id )
        //In this case the local progressChanges vector does not contain 
        //any information related to the given view. So we add a vector 
        //for the view. Since we know that to each views is assigned 
        //an increasing id number starting from 0, we create
        //a progressChanges entry for each view with an id number lower
        //than the given one.
        //progressChanges.resize(id+1, 10);
        progressChanges.resize(id+1, 0xFFFFFFFF);

    return progressChanges[id];
}

void Transfer::resetProgressFlags(ViewInterface * view)
{
    int id = view->getId();
    int s = progressChanges.size();

    QValueVector<int> p;
    p.resize(10, 1);
        
    if( id < s )
        //In this case the local progressChanges vector does not contain 
        //any information related to the given view. So we add a vector 
        //for the view. Since we know that to each views is assigned 
        //an increasing id number starting from 0, we create
        //a progressChanges entry for each view with an id number lower
        //than the given one.
        //progressChanges.resize(id+1, 10);
        progressChanges.resize(id+1, 0xFFFFFFFF);
    
    progressChanges[id] = 0;
}


void Transfer::setPriority(int p)
{
    info.priority = p;
    setProgressChange(Pc_Priority);
}

void Transfer::setGroup(const QString& group)
{
    info.group = group;
}

void Transfer::about() const
{
    kdDebug() << "TRANSFER: (" << this << ") " << info.src.fileName() << endl;
}

void Transfer::setProgressChange(ProgressChange p)
{
    int vectorSize = progressChanges.size();
    for(int i=0; i<vectorSize; i++)
        progressChanges[i] |= p;
}

bool Transfer::read(QDomDocument * doc, QDomNode * n)
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


void Transfer::write(QDomDocument * doc, QDomNode * n)
{
    sDebugIn << endl;
    
    QDomElement t = doc->createElement("Transfer");
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
