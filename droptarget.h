/***************************************************************************
*                                droptarget.h
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

#ifndef _DROPTARGET_H
#define _DROPTARGET_H

#include <qwidget.h>
#include <qbitmap.h>
#include <qdragobject.h>

#include <kpopupmenu.h>

class KMainWidget;

class DropTarget:public QWidget
{

Q_OBJECT public:
        DropTarget();
        ~DropTarget();



protected:
        virtual void resizeEvent(QResizeEvent *);

        // drag and drop
        void dragEnterEvent(QDragEnterEvent *);
        /** No descriptions */
        virtual void mouseDoubleClickEvent(QMouseEvent * e);
        void dropEvent(QDropEvent *);
        /** No descriptions */
        virtual void mouseMoveEvent(QMouseEvent *);

private slots:
        void toggleSticky();

        void mousePressEvent(QMouseEvent * e);
        void toggleMinimizeRestore();

private:
        KPopupMenu * popupMenu;
        KMainWidget *parent;

        bool b_sticky;

        int pop_sticky;
        int pop_Max;
        int pop_Min;

        int size[4];

        QPixmap *handpix1;
        QPixmap *handpix2;
        QPixmap *handpix3;

        QBitmap mask;
public:                      // Public attributes
        /**  */
        int oldX;
        int oldY;
};

#endif                          // _DROPTARGET_H
