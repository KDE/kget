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

#include <kapp.h>
#include <kstddirs.h>
#include <klocale.h>

#include "kmainwidget.h"
#include "settings.h"
#include "docking.h"

#define ICONWIDTH 24
#define ICONHEIGHT 24


DockWidget::DockWidget(KMainWidget * _parent):KDockWindow(_parent)
{

    parent = _parent;

    // TODO CHECK the path
    QString path = "kget/pics/";

    QPixmap *tmppix = new QPixmap();

    tmppix->load(locate("data", path + "dock_hand1.xpm"));

    handpix1 = new QPixmap(ICONWIDTH, ICONHEIGHT);
    handpix1->fill(backgroundColor());
    bitBlt(handpix1, 0, 0, tmppix);

    delete tmppix;

    tmppix = new QPixmap();
    tmppix->load(locate("data", path + "dock_hand2.xpm"));

    handpix2 = new QPixmap(ICONWIDTH, ICONHEIGHT);
    handpix2->fill(backgroundColor());
    bitBlt(handpix2, 0, 0, tmppix);

    delete tmppix;

    tmppix = new QPixmap();
    tmppix->load(locate("data", path + "dock_hand3.xpm"));

    handpix3 = new QPixmap(ICONWIDTH, ICONHEIGHT);
    handpix3->fill(backgroundColor());
    bitBlt(handpix3, 0, 0, tmppix);

    delete tmppix;

    setPixmap(*handpix1);

    for (int i = 0; i < 3; i++) {
	size[i] = 0;
    }

    // popup menu for right mouse button
    KPopupMenu *popupMenu = contextMenu();

    popupMenu->insertItem(i18n("Preferences"), parent, SLOT(slotPreferences()));

    // Enable dropping
    setAcceptDrops(true);

}


DockWidget::~DockWidget()
{
    delete handpix1;
    delete handpix2;
    delete handpix3;
}


void
 DockWidget::setAnim(int i1, int i2, int i3, bool online)
{

    size[0] = i1;
    size[1] = i2;
    size[2] = i3;

    if (isVisible()) {
	if (!online || ksettings.b_offlineMode) {
	    setPixmap(*handpix3);
	} else if (size[0] == 0 && size[1] == 0 && size[2] == 0) {
	    setPixmap(*handpix1);
	} else {
	    QPixmap pm(*handpix2);
	    QPainter p;

	    p.begin(&pm);

	    p.setPen(white);
	    for (int i = 0; i < 3; i++) {
		if (size[i] != 0) {
		    int pixels = (int) (ICONWIDTH * (float) size[i] / 100.0);

		    p.fillRect(1, i * 8, pixels, 7, blue);
		}
	    }

	    p.end();
	    setPixmap(pm);
	}
    }
}


void DockWidget::dragEnterEvent(QDragEnterEvent * event)
{
    event->accept(QUriDrag::canDecode(event)
		  || QTextDrag::canDecode(event));
}


void DockWidget::dropEvent(QDropEvent * event)
{
    QStrList list;
    QString str;

    if (QUriDrag::decode(event, list)) {
	parent->addDropTransfers(&list);
    } else if (QTextDrag::decode(event, str)) {
	parent->addTransfer(str);
    }
}


void DockWidget::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == MidButton) {
	parent->slotPasteTransfer();
    } else {
	KDockWindow::mousePressEvent(e);
    }
}

#include "docking.moc"
