/***************************************************************************
*                                droptarget.cpp
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

#include <qpainter.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kwin.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kurldrag.h>

#include "kmainwidget.h"
#include <qcursor.h>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>
#undef Bool
#undef Status
#endif
#include "settings.h"
#include "droptarget.h"

DropTarget::DropTarget():QWidget()
{
    int x = ksettings.dropPosition.x();
    int y = ksettings.dropPosition.y();
    
    QRect desk = KGlobalSettings::desktopGeometry(this);
    QPixmap bgnd = UserIcon( "target" );

    if (x != -1 &&
        x >= desk.left() && y >= desk.top() &&
        (x + bgnd.width()) <= desk.right() &&
        (y + bgnd.height()) <= desk.bottom() )
    {
        move(ksettings.dropPosition);
        resize(bgnd.width(), bgnd.height());
        KWin::setState(winId(), ksettings.dropState);
    }
    else
    {
        setGeometry(desk.x()+200, desk.y()+200, bgnd.width(), bgnd.height());
        KWin::setState(winId(), NET::SkipTaskbar | NET::StaysOnTop);
    }

    b_sticky = ksettings.dropState & NET::Sticky;

    // setup pixmaps

    if (!bgnd.mask())
        kdError(5001) << "Drop target pixmap has no mask!\n";
    else
        mask = *bgnd.mask();

    setBackgroundPixmap( bgnd );

    // popup menu for right mouse button
    popupMenu = new KPopupMenu();
    popupMenu->insertTitle(kapp->caption());
    popupMenu->setCheckable(true);

    pop_Max = popupMenu->insertItem(i18n("Maximize"), this, SLOT(toggleMinimizeRestore()));
    pop_Min = popupMenu->insertItem(i18n("Minimize"), this, SLOT(toggleMinimizeRestore()));

    pop_sticky = popupMenu->insertItem(i18n("Sticky"), this, SLOT(toggleSticky()));
    popupMenu->setItemChecked(pop_sticky, b_sticky);
    kmain->m_paPreferences->plug(popupMenu);
    popupMenu->insertSeparator();
    kmain->m_paQuit->plug(popupMenu);

    // Enable dropping
    setAcceptDrops(true);
}


DropTarget::~DropTarget()
{
    delete popupMenu;
}


void
DropTarget::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == LeftButton)
    {
        // toggleMinimizeRestore ();
        oldX = 0;
        oldY = 0;

    }
    else if (e->button() == RightButton)
    {
        popupMenu->setItemEnabled(pop_Min, kmain->isVisible());
        popupMenu->setItemEnabled(pop_Max, kmain->isHidden());

        popupMenu->popup(QCursor::pos());
    }
    else if (e->button() == MidButton)
    {
        kmain->slotPasteTransfer();
    }
}


void DropTarget::resizeEvent(QResizeEvent *)
{
#ifdef Q_WS_X11
    XShapeCombineMask(x11Display(), winId(), ShapeBounding, 0, 0, mask.handle(), ShapeSet);
#endif
}


void DropTarget::dragEnterEvent(QDragEnterEvent * event)
{
    event->accept(KURLDrag::canDecode(event)
                  || QTextDrag::canDecode(event));
}


void DropTarget::dropEvent(QDropEvent * event)
{
    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list))
    {
        kmain->addTransfers(list);
    }
    else if (QTextDrag::decode(event, str))
    {
        kmain->addTransfer(str);
    }
}


void DropTarget::toggleSticky()
{
    b_sticky = !b_sticky;
    popupMenu->setItemChecked(pop_sticky, b_sticky);
    updateStickyState();
}

void DropTarget::updateStickyState()
{
    if (b_sticky)
    {
        KWin::setState(winId(), NET::SkipTaskbar | NET::StaysOnTop | NET::Sticky);
    }
    else
    {
        KWin::clearState(winId(), NET::Sticky);
    }
}

void DropTarget::toggleMinimizeRestore()
{
    if (kmain->isVisible())
        kmain->hide();
    else
        kmain->show();
}

/** No descriptions */
void DropTarget::mouseMoveEvent(QMouseEvent * e)
{
    if (oldX == 0)
    {
        oldX = e->x();
        oldY = e->y();
        return;
    }

    QWidget::move(x() + (e->x() - oldX), y() + (e->y() - oldY));
}

/** No descriptions */
void DropTarget::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (e->button() == LeftButton)
        toggleMinimizeRestore();
}

#include "droptarget.moc"
