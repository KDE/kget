#include <qobject.h>
#include "viewinterface.h"
#include "scheduler.h"

ViewInterface::ViewInterface( Scheduler * sched )
{
    // Incoming data: connect shceduler's signals to local slots
    connect( sched, SIGNAL( addedItems(QValueList<Transfer *>) ),
	     this, SLOT( schedulerAddedItems(QValueList<Transfer *>) ) );
    connect( sched, SIGNAL( removedItems(QValueList<Transfer *>) ),
	     this, SLOT( schedulerRemovedItems(QValueList<Transfer *>) ) );
    connect( sched, SIGNAL( changedItems(QValueList<Transfer *>) ),
	     this, SLOT( schedulerChangedItems(QValueList<Transfer *>) ) );
    connect( sched, SIGNAL( clear() ),
	     this, SLOT( schedulerCleared() ) );
    connect( sched, SIGNAL( globalStatus(GlobalStatus *) ),
	     this, SLOT( schedulerStatus(GlobalStatus *) ) );

    // Outgoing data: connect local signals to scheduler's slots
    connect( this, SIGNAL( newURLs(const KURL::List &, const QString &) ),
	     sched, SLOT( slotNewURLs(const KURL::List &, const QString &) ) );
    connect( this, SIGNAL( removeItems(QValueList<Transfer *>) ),
	     sched, SLOT( slotRemoveItems(QValueList<Transfer *>) ) );
    connect( this, SIGNAL( setPriority(QValueList<Transfer *>, int) ),
	     sched, SLOT( slotSetPriority(QValueList<Transfer *>, int) ) );
    connect( this, SIGNAL( setOperation(QValueList<Transfer *>, TransferOperation) ),
	     sched, SLOT( slotSetOperation(QValueList<Transfer *>, TransferOperation) ) );
    connect( this, SIGNAL( setGroup(QValueList<Transfer *>, const QString &) ),
	     sched, SLOT( slotSetGroup(QValueList<Transfer *>, const QString &) ) );
}


void ViewInterface::schedulerCleared()
{}

void ViewInterface::schedulerAddedItems( QValueList<Transfer *> )
{}

void ViewInterface::schedulerRemovedItems( QValueList<Transfer *> )
{}

void ViewInterface::schedulerChangedItems( QValueList<Transfer *> )
{}

void ViewInterface::schedulerStatus( GlobalStatus * )
{}

#include "viewinterface.moc"
