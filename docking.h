/***************************************************************************
*                                  docking.h
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



#ifndef _DOCKING_H_
#define _DOCKING_H_

#include <stdio.h>

#include <qdragobject.h>

#include <ksystemtray.h>

class KPopupMenu;
class KMainWidget;

class DynamicTip : public QToolTip
{
   public:
       DynamicTip( QWidget * parent );
       virtual ~DynamicTip() {}//TODO workaround for qt-bug, can be removed after 4.0 
       void setStatus( const QString & _status );

   protected:
       void maybeTip( const QPoint & );
       
   private:
       QString status;
};

class DockWidget:public KSystemTray
{

Q_OBJECT public:
    DockWidget(KMainWidget * parent);
    ~DockWidget();
    /** No descriptions */
    virtual void contextMenuAboutToShow ( KPopupMenu* menu );
    void updateToolTip( const QString& );



private slots:
    void mousePressEvent(QMouseEvent * e);

protected:
    // drag and drop
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private:
    KMainWidget *parent;
   DynamicTip * dtip;

};

#endif
