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

#include <qpainter.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurldrag.h>

#include "kmainwidget.h"
#include "settings.h"
#include "docking.h"

#define DOCK_ICONWIDTH 24
#define DOCK_ICONHEIGHT 24


DockWidget::DockWidget(KMainWidget * _parent):KSystemTray(_parent)
{
    parent = _parent;

    QPixmap *tmppix = new QPixmap();

    tmppix->load(locate("appdata", "pics/dock.png"));

    handpix1 = new QPixmap(DOCK_ICONWIDTH, DOCK_ICONHEIGHT);
    handpix1->fill(backgroundColor());
    bitBlt(handpix1, 0, 0, tmppix);

    handpix2 = new QPixmap(DOCK_ICONWIDTH, DOCK_ICONHEIGHT);
    handpix2->fill(backgroundColor());
    bitBlt(handpix2, 0, 0, tmppix);

    handpix3 = new QPixmap(DOCK_ICONWIDTH, DOCK_ICONHEIGHT);
    handpix3->fill(backgroundColor());
    bitBlt(handpix3, 0, 0, tmppix);

    delete tmppix;

    setPixmap(*handpix1);

    for (int i = 0; i < 3; i++) {
        size[i] = 0;
    }

    // popup menu for right mouse button
    KPopupMenu *popupMenu = contextMenu();
    parent->m_paPreferences->plug(popupMenu);


    // Enable dropping
    setAcceptDrops(true);

}


DockWidget::~DockWidget()
{
    delete handpix1;
    delete handpix2;
    delete handpix3;
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


void DockWidget::contextMenuAboutToShow ( KPopupMenu* menu ){

    menu->connectItem( menu->idAt(4), kmain, SLOT(slotQuit()));

}

#include "docking.moc"
