#ifndef _VIEWINTERFACE_H
#define _VIEWINTERFACE_H

#include "scheduler.h"

class ViewInterface : public QObject
{
QOBJECT
public:
	ViewInterface( Scheduler * s ) { connectScheduler( s ); }

signals:
	void newURLs( KURL::List );
	void removeItems( QValueList<Transfer *> );
	void setPriority( QValueList<Transfer *>, int );
	void setOperation( QValueList<Transfer *>, enum Operation );
	void setGroup( QVaueList<Transfer *>, const QString & );

public slots:
	virtual void schedulerCleared();
	virtual void schedulerAddedItems( QValueList<Transfer *> );
	virtual void schedulerRemovedItems( QValueList<Transfer *> );
	virtual void schedulerChangedItems( QValueList<Transfer *> );
	virtual void schedulerStatus( GlobalStatus * );
	
private: 
	void connectScheduler( Scheduler * sched )
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
		connect( this, SIGNAL( newURLs( KURL::List ) ),
			 sched, SLOT( slotNewURLs(KURL::List) ) );
		connect( this, SIGNAL( removeItems(QValueList<Transfer *>) ),
			 sched, SLOT( slotRemoveItems(QVaueList<Transfer *>) ) );
		connect( this, SIGNAL( setPriority(QValueList<Transfer *>, int) ),
			 sched, SLOT( slotSetPriority(QVaueList<Transfer *>, int) ) );
		connect( this, SIGNAL( setOperation(QValueList<Transfer *>, enum Operation) ),
			 sched, SLOT( slotSetOperation(QVaueList<Transfer *>, enum Operation) ) );
		connect( this, SIGNAL( setGroup(QVaueList<Transfer *>, const QString &) ),
			 sched, SLOT( slotSetGroup(QVaueList<Transfer *>, const QString &) ) );
	}
};

#endif

