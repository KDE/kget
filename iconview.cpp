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
#include <qpalette.h>
#include <kpopupmenu.h>
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
    QPixmap compostedPix = mimePix;
    int width = compostedPix.width(),
        height = compostedPix.height();
    if ( width < 20 || height < 10 )
	return;
    
    //draw the overlay bar
    QPainter p( &compostedPix );
    p.setPen( Qt::black );
    p.setBrush( Qt::white );
    p.drawRect( 5, height - 8, width - 10, 7 );
    int percent = transfer->getPercent();
    if ( percent > 100 )
	percent = 100;
    if ( percent < 0 )
	percent = 0;
    width = (width * percent) / 100;
    p.setPen( Qt::white );
    p.setBrush( transfer->getStatus() == Transfer::ST_RUNNING ? Qt::green : Qt::red );
    p.drawRect( 6, height - 7, width, 5 );
    p.end();

    setPixmap( compostedPix );
//    repaint();
}
 
Transfer * IconViewTransfer::getTransfer()
{
    return transfer;
}

void IconViewTransfer::paintFocus( QPainter * p, const QColorGroup & cg )
{
    QColorGroup inverted = cg;
    inverted.setColor( QColorGroup::Background, cg.foreground() );
    inverted.setColor( QColorGroup::Foreground, cg.background() );
    paintItem( p, cg );
}


/*
* KGetIconView
*
*/
// 	void schedNewURLs( const KURL::List &, const QString &destDir );
// 	void schedRemoveItems( TransferList & );
// 	void schedSetPriority( TransferList &, int );
// 	void schedSetCommand( TransferList &, TransferCommand );
// 	void schedSetGroup( TransferList &, const QString & );

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
	Transfer * transfer = ivt->getTransfer();
	if ( !transfer )
	    overItems = false;
    }
    
    KPopupMenu * popup = new KPopupMenu( this );
    
    //insert title
    QFont boldFont = popup->font();
    boldFont.setBold( true );
    QLabel * lab = new QLabel( overItems ? "Transfer operations Menu" : "KGet Menu", popup );
    lab->setFont( boldFont );
    popup->insertItem( (QWidget*)lab );
    
    //popup menu construction
    if ( overItems )
    {
	//popup->insertItem( transfer->getDest().path() );
	popup->insertItem( "Remove", this, SLOT(slotRemoveItems()) );
	popup->insertItem( "Set prio >", this, SLOT(slotSetPriority()) );
	popup->insertItem( "Operation >", this, SLOT(slotSetOperation()) );
	popup->insertItem( "set group >", this, SLOT(slotSetGroup()) );
	popup->insertItem( "... FIXME: implement ..." );
    }
    else
    {
	popup->insertItem( "New transfer ...", this, SLOT(slotNewTransfer()) );
    }
    popup->popup( pos );
}

void IconView::slotNewTransfer()
{
    schedNewURLs( KURL(), "/root" );
}

void IconView::slotRemoveItems()
{
}

void IconView::slotSetPriority()
{
}

void IconView::slotSetOperation()
{
}

void IconView::slotSetGroup()
{
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

#include "iconview.moc"
