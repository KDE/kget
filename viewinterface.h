/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _VIEWINTERFACE_H
#define _VIEWINTERFACE_H

#include <kurl.h>
#include <qobject.h>
#include "globals.h"

/**
  * This class defines an unified interface to talk to the scheduler.
  * It defines some pure virtual functions, that must be implemented to
  * make a VIEW (a generic 'monitor' over the scheduler) working.
  *
  * This class is not a widget, and may be inherited from every 'VIEW'
  * such as the tray manager, the small dock-widget or any mdichild*
  * objects.
  *
  * @short A base class for exchanging data with the scheduler.
  * @author Enrico Ros <eros.kde@email.it>
  * @version $Id: 
  **/

class ViewInterface
{
    public:
	ViewInterface( const char * name = "view-iface" );
	virtual ~ViewInterface();

	/** Call this method to wire up the view interface to a Scheduler */
	void connectToScheduler( Scheduler * );

	/** commands (-> scheduler)
	 * Those functions must be called to dispatch information to the
	 * connected scheduler.
	 */
	void schedNewURLs( const KURL::List &, const QString &destDir );
	void schedRemoveItems( TransferList & );
	void schedSetPriority( TransferList &, int );
	void schedSetCommand( TransferList &, TransferCommand );
	void schedSetGroup( TransferList &, const QString & );
	void schedRequestOperation( SchedulerOperation );
	void schedDebugOperation( SchedulerDebugOp );

	/** pure virtual 'notifications' (<- scheduler)
	 * The functions *must* be implemented to receive notifications
	 * (files added/removed, status changed, etc..) from the scheduler.
	 */
	virtual void schedulerCleared() {};
	virtual void schedulerAddedItems( TransferList &) {};
	virtual void schedulerRemovedItems( TransferList &) {};
	virtual void schedulerChangedItems( TransferList &) {};
	virtual void schedulerStatus( GlobalStatus * ) {};

    private:
	class ViewInterfaceConnector * d;
	const char * name;
};

#endif

