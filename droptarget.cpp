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
#include <kstandarddirs.h>
#include <kwin.h>
#include <klocale.h>
#include <kurldrag.h>

#include "kmainwidget.h"
#include <qcursor.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>
#undef Bool
#undef Status
#include "settings.h"
#include "droptarget.h"
#define TARGET_WIDTH   70
#define TARGET_HEIGHT  69


DropTarget::DropTarget():QWidget()
{

    if (ksettings.dropPosition.x() != -1)
    {
        move(ksettings.dropPosition);
        KWin::setState(winId(), ksettings.dropState);
    }
    else
    {
        setGeometry(200, 200, TARGET_WIDTH, TARGET_HEIGHT);
        KWin::setState(winId(), NET::SkipTaskbar | NET::StaysOnTop);
    }

    b_sticky = ksettings.dropState & NET::Sticky;

    // setup mask
    mask.resize(TARGET_WIDTH, TARGET_HEIGHT);
    mask.fill(color0);

    QPainter p2;

    p2.begin(&mask);
    p2.setBrush(color1);
    p2.drawChord( 0, 0,TARGET_WIDTH,TARGET_HEIGHT,5760,5760);
    p2.end();

    // setup pixmaps

    int offsetx = -10;
    int offsety = -5;

    QPixmap *tmppix = new QPixmap();
    tmppix->load(locate("appdata", "pics/target.png"));

    handpix1 = new QPixmap(TARGET_WIDTH, TARGET_HEIGHT);
    handpix1->fill(backgroundColor());
    bitBlt(handpix1, offsetx, offsety, tmppix);

    handpix2 = new QPixmap(TARGET_WIDTH, TARGET_HEIGHT);
    handpix2->fill(backgroundColor());
    bitBlt(handpix2, offsetx, offsety, tmppix);

    handpix3 = new QPixmap(TARGET_WIDTH, TARGET_HEIGHT);
    handpix3->fill(backgroundColor());
    bitBlt(handpix3, offsetx, offsety, tmppix);

    setBackgroundPixmap(*handpix1);

    // popup menu for right mouse button
    popupMenu = new KPopupMenu();
    popupMenu->setTitle(kapp->caption());
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

    delete tmppix;

}


DropTarget::~DropTarget()
{
    delete handpix1;
    delete handpix2;
    delete handpix3;
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
    XShapeCombineMask(x11Display(), winId(), ShapeBounding, 0, 0, mask.handle(), ShapeSet);
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
        kmain->addDropTransfers(list);
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

#include "droptarget.moc"

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
