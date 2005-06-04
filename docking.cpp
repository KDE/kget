/***************************************************************************
*                               docking.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/

#include <qtooltip.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kurldrag.h>

#include "kmainwidget.h"
#include "settings.h"
#include "docking.h"


DockWidget::DockWidget(KMainWidget * _parent):KSystemTray(_parent)
{
    parent = _parent;

    setPixmap( loadIcon( "kget_dock" ));

    // popup menu for right mouse button
    KPopupMenu *popupMenu = contextMenu();
    parent->action("drop_target")->plug(popupMenu);
    popupMenu->insertSeparator();
    parent->m_paPreferences->plug(popupMenu);

    // Enable dropping
    setAcceptDrops(true);
   
    dtip = new DynamicTip( this );
    dtip->setStatus( kapp->aboutData()->shortDescription() );

}


DockWidget::~DockWidget()
{
   delete dtip;
   dtip = 0;
}


void DockWidget::dragEnterEvent(QDragEnterEvent * event)
{
    event->accept(KURLDrag::canDecode(event)
                  || QTextDrag::canDecode(event));
}


void DockWidget::dropEvent(QDropEvent * event)
{
    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list)) {
        parent->addTransfers(list);
    } else if (QTextDrag::decode(event, str)) {
        parent->addTransfer(str);
    }
}


void DockWidget::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == MidButton) {
        parent->slotPasteTransfer();
    } else {
        KSystemTray::mousePressEvent(e);
    }
}


void DockWidget::updateToolTip( const QString& _status )
{
   dtip->setStatus( _status );
}


DynamicTip::DynamicTip( QWidget * parent )
   : QToolTip( parent )
{
   // no explicit initialization needed
}


void DynamicTip::setStatus( const QString & _status )
{
   status = _status;
}

void DynamicTip::maybeTip( const QPoint & _pos )
{
   QRect r( parentWidget()->rect() );
   tip( r, status );
}

#include "docking.moc"
