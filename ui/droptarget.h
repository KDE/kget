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
#include <qdragobject.h>

#include "core/viewinterface.h"

class KPopupMenu;
class KMainWidget;
class QTimer;

class DropTarget:public QWidget, public ViewInterface
{
Q_OBJECT 

public:
    DropTarget(KMainWidget * parent);
    ~DropTarget();

    void updateStickyState();
    void playAnimation();
    void playAnimationHide();
    void playAnimationSync();
    void setShown( bool shown, bool internal = true );
   
protected:
    // drag and drop
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent * e);
    virtual void dropEvent(QDropEvent *);

    // handle quit events as hide events
    virtual void closeEvent( QCloseEvent * );
    
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent * e);
    virtual void mouseReleaseEvent(QMouseEvent *);

private slots:
    void toggleSticky();
    void toggleMinimizeRestore();
    void slotStartStopToggled( bool );
    void slotAnimate();
    void slotAnimateHide();
    void slotAnimateSync();
    void slotClose();

private:
    KPopupMenu * popupMenu;
    QWidget * parentWidget;
    QTimer * animTimer;

    int pop_sticky;
    int pop_show;

//    int oldX;
//    int oldY;
    int dx;
    int dy;
    bool isdragging;
    
    float ani_y, ani_vy;
};

#endif                          // _DROPTARGET_H
