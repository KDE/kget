/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qpixmap.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qfont.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpalette.h>
#include <kpopupmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmimetype.h>

#include "iconview.h"
#include "transferlist.h"
#include "transfer.h"

/*
* KGetIconViewItem
*
*/

IconViewTransfer::IconViewTransfer( QIconView * parent, Transfer * t )
    : QIconViewItem( parent ), transfer( t )
{
    setRenameEnabled( false );
    setDragEnabled( false );
    setDropEnabled( false );

    if ( transfer )
    {
	KMimeType::Ptr mime = KMimeType::findByURL( transfer->getDest() , 0, true, false );
	mimePix = mime->pixmap( KIcon::Desktop, 48 );
	setPixmap( mimePix );
	setText( transfer->getDest().path() );
    } else
	setText( "transfer = NULL" );
}

void IconViewTransfer::update()
{
    repaint();
}
 
Transfer * IconViewTransfer::getTransfer()
{
    return transfer;
}

void IconViewTransfer::paintItem( QPainter * p, const QColorGroup & cg )
{
    //first draw the icon
    QIconViewItem::paintItem( p, cg );

    //the draw the overlay informations
    p->save();
    int w = mimePix.width(),
        h = mimePix.height();
    if ( w < 20 || h < 10 || !transfer )
	return;

    QRect r = pixmapRect();
    r.moveBy( x(), y() );
    p->setPen( Qt::black );
    p->setBrush( Qt::white );
    p->drawRect( r.left() + 5, r.top() + h - 8, w - 10, 7 );
    int percent = transfer->getPercent();
    if ( percent > 100 )
	percent = 100;
    if ( percent < 0 )
	percent = 0;
    w = ((w - 12) * percent) / 100;
    p->setPen( Qt::white );
    p->setBrush( transfer->getStatus() == ST_RUNNING ? Qt::green : Qt::red );
    p->drawRect( r.left() + 6, r.top() + h - 7, w, 5 );
    p->restore();
}

void IconViewTransfer::paintFocus( QPainter *, const QColorGroup & )
{
/*  No focus is better than colored focus */
}


/*
* KGetIconView
*
*/

IconView::IconView( QWidget * parent, const char * name )
    : QIconView( parent, name ), ViewInterface()
{
    setSelectionMode( Extended );
    connect( this, SIGNAL(rightButtonClicked(QIconViewItem *,const QPoint &)),
             this, SLOT(slotRightButtonClicked(QIconViewItem *,const QPoint &)) );
}

void IconView::slotRightButtonClicked( QIconViewItem * item, const QPoint & pos )
{
    bool overItems = item != 0;
    
    Transfer * transfer = 0;
    if ( overItems )
    {
	IconViewTransfer * ivt = static_cast<IconViewTransfer*>(item);
	transfer = ivt->getTransfer();
	if ( !transfer )
	    overItems = false;
    }
    
    KPopupMenu * popup = new KPopupMenu( this );
    
    // insert title
    QString t1 = i18n("Transfer operations Menu");
    QString t2 = i18n("KGet Menu");
    popup->insertTitle( overItems ? t1 : t2 );
    
    // add menu entries
    if ( overItems )
    {   // menu over an item
	popup->insertItem( SmallIcon("down"), i18n("Resume"), this, SLOT(slotResumeItems()) );
	popup->insertItem( SmallIcon("stop"), i18n("Stop"), this, SLOT(slotStopItems()) );
	popup->insertItem( SmallIcon("remove"), i18n("Remove"), this, SLOT(slotRemoveItems()) );
	
	KPopupMenu * subPrio = new KPopupMenu( popup );
	subPrio->insertItem( SmallIcon("2uparrow"), i18n("highest"), this,  SLOT( slotSetPriority1() ) );
	subPrio->insertItem( SmallIcon("1uparrow"), i18n("high"), this,  SLOT( slotSetPriority2() ) );
	subPrio->insertItem( i18n("normal"), this,  SLOT( slotSetPriority3() ) );
	subPrio->insertItem( SmallIcon("1downarrow"), i18n("low"), this,  SLOT( slotSetPriority4() ) );
	subPrio->insertItem( SmallIcon("2downarrow"), i18n("lowest"), this,  SLOT( slotSetPriority5() ) );
	subPrio->insertItem( SmallIcon("stop"), i18n("do now download"), this,  SLOT( slotSetPriority6() ) );
	popup->insertItem( i18n("Set priority"), subPrio );
	
	KPopupMenu * subGroup = new KPopupMenu( popup );
	//for loop inserting all existant groups
	subGroup->insertItem( i18n("new ..."), this,  SLOT( slotSetGroup() ) );	
	popup->insertItem( SmallIcon("folder"), i18n("Set group ..."), subGroup );
    }
    else
	// menu on empty space
	popup->insertItem( i18n("New transfer ..."), this, SLOT(slotNewTransfer()) );

    // show popup
    popup->popup( pos );
}

TransferList IconView::getSelectedList()
{
    TransferList sl;
    IconViewTransfer * ivt = static_cast<IconViewTransfer*>(firstItem());
    while ( ivt )
    {
	if ( ivt->isSelected() )
	    sl.addTransfer( ivt->getTransfer() );
	ivt = static_cast<IconViewTransfer*>(ivt->nextItem());
    }
    return sl;
}

void IconView::slotNewTransfer()
{
    schedNewURLs( KURL(), QString::null );
}

void IconView::slotResumeItems()
{
    TransferList tl = getSelectedList();
    schedSetCommand( tl, CmdResume );
}

void IconView::slotStopItems()
{
    TransferList tl = getSelectedList();
    schedSetCommand( tl, CmdPause );
}

void IconView::slotRemoveItems()
{
    TransferList tl = getSelectedList();
    schedRemoveItems( tl );
}

void IconView::slotSetPriority1()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 1 );
}

void IconView::slotSetPriority2()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 2 );
}

void IconView::slotSetPriority3()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 3 );
}

void IconView::slotSetPriority4()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 4 );
}

void IconView::slotSetPriority5()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 5 );
}

void IconView::slotSetPriority6()
{
    TransferList tl = getSelectedList();
    schedSetPriority( tl, 6 );
}

void IconView::slotSetGroup()
{
    TransferList tl = getSelectedList();
    schedSetGroup( tl, "$TESTGROUP$" );
}


//BEGIN ViewInterface's scheduler notifications 
void IconView::schedulerCleared()
{
    clear();
}

void IconView::schedulerAddedItems( TransferList &tl )
{
    TransferList::iterator it = tl.begin();
    TransferList::iterator endList = tl.end();
    for (; it != endList; ++it)
	new IconViewTransfer( this, (*it) );
}

void IconView::schedulerRemovedItems( TransferList &tl )
{
    if ( !count() )
	return;
    TransferList::iterator it = tl.begin();
    TransferList::iterator endList = tl.end();
    for (; it != endList; ++it)
    {
	Transfer * transfer = *it;
	IconViewTransfer * ivt = static_cast<IconViewTransfer*>(firstItem());
	while ( ivt )
	{
	    IconViewTransfer * next = static_cast<IconViewTransfer*>(ivt->nextItem());
	    if ( ivt->getTransfer() == transfer )
		delete ivt;
	    ivt = next;
	}
    }
}

void IconView::schedulerChangedItems( TransferList &tl )
{
    if ( !count() )
	return;
    TransferList::iterator it = tl.begin();
    TransferList::iterator endList = tl.end();
    for (; it != endList; ++it)
    {
	Transfer * transfer = *it;
	IconViewTransfer * ivt = static_cast<IconViewTransfer*>(firstItem());
	while ( ivt )
	{
	    if ( ivt->getTransfer() == transfer )
		ivt->update();
	    ivt = static_cast<IconViewTransfer*>(ivt->nextItem());
	}
    }
}

void IconView::schedulerStatus( GlobalStatus * )
{
}
//END 



IconViewMdiView::IconViewMdiView( QWidget * parent )
    : KMdiChildView( parent, "IconView-CV" )
{
    QHBoxLayout * mainLay = new QHBoxLayout( this,2 );
     QFrame * ivFrame = new QFrame( this );
     mainLay->addWidget( ivFrame );
      QVBoxLayout * rightVLay = new QVBoxLayout( ivFrame,4,4 );
       iv1 = new IconView( ivFrame );
       iv1->setItemTextPos( QIconView::Right );
       iv1->setLineWidth(1);
       rightVLay->addWidget( new QLabel("Downloading", ivFrame) );
       rightVLay->addWidget( iv1 );
     //ivFrame->setPaletteBackgroundColor( iv1->paletteBackgroundColor());
}
    
void IconViewMdiView::connectToScheduler( Scheduler * s )
{
    iv1->connectToScheduler( s );
}

#include "iconview.moc"
