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
class KMainWidget;
class QTimer;
class QPixmap;

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
    DockWidget( KMainWidget * parent );
    ~DockWidget();
    
    void setDownloading( bool );

protected:
    // drag and drop
    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );

    // hook to mangle popup connections before display
    virtual void contextMenuAboutToShow( KPopupMenu * menu );

private:
    // repaints trayIcon showing progress (and overlay if present)
    void paintIcon( int mergePixels = -1, bool force = false );
    // blend an overlay icon over 'sourcePixmap' and repaint trayIcon
    void blendOverlay( QPixmap * sourcePixmap );
    
    QTimer * blinkTimer;
    QPixmap *baseIcon, *grayedIcon, *alternateIcon;
    QPixmap *playOverlay, *stopOverlay;
    QPixmap *overlay;   // the current overlay (may be NULL)
    bool iconOn;
    bool overlayVisible;

private slots:
    void mousePressEvent( QMouseEvent * e );
    void slotTimeout();
};

#endif

