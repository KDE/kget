#include "viewinterface.h"
#include "viewinterface_p.h"
#include "scheduler.h"
#include "transferlist.h"

#include <kdebug.h>

//BEGIN ViewInterfaceConnector implementation 
ViewInterfaceConnector::ViewInterfaceConnector( ViewInterface * viewIface, Scheduler * sched, const char * name )
    : QObject( 0, name ), iface( viewIface )
{
    // Incoming data: connect shceduler's signals to local slots
    connect( sched, SIGNAL( clear() ),
	     this, SLOT( slotCleared() ) );
    connect( sched, SIGNAL( addedItems(TransferList) ),
	     this, SLOT( slotAddedItems(TransferList) ) );
    connect( sched, SIGNAL( removedItems(TransferList) ),
	     this, SLOT( slotRemovedItems(TransferList) ) );
    connect( sched, SIGNAL( changedItems(TransferList) ),
	     this, SLOT( slotChangedItems(TransferList) ) );
    connect( sched, SIGNAL( globalStatus(GlobalStatus *) ),
	     this, SLOT( slotStatus(GlobalStatus *) ) );
    // Outgoing data: connect local signals to scheduler's slots
    connect( this, SIGNAL( newURLs(const KURL::List &, const QString &) ),
	     sched, SLOT( slotNewURLs(const KURL::List &, const QString &) ) );
    connect( this, SIGNAL( removeItems(TransferList) ),
	     sched, SLOT( slotRemoveItems(TransferList) ) );
    connect( this, SIGNAL( setPriority(TransferList, int) ),
	     sched, SLOT( slotSetPriority(TransferList, int) ) );
    connect( this, SIGNAL( setOperation(TransferList, TransferCommand) ),
	     sched, SLOT( slotSetCommand(TransferList, TransferCommand) ) );
    connect( this, SIGNAL( setGroup(TransferList, const QString &) ),
	     sched, SLOT( slotSetGroup(TransferList, const QString &) ) );
    connect( this, SIGNAL( reqOperation(SchedulerOperation) ),
	     sched, SLOT( slotReqOperation(SchedulerOperation) ) );
    connect( this, SIGNAL( reqOperation(SchedulerDebugOp) ),
	     sched, SLOT( slotReqOperation(SchedulerDebugOp) ) );
    // Clears and fills up the view
    slotCleared();
    slotAddedItems( sched->getTransfers() );
}

void ViewInterfaceConnector::slotCleared()
{
    //kdDebug() << "slotCleared()" << endl;
    iface->schedulerCleared();
}

void ViewInterfaceConnector::slotAddedItems( TransferList tl )
{
    //kdDebug() << "slotAddedItems()" << endl;
    iface->schedulerAddedItems( tl );
}

void ViewInterfaceConnector::slotRemovedItems( TransferList tl )
{
    //kdDebug() << "slotRemovedItems()" << endl;
    iface->schedulerRemovedItems( tl );
}

void ViewInterfaceConnector::slotChangedItems( TransferList tl )
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
ViewInterface::ViewInterface( const char * n )
    : d( 0 ), name( n ) 
{
    static int newId=-1;
    id = ++newId;
    kdDebug() << "new ViewInterface: id = " << id << endl;
}

ViewInterface::~ViewInterface()
{
    delete d;
}

void ViewInterface::connectToScheduler( Scheduler * scheduler )
{
    ViewInterfaceConnector * old = d;
    d = new ViewInterfaceConnector( this, scheduler, name );
    delete old;
}

int ViewInterface::getId()
{
    return id;
}

void ViewInterface::schedNewURLs( const KURL::List &l, const QString &dir )
{
    if ( d )
	d->newURLs( l, dir );
}

void ViewInterface::schedRemoveItems( TransferList l )
{
    if ( d )
	d->removeItems( l );
}

void ViewInterface::schedSetPriority( TransferList l, int p )
{
    if ( d )
	d->setPriority( l, p );
}

void ViewInterface::schedSetCommand( TransferList l, TransferCommand op )
{
    if ( d )
    {
        kdDebug() << "aaaaaaaaaaaaaaaaaaaaa" << endl;
	    d->setOperation( l, op );
    }
    else
        kdDebug() << "bbbbbbbbbbbbbbbbbbbbb" << endl;
}

void ViewInterface::schedSetGroup( TransferList l, const QString & g )
{
    if ( d )
	d->setGroup( l, g );
}

void ViewInterface::schedRequestOperation( SchedulerOperation op )
{
    if ( d )
	d->reqOperation( op );
}

void ViewInterface::schedDebugOperation( SchedulerDebugOp op )
{
    if ( d )
	d->reqOperation( op );
}

//END 

#include "viewinterface_p.moc"
