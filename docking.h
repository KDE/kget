/***************************************************************************
                                docking.h
                             -------------------
    Revision 				: $Id$
    begin						: Tue Jan 29 2002
    copyright				: (C) 2002 by Patrick Charbonnier
									: Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
    email						: pch@freeshell.og
 ***************************************************************************/

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



#ifndef _DOCKING_H_
#define _DOCKING_H_

#include <stdio.h>

#include <qpopupmenu.h>
#include <qdragobject.h>

#include <kpopupmenu.h>
#include <kdockwindow.h>

class KMainWidget;

class DockWidget:public KDockWindow
{

Q_OBJECT public:
        DockWidget(KMainWidget * parent);
        ~DockWidget();

        void setAnim(int i1, int i2, int i3, bool online);

private slots:
        void mousePressEvent(QMouseEvent * e);

protected:
        // drag and drop
        void dragEnterEvent(QDragEnterEvent *);
        void dropEvent(QDropEvent *);

private:
        int size[3];
        KMainWidget *parent;

        QPixmap *handpix1;
        QPixmap *handpix2;
        QPixmap *handpix3;

};

#endif
