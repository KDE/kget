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

#include <qiconview.h>
#include <qlayout.h>

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
	virtual void paintFocus( QPainter * p, const QColorGroup & cg );

    private:
	QPixmap mimePix;
	Transfer * transfer;
};

class IconView : public QIconView, public ViewInterface
{
    Q_OBJECT
    public:
	IconView( Scheduler * s, QWidget * parent = 0, const char * name = 0 );
	
	virtual void schedulerCleared();
	virtual void schedulerAddedItems( TransferList & );
	virtual void schedulerRemovedItems( TransferList & );
	virtual void schedulerChangedItems( TransferList & );
	virtual void schedulerStatus( GlobalStatus * );

    public slots:
	void slotRightButtonClicked( QIconViewItem *, const QPoint & );

	//from "kget menu" popup
	void slotNewTransfer();
	void slotRemoveItems();
	void slotSetPriority();
	void slotSetOperation();
	void slotSetGroup();
	//from "transfer operations" popup
};

class IconViewWidget : public QWidget
{
    public:
	IconViewWidget( Scheduler * s, QWidget * parent = 0, const char * name = 0 )
	{
	    QVBoxLayout * vLay = new QVBoxLayout( this );
	    vLay->addWidget( new IconView( s, this ) );
	    IconView * iv2 = new IconView( s, this );
	    iv2->setItemTextPos( QIconView::Right );
	    vLay->addWidget( iv2 );
	}
};

#endif
