#include "viewinterface.h"
#include "viewinterface_p.h"
#include "scheduler.h"
#include "transferlist.h"

//BEGIN ViewInterfaceConnector implementation 
ViewInterfaceConnector::ViewInterfaceConnector( ViewInterface * viewIface, Scheduler * sched, const char * name )
    : QObject( 0, name ), iface( viewIface )
{
    // Incoming data: connect shceduler's signals to local slots
    connect( sched, SIGNAL( clear() ),
	     this, SLOT( slotCleared() ) );
    connect( sched, SIGNAL( addedItems(TransferList &) ),
	     this, SLOT( slotAddedItems(TransferList &) ) );
    connect( sched, SIGNAL( removedItems(TransferList &) ),
	     this, SLOT( slotRemovedItems(TransferList &) ) );
    connect( sched, SIGNAL( changedItems(TransferList &) ),
	     this, SLOT( slotChangedItems(TransferList &) ) );
    connect( sched, SIGNAL( globalStatus(GlobalStatus *) ),
	     this, SLOT( slotStatus(GlobalStatus *) ) );
    // Outgoing data: connect local signals to scheduler's slots
    connect( this, SIGNAL( newURLs(const KURL::List &, const QString &) ),
	     sched, SLOT( slotNewURLs(const KURL::List &, const QString &) ) );
    connect( this, SIGNAL( removeItems(TransferList &) ),
	     sched, SLOT( slotRemoveItems(TransferList &) ) );
    connect( this, SIGNAL( setPriority(TransferList &, int) ),
	     sched, SLOT( slotSetPriority(TransferList &, int) ) );
    connect( this, SIGNAL( setOperation(TransferList &, TransferOperation) ),
	     sched, SLOT( slotSetOperation(TransferList &, TransferOperation) ) );
    connect( this, SIGNAL( setGroup(TransferList &, const QString &) ),
	     sched, SLOT( slotSetGroup(TransferList &, const QString &) ) );
}

void ViewInterfaceConnector::slotCleared()
{
    //kdDebug() << "slotCleared()" << endl;
    iface->schedulerCleared();
}

void ViewInterfaceConnector::slotAddedItems( TransferList &tl )
{
    //kdDebug() << "slotAddedItems()" << endl;
    iface->schedulerAddedItems( tl );
}

void ViewInterfaceConnector::slotRemovedItems( TransferList &tl )
{
    //kdDebug() << "slotRemovedItems()" << endl;
    iface->schedulerRemovedItems( tl );
}

void ViewInterfaceConnector::slotChangedItems( TransferList &tl )
{
    //kdDebug() << "slotChangedItems()" << endl;
    iface->schedulerChangedItems( tl );
}

void ViewInterfaceConnector::slotStatus( GlobalStatus *gs )
{
    //kdDebug() << "slotStatus()" << endl;
    iface->schedulerStatus( gs );
}
//END 


//BEGIN ViewInterface iplementation 
ViewInterface::ViewInterface( Scheduler * scheduler, const char * name )
{
    d = new ViewInterfaceConnector( this, scheduler, name );
}

ViewInterface::~ViewInterface()
{
    delete d;
}

void ViewInterface::schedNewURLs( const KURL::List &l, const QString &dir )
{
    d->newURLs( l, dir );
}

void ViewInterface::schedRemoveItems( TransferList &l )
{
    d->removeItems( l );
}

void ViewInterface::schedSetPriority( TransferList &l, int p )
{
    d->setPriority( l, p );
}

void ViewInterface::schedSetOperation( TransferList &l, TransferOperation op )
{
    d->setOperation( l, op );
}

void ViewInterface::schedSetGroup( TransferList &l, const QString & g )
{
    d->setGroup( l, g );
}
//END 

#include "viewinterface_p.moc"
