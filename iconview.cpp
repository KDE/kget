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
    w = ((w - 10) * percent) / 100;
    p->setPen( Qt::white );
    p->setBrush( transfer->getStatus() == Transfer::ST_RUNNING ? Qt::green : Qt::red );
    p->drawRect( r.left() + 6, r.top() + h - 7, w, 5 );
    p->restore();
}

void IconViewTransfer::paintFocus( QPainter * p, const QColorGroup & cg )
{
/*  No focus is better than colored focus
    QColorGroup inverted = cg;
    inverted.setColor( QColorGroup::Background, cg.foreground() );
    inverted.setColor( QColorGroup::Foreground, cg.background() );
    paintItem( p, inverted );*/
}


/*
* KGetIconView
*
*/

IconView::IconView( Scheduler * s, QWidget * parent, const char * name )
    : QIconView( parent, name ), ViewInterface( s )
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
    QFont boldFont = popup->font();
    boldFont.setBold( true );
    QLabel * lab = new QLabel( overItems ? "Transfer operations Menu" : "KGet Menu", popup );
    lab->setFont( boldFont );
    popup->insertItem( (QWidget*)lab );
    
    // add menu entries
    if ( overItems )
    {   // menu over an item
	popup->insertItem( new QLabel(transfer->getDest().path(), popup) );
	popup->insertItem( SmallIcon("remove"), i18n("Remove"), this, SLOT(slotRemoveItems()) );
	
	KPopupMenu * subPrio = new KPopupMenu( popup );
	subPrio->insertItem( SmallIcon("2uparrow"), i18n("highest"), this,  SLOT( slotSetPriority() ) );
	subPrio->insertItem( SmallIcon("1uparrow"), i18n("highe"), this,  SLOT( slotSetPriority() ) );
	subPrio->insertItem( i18n("normal"), this,  SLOT( slotSetPriority() ) );
	subPrio->insertItem( SmallIcon("1downarrow"), i18n("low"), this,  SLOT( slotSetPriority() ) );
	subPrio->insertItem( SmallIcon("2downarrow"), i18n("lowest"), this,  SLOT( slotSetPriority() ) );
	popup->insertItem( i18n("Set priority"), subPrio );
	
	KPopupMenu * subOp = new KPopupMenu( popup );
	subOp->insertItem( i18n("resume"), this,  SLOT( slotSetCommand() ) );
	popup->insertItem( SmallIcon("forward"), i18n("Operation"), subOp );
	
	KPopupMenu * subGroup = new KPopupMenu( popup );
	//for loop inserting all existant groups
	subGroup->insertItem( i18n("new ..."), this,  SLOT( slotSetGroup() ) );	
	popup->insertItem( SmallIcon("folder"), i18n("Set group ..."), subGroup );
    }
    else
	// menu on empty space
	popup->insertItem( "New transfer ...", this, SLOT(slotNewTransfer()) );

    // show popup
    popup->popup( pos );
}

void IconView::slotNewTransfer()
{
    schedNewURLs( KURL(), "/root" );
}

void IconView::slotRemoveItems()
{
// 	void schedRemoveItems( TransferList & );
}

void IconView::slotSetPriority()
{
// 	void schedSetPriority( TransferList &, int );
}

void IconView::slotSetCommand()
{
// 	void schedSetCommand( TransferList &, TransferCommand );
}

void IconView::slotSetGroup()
{
// 	void schedSetGroup( TransferList &, const QString & );
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



IconViewMdiView::IconViewMdiView( Scheduler * s, QWidget * parent, const char * name )
    : KMdiChildView( parent, name )
{
    QHBoxLayout * mainLay = new QHBoxLayout( this,2 );
     QFrame * descFrame = new QFrame( this );
     mainLay->addWidget( descFrame );
      QVBoxLayout * leftVLay = new QVBoxLayout( descFrame,4,4 );
       descFrame->setMinimumWidth( 150 );
       leftVLay->addWidget( new QLabel("Transfer:", descFrame) );
     QFrame * ivFrame = new QFrame( this );
     mainLay->addWidget( ivFrame );
      QVBoxLayout * rightVLay = new QVBoxLayout( ivFrame,4,4 );
       IconView * iv1 = new IconView( s, ivFrame );
       iv1->setLineWidth(1);
       IconView * iv2 = new IconView( s, ivFrame );
       iv2->setLineWidth(1);
       iv2->setItemTextPos( QIconView::Right );
       rightVLay->addWidget( new QLabel("Downloading", ivFrame) );
       rightVLay->addWidget( iv1 );
       rightVLay->addWidget( new QLabel("Next in chain", ivFrame) );
       rightVLay->addWidget( iv2 );
//       ivFrame->setPaletteBackgroundColor( iv2->paletteBackgroundColor());
}

#include "iconview.moc"
