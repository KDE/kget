/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "viewinterface.h"
#include "viewinterface_p.h"
#include "scheduler.h"
#include "transferlist.h"
#include "group.h"

#include <kdebug.h>

//BEGIN ViewInterfaceConnector implementation 
ViewInterfaceConnector::ViewInterfaceConnector( ViewInterface * viewIface, Scheduler * sched, const char * name )
    : QObject( 0, name ), iface( viewIface )
{
    // Incoming data: connect scheduler's signals to local slots
    connect( sched, SIGNAL( clear() ),
	     this, SLOT( slotCleared() ) );
    connect( sched, SIGNAL( addedItems(TransferList) ),
	     this, SLOT( slotAddedItems(TransferList) ) );
    connect( sched, SIGNAL( removedItems(TransferList) ),
	     this, SLOT( slotRemovedItems(TransferList) ) );
    connect( sched, SIGNAL( changedItems(TransferList) ),
	     this, SLOT( slotChangedItems(TransferList) ) );
    connect( sched, SIGNAL( addedGroups(GroupList) ),
	     this, SLOT( slotAddedGroups(GroupList) ) );
    connect( sched, SIGNAL( removedGroups(GroupList) ),
	     this, SLOT( slotRemovedGroups(GroupList) ) );
    connect( sched, SIGNAL( changedGroups(GroupList) ),
	     this, SLOT( slotChangedGroups(GroupList) ) );
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
    connect( this, SIGNAL( addGroup(GroupList) ),
	     sched, SLOT( slotAddGroup(GroupList) ) );
    connect( this, SIGNAL( delGroup(GroupList) ),
	     sched, SLOT( slotDelGroup(GroupList) ) );
    connect( this, SIGNAL( modifyGroup(const QString&, Group) ),
	     sched, SLOT( slotModifyGroup(const QString&, Group) ) );
    connect( this, SIGNAL( reqOperation(SchedulerOperation) ),
	     sched, SLOT( slotReqOperation(SchedulerOperation) ) );
    connect( this, SIGNAL( reqOperation(SchedulerDebugOp) ),
	     sched, SLOT( slotReqOperation(SchedulerDebugOp) ) );
    // Clears and fills up the view
    slotCleared();
    slotAddedGroups( sched->getGroups() );
    slotAddedItems( sched->getTransfers() );
}

void ViewInterfaceConnector::slotCleared()
{
    //kdDebug() << "slotCleared()" << endl;
    iface->schedulerCleared();
}

void ViewInterfaceConnector::slotAddedItems( TransferList tl )
{
//     kdDebug() << "slotAddedItems()" << endl;
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

void ViewInterfaceConnector::slotAddedGroups( GroupList gl )
{
//     kdDebug() << "slotAddedGroups()" << endl;
    iface->schedulerAddedGroups( gl );
}

void ViewInterfaceConnector::slotRemovedGroups( GroupList gl )
{
    //kdDebug() << "slotRemovedItems()" << endl;
    iface->schedulerRemovedGroups( gl );
}

void ViewInterfaceConnector::slotChangedGroups( GroupList gl )
{
    //kdDebug() << "slotChangedItems()" << endl;
    iface->schedulerChangedGroups( gl );
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

void ViewInterface::schedDelItems( TransferList l )
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
    d->setOperation( l, op );
}

void ViewInterface::schedSetGroup( TransferList l, const QString & g )
{
    if ( d )
	d->setGroup( l, g );
}

void ViewInterface::schedAddGroup( GroupList l )
{
    kdDebug() << "viewinterface" << endl;
    if ( d )
    d->addGroup( l );
}

void ViewInterface::schedDelGroup( GroupList l )
{
    if ( d )
    {
	kdDebug() << "aaaaaaaaa" << endl;
    d->delGroup( l );
    }
}

void ViewInterface::schedModifyGroup( const QString & n, Group g )
{
    if ( d )
	d->modifyGroup( n, g );
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
