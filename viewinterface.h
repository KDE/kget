/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

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

#include "transfer.h"
#include "transferlist.h"
#include "group.h"


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

class ViewInterface : public TransferInterrogator
{
    public:
	ViewInterface( const char * name = "view-iface" );
	virtual ~ViewInterface();

	/** Call this method to wire up the view interface to a Scheduler */
	void connectToScheduler( Scheduler * );

    /** Call this method to get the View id number */
    int getId();
    
	/** commands (-> scheduler)
	 * Those functions must be called to dispatch information to the
	 * connected scheduler.
	 */
	void schedNewURLs( const KURL::List &, const QString &destDir );
	void schedDelItems( TransferList );
	void schedSetPriority( TransferList, int );
	void schedSetCommand( TransferList, TransferCommand );
	
    void schedSetGroup( TransferList , const QString &);
    /**
     * schedAddGroup: here we must provide the new group
     */
    void schedAddGroup( GroupList );
    /**
     * schedDelGroup: here we must provide the group name
     */
    void schedDelGroup( GroupList );
    /**
     * schedModifyGroup: here we must provide the group name before the
     * last changes and the new group itself 
     */
	void schedModifyGroup( const QString &, Group );
    
    void schedRequestOperation( SchedulerOperation );
	void schedDebugOperation( SchedulerDebugOp );

	/** pure virtual 'notifications' (<- scheduler)
	 * The functions *must* be implemented to receive notifications
	 * (files added/removed, status changed, etc..) from the scheduler.
	 */
	virtual void schedulerCleared() {};
	virtual void schedulerAddedItems( const TransferList& ) {};
	virtual void schedulerRemovedItems( const TransferList& ) {};
	virtual void schedulerChangedItems( const TransferList& ) {};
    virtual void schedulerAddedGroups( const GroupList& ) {};
    virtual void schedulerRemovedGroups( const GroupList& ) {};
    virtual void schedulerChangedGroups( const GroupList& ) {};
	virtual void schedulerStatus( GlobalStatus * ) {};

    private:
	class ViewInterfaceConnector * d;
    int id;
	const char * name;
};

#endif

