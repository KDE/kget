#ifndef _VIEWINTERFACE_H
#define _VIEWINTERFACE_H

#include <kurl.h>
#include <qvaluelist.h>
#include <qobject.h>

#include "globals.h"

class ViewInterface : public QObject
{
Q_OBJECT
public:
	ViewInterface( Scheduler * );

signals:
	/**
	 * Those are the commands for the scheduler
	 */
	void newURLs( const KURL::List &, const QString &destDir );
	void removeItems( QValueList<Transfer *> );
	void setPriority( QValueList<Transfer *>, int );
	void setOperation( QValueList<Transfer *>, TransferOperation );
	void setGroup( QValueList<Transfer *>, const QString & );

public slots:
	/**
	 * Every slot here is a scheduler notification/answer. Just
	 * reimplement those in a subclass to catch broadcasted
	 * messages.
	 */
	virtual void schedulerCleared();
	virtual void schedulerAddedItems( QValueList<Transfer *> );
	virtual void schedulerRemovedItems( QValueList<Transfer *> );
	virtual void schedulerChangedItems( QValueList<Transfer *> );
	virtual void schedulerStatus( GlobalStatus * );
};

#endif

