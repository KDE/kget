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

#include <kapplication.h>
#include <kmainwindow.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kaction.h>
#include <qtooltip.h>
#include <qtimer.h>

#include "docking.h"
#include "kmainwidget.h"

/** class DockWidget
  * Reimplmentation of the system tray class adding drag/drop
  * capabilities and the quit action.
  */
DockWidget::DockWidget(KMainWidget * parent)
    : KSystemTray(parent), ViewInterface(), timer( 0 )
{
    setPixmap( loadIcon( "dock" ));

    // add preferences action to the context menu
    parent->actionCollection()->action("open_transfer")->plug(contextMenu());
    parent->actionCollection()->action("preferences")->plug(contextMenu());

    // enable dropping
    setAcceptDrops(true);

    // add tooltip telling "I'm kget"
    QToolTip::add( this, kapp->aboutData()->shortDescription() );
}

// dtor: delete internal classes
DockWidget::~DockWidget()
{
    delete timer;
}

// test if dropped thing can be handled (must be an URLlist or a QString)
void DockWidget::dragEnterEvent(QDragEnterEvent * event)
{
    event->accept(KURLDrag::canDecode(event)
                  || QTextDrag::canDecode(event));
}

// decode the dropped element asking scheduler to download that
void DockWidget::dropEvent(QDropEvent * event)
{
    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list))
        schedNewURLs(list, QString::null);
    else if (QTextDrag::decode(event, str))
        schedNewURLs(KURL::fromPathOrURL(str), QString::null);
}

// filter middle mouse clicks to ask scheduler to paste URL
void DockWidget::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == MidButton)
        schedRequestOperation( OpPasteTransfer );
    else
        KSystemTray::mousePressEvent(e);
}

// connect the 4th menu entry ("quit") to QApplication::quit()
void DockWidget::contextMenuAboutToShow ( KPopupMenu* menu )
{
    menu->connectItem( menu->idAt(5), kapp, SLOT(quit()));
}

// display blinking icon when downloading
void DockWidget::setDownloading( bool blink )
{
    if ( !blink )
    {
        if( timer )
            timer->stop();
        setPixmap( loadIcon( "dock" ));
    }
    else
    {
        if ( !timer )
        {
            timer = new QTimer;
            connect( timer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
        }
        if ( !timer->isActive() )
        {
            iconOn = false;
            setPixmap( loadIcon( "dock_off" ));
        }
        timer->start( 1000 );
    }
}

// slot executed every 1s: toggle icon pixmap
void DockWidget::slotTimeout()
{
    iconOn = !iconOn;
    setPixmap( loadIcon( iconOn ? "dock_on" : "dock_off" ));
}

#include "docking.moc"
