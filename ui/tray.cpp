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

#include <kapplication.h>
#include <kmainwindow.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kpopupmenu.h>
#include <kurldrag.h>
#include <kaction.h>
#include <qtooltip.h>
#include <qtimer.h>

#include "tray.h"
#include "kget.h"

/** class Tray
  * Reimplmentation of the system tray class adding drag/drop
  * capabilities and the quit action.
  */
Tray::Tray(KMainWidget * parent)
    : KSystemTray(parent), 
      ViewInterface(), 
      blinkTimer( 0 ),
      overlay( 0 ),
      alternateIcon( 0 ),
      grayedIcon( 0 ),
      overlayVisible( false )
{
    baseIcon = new QPixmap( KSystemTray::loadIcon("dock") );
    playOverlay = new QPixmap( SmallIcon( "dock_overlay_run" ) );
    stopOverlay = new QPixmap( SmallIcon( "dock_overlay_stop" ) );
    
    paintIcon();

    // add preferences action to the context menu
    parent->actionCollection()->action("open_transfer")->plug(contextMenu());
    parent->actionCollection()->action("preferences")->plug(contextMenu());

    // enable dropping
    setAcceptDrops(true);

    // add tooltip telling "I'm kget"
    QToolTip::add( this, kapp->aboutData()->shortDescription() );
}

// dtor: delete internal classes
Tray::~Tray()
{
    delete blinkTimer;
}

// test if dropped thing can be handled (must be an URLlist or a QString)
void Tray::dragEnterEvent(QDragEnterEvent * event)
{
    event->accept(KURLDrag::canDecode(event)
                  || QTextDrag::canDecode(event));
}

// decode the dropped element asking scheduler to download that
void Tray::dropEvent(QDropEvent * event)
{
    KURL::List list;
    QString str;

    if (KURLDrag::decode(event, list))
        schedNewURLs(list, QString::null);
    else if (QTextDrag::decode(event, str))
        schedNewURLs(KURL::fromPathOrURL(str), QString::null);
}

// filter middle mouse clicks to ask scheduler to paste URL
void Tray::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == MidButton)
        schedRequestOperation( OpPasteTransfer );
    else
        KSystemTray::mousePressEvent(e);
}

// connect the 4th menu entry ("quit") to QApplication::quit()
void Tray::contextMenuAboutToShow ( KPopupMenu* menu )
{
    menu->connectItem( menu->idAt(5), kapp, SLOT(quit()));
}

// display blinking icon when downloading
void Tray::setDownloading( bool running )
{
    if(!blinkTimer)
    {
        blinkTimer = new QTimer;
        connect( blinkTimer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
    }
    
    overlayVisible = true;
    
    if(running)
    {
        overlay = playOverlay;
        blinkTimer->start( 1500 );  // start 'blink' timer
        paintIcon( 50, true );
    }
    else
    {        
        overlay = stopOverlay;
        blinkTimer->start( 1500 );  // start 'hide' timer
        paintIcon( 50, true );
    }
}

// slot executed every 1s: toggle icon pixmap
void Tray::slotTimeout()
{
    if ( overlay == playOverlay )
    {
        overlayVisible = !overlayVisible;
        paintIcon( 50/*mergeLevel*/, true );
    }
    else if( overlay == stopOverlay )
    {
        overlay = 0;
        blinkTimer->stop();
        paintIcon( -1, true );
        overlayVisible = false;
    }
}

void Tray::paintIcon( int mergePixels, bool force )
{
    // skip redrawing the same pixmap
    static int mergePixelsCache = 0;
    if ( mergePixels == mergePixelsCache && !force )
         return;
    mergePixelsCache = mergePixels;

    if ( mergePixels < 0 )
        return blendOverlay( baseIcon );
        
    // make up the grayed icon
    if ( !grayedIcon )
    {
        QImage tmpTrayIcon = baseIcon->convertToImage();
        KIconEffect::semiTransparent( tmpTrayIcon );
        grayedIcon = new QPixmap( tmpTrayIcon );
    }
    if ( mergePixels == 0 )
        return blendOverlay( grayedIcon );
    
    // make up the alternate icon (use hilight color but more saturated)
    if ( !alternateIcon )
    {
        QImage tmpTrayIcon = baseIcon->convertToImage();
        // eros: this looks cool with dark red blue or green but sucks with
        // other colors (such as kde default's pale pink..). maybe the effect
        // or the blended color has to be changed..
        QColor saturatedColor = palette().active().highlight();
        int hue, sat, value;
        saturatedColor.getHsv( &hue, &sat, &value );
        saturatedColor.setHsv( hue, (sat + 510) / 3, value );
        KIconEffect::colorize( tmpTrayIcon, saturatedColor/* Qt::blue */, 0.9 );
        alternateIcon = new QPixmap( tmpTrayIcon );
    }
    if ( mergePixels >= alternateIcon->height() )
        return blendOverlay( alternateIcon );

    // mix [ grayed <-> colored ] icons
    QPixmap tmpTrayPixmap( *alternateIcon );
   copyBlt( &tmpTrayPixmap, 0,0, grayedIcon, 0,0,
            alternateIcon->width(), alternateIcon->height() - mergePixels );
    blendOverlay( &tmpTrayPixmap );
}

void Tray::blendOverlay( QPixmap * sourcePixmap )
{
    if ( !overlayVisible || !overlay || overlay->isNull() )
        return setPixmap( *sourcePixmap ); // @since 3.2
    
    // here comes the tricky part.. no kdefx functions are helping here.. :-(
    // we have to blend pixmaps with different sizes (blending will be done in
    // the bottom-left corner of source pixmap with a smaller overlay pixmap)
    int opW = overlay->width(),
        opH = overlay->height(),
        opX = 1,
        opY = sourcePixmap->height() - opH;

    // get the rectangle where blending will take place 
    QPixmap sourceCropped( opW, opH, sourcePixmap->depth() );
    copyBlt( &sourceCropped, 0,0, sourcePixmap, opX,opY, opW,opH );

    // blend the overlay image over the cropped rectangle
    QImage blendedImage = sourceCropped.convertToImage();
    QImage overlayImage = overlay->convertToImage();
    KIconEffect::overlay( blendedImage, overlayImage );
    sourceCropped.convertFromImage( blendedImage );
    
    // put back the blended rectangle to the original image
    QPixmap sourcePixmapCopy = *sourcePixmap;
    copyBlt( &sourcePixmapCopy, opX,opY, &sourceCropped, 0,0, opW,opH );

    setPixmap( sourcePixmapCopy ); // @since 3.2
}

#include "tray.moc"
