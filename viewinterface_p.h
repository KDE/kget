/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _VIEWINTERFACE_P_H
#define _VIEWINTERFACE_P_H

class ViewInterface;

//NOTE this class is privately used inside the viewinterface

/*
 * VIConnector class : connects via signals/slots to the scheduler and
 * inform the viewinterface (when new data arrives) by calling
 * interface's virtual functions,
 */
class ViewInterfaceConnector : public QObject
{
    Q_OBJECT
    public:
	ViewInterfaceConnector( ViewInterface * viewIface, Scheduler * sched, const char * name );

    signals:
	void newURLs( const KURL::List &, const QString &destDir );
	void removeItems( TransferList & );
	void setPriority( TransferList &, int );
	void setOperation( TransferList &, TransferCommand );
	void setGroup( TransferList &, const QString & );
	void reqOperation( SchedulerOperation );

    public slots:
	void slotCleared();
	void slotAddedItems( TransferList & );
	void slotRemovedItems( TransferList & );
	void slotChangedItems( TransferList & );
	void slotStatus( GlobalStatus * );

    private:
	friend class ViewInterface;
	ViewInterface * iface;
};

#endif
