/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _ICONVIEW_H
#define _ICONVIEW_H

#include "viewinterface.h"
#include "transfer.h"

#include <qiconview.h>
#include <kmdichildview.h>

class IconView;
class QPixmap;

class IconViewTransfer : public QIconViewItem
{
    public:
	IconViewTransfer( QIconView * parent, Transfer * transfer );

	// recalculates the pixmap
	void update();

	// return the internal pointer to the trasfer
	Transfer * getTransfer();

    protected:
	virtual void paintItem( QPainter * p, const QColorGroup & cg );
	virtual void paintFocus( QPainter * p, const QColorGroup & cg );

    private:
	QPixmap mimePix;
	Transfer * transfer;
    Transfer::Info transferInfo;
};

class IconView : public QIconView, public ViewInterface
{
    Q_OBJECT
    public:
	IconView( QWidget * parent = 0, const char * name = 0 );
	
	virtual void schedulerCleared();
	virtual void schedulerAddedItems( TransferList );
	virtual void schedulerRemovedItems( TransferList );
	virtual void schedulerChangedItems( TransferList );
	virtual void schedulerStatus( GlobalStatus * );

    public slots:
	void slotRightButtonClicked( QIconViewItem *, const QPoint & );

	//from "kget menu" popup
	void slotNewTransfer();
	//from "transfer operations" popup
	void slotResumeItems();
	void slotStopItems();
	void slotRemoveItems();

	void slotSetPriority1();
	void slotSetPriority2();
	void slotSetPriority3();
	void slotSetPriority4();
	void slotSetPriority5();
	void slotSetPriority6();
	void slotSetGroup();

    private:
	TransferList getSelectedList();
};

class IconViewMdiView : public KMdiChildView
{
    public:
	IconViewMdiView( QWidget * parent = 0 );
	void connectToScheduler( Scheduler * );
    private:
	IconView * iv1;
};

#endif
