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

#include <qdragobject.h>
#include <ksystemtray.h>
#include "viewinterface.h"

class KPopupMenu;
class KMainWindow;

/**
  * This class implements the main tray icon for kget. It has a popup
  * from which the user can open a new transfer, configure kget, 
  * minimize/restore or quit the app (default behavior) and drop
  * links to download.
  *
  * @short KGet's system tray widget.
  **/

class DockWidget:public KSystemTray, public ViewInterface
{
Q_OBJECT
public:
    DockWidget( KMainWindow * parent );

    virtual void contextMenuAboutToShow( KPopupMenu * menu );

private slots:
    void mousePressEvent( QMouseEvent * e );

protected:
    // drag and drop
    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );
};

#endif

