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

#include <kaction.h>
#include <kmainwindow.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kiconeffect.h>
#include <stdlib.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qbitmap.h>
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
#define TARGET_WIDTH   68
#define TARGET_HEIGHT  67


DropTarget::DropTarget(KMainWindow * mw)
    : QWidget(), ViewInterface(), parentWidget((QWidget*)mw)
{
    int x = ksettings.dropPosition.x();
    int y = ksettings.dropPosition.y();
    
    QRect desk = KGlobalSettings::desktopGeometry(this);

    if (x != -1 &&
        x >= desk.left() && y >= desk.top() &&
        (x + TARGET_WIDTH) <= desk.right() &&
        (y + TARGET_HEIGHT) <= desk.bottom() )
    {
        move(ksettings.dropPosition);
        KWin::setState(winId(), ksettings.dropState);
    }
    else
    {
        setGeometry(desk.x()+200, desk.y()+200, TARGET_WIDTH, TARGET_HEIGHT);
        KWin::setState(winId(), NET::SkipTaskbar | NET::StaysOnTop);
    }

    b_sticky = ksettings.dropState & NET::Sticky;

    // setup mask
    QBitmap mask(TARGET_WIDTH, TARGET_HEIGHT);
    mask.fill(color0);
    QPainter p2;
    p2.begin(&mask);
    p2.setBrush(color1);
    p2.drawChord(0, 0, TARGET_WIDTH, TARGET_HEIGHT, 5760, 5760);
    p2.end();
    setMask( mask );

    // setup pixmaps
    int offsetx = -11;
    int offsety = -6;

    QPixmap bgnd = QPixmap(TARGET_WIDTH, TARGET_HEIGHT);
    bgnd.fill( Qt::white );
    QPixmap tmp = UserIcon( "target" );
    bitBlt(&bgnd, offsetx, offsety, &tmp );

    /* The following code was adapted from kdebase/kicker/ui/k_mnu.cpp
     * It paints a tint over the kget arrow taking the tint color from
     * active or inactive window title colors
     */
    KConfig *config = KGlobal::config();
    QColor color = palette().active().highlight();

    config->setGroup("WM");
    QColor activeTitle = config->readColorEntry("activeBackground", &color);
    QColor inactiveTitle = config->readColorEntry("inactiveBackground", &color);

    // figure out which color is most suitable for recoloring to
    int h1, s1, v1, h2, s2, v2, h3, s3, v3;
    activeTitle.hsv(&h1, &s1, &v1);
    inactiveTitle.hsv(&h2, &s2, &v2);
    palette().active().background().hsv(&h3, &s3, &v3);

    if ( (abs(h1-h3)+abs(s1-s3)+abs(v1-v3) < abs(h2-h3)+abs(s2-s3)+abs(v2-v3)) &&
         ((abs(h1-h3)+abs(s1-s3)+abs(v1-v3) < 32) || (s1 < 32)) && (s2 > s1))
        color = inactiveTitle;
    else
        color = activeTitle;

    // limit max/min brightness
    int h, s, v;
    color.getHsv( &h, &s, &v );
    if (v > 180)
        color.setHsv( h, s, 180 );
    else if (v < 76 )
        color.setHsv( h, s, 76 );

    QImage image = bgnd.convertToImage();
    KIconEffect::colorize( image, color, 1.0 );
    bgnd.convertFromImage( image );
    setErasePixmap( bgnd );

    // popup menu for right mouse button
    popupMenu = new KPopupMenu();
    popupMenu->insertTitle(mw->caption());
    popupMenu->setCheckable(true);

    pop_Max = popupMenu->insertItem(i18n("Maximize"), this, SLOT(toggleMinimizeRestore()));
    pop_Min = popupMenu->insertItem(i18n("Minimize"), this, SLOT(toggleMinimizeRestore()));
    pop_sticky = popupMenu->insertItem(i18n("Sticky"), this, SLOT(toggleSticky()));
    popupMenu->setItemChecked(pop_sticky, b_sticky);
    mw->actionCollection()->action("preferences")->plug(popupMenu);
    popupMenu->insertSeparator();
    mw->actionCollection()->action("quit")->plug(popupMenu);

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
        popupMenu->setItemEnabled(pop_Min, parentWidget->isVisible());
        popupMenu->setItemEnabled(pop_Max, parentWidget->isHidden());
        popupMenu->popup(QCursor::pos());
    }
    else if (e->button() == MidButton)
        schedRequestOperation( OpPasteTransfer );
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
	schedNewURLs( list, QString::null );
    else if (QTextDrag::decode(event, str))
	schedNewURLs( KURL(str), QString::null );
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
    if (parentWidget->isVisible())
        parentWidget->hide();
    else
        parentWidget->show();
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
