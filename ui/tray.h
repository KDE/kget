/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef _TRAY_H
#define _TRAY_H

#include <QPixmap>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QDropEvent>

#include <ksystemtray.h>

#include "core/observer.h"

class KGet;
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

class Tray : public KSystemTray, public ModelObserver
{
Q_OBJECT
public:
    Tray( KGet * parent );
    ~Tray();

    void setDownloading( bool );

protected:
    // drag and drop
    void dragEnterEvent( QDragEnterEvent * );
    void dropEvent( QDropEvent * );

private:
    /**
     * Repaints trayIcon showing progress (and overlay if present)
     */
    void paintIcon( int mergePixels = -1, bool force = false );
    /**
     * Blend an overlay icon over 'sourcePixmap' and repaint trayIcon
     */
    void blendOverlay( QPixmap * sourcePixmap );
    
    QTimer * blinkTimer;
    QPixmap *baseIcon, *grayedIcon, *alternateIcon;
    QPixmap *playOverlay, *stopOverlay;
    QPixmap *overlay;   //!< The current overlay (may be NULL)
    bool iconOn;
    bool overlayVisible;

private slots:
    void mousePressEvent( QMouseEvent * e );
    void slotTimeout();
};

#endif

