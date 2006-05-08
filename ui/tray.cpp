/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QTimer>
#include <QClipboard>
#include <QPainter>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kmenu.h>
#include <kdebug.h>

#include "core/model.h"
#include "ui/tray.h"
#include "kget.h"

/** class Tray
  * Reimplmentation of the system tray class adding drag/drop
  * capabilities and the quit action.
  */
Tray::Tray(KGet * parent)
    : KSystemTray(parent),
      blinkTimer( 0 ),
      grayedIcon( 0 ),
      alternateIcon( 0 ),
      overlay( 0 ),
      overlayVisible( false )
{
    baseIcon = new QPixmap( KSystemTray::loadIcon("tool_drop_target") );
    playOverlay = new QPixmap( SmallIcon( "dock_overlay_run" ) );
    stopOverlay = new QPixmap( SmallIcon( "dock_overlay_stop" ) );

    paintIcon();

    // add preferences action to the context menu
    KMenu * cm = contextMenu();
    cm->addAction( parent->actionCollection()->action("new_transfer") );
    cm->addAction( parent->actionCollection()->action("preferences") );
    cm->addAction( parent->actionCollection()->action("konqueror_integration") );

    // enable dropping
    setAcceptDrops(true);

    // add tooltip telling "I'm kget"
    setToolTip( kapp->aboutData()->shortDescription() );

    // connecting the "Exit" menu item to the quit() of our app
    connect( this, SIGNAL( quitSelected() ), kapp, SLOT(quit()));
}

// dtor: delete internal classes
Tray::~Tray()
{
    delete blinkTimer;
    delete baseIcon;
    delete grayedIcon;
    delete alternateIcon;
    delete playOverlay;
    delete stopOverlay;
    delete overlay;
}

// test if dropped thing can be handled (must be an URLlist or a QString)
void Tray::dragEnterEvent(QDragEnterEvent * event)
{
    event->setAccepted(KUrl::List::canDecode(event->mimeData())
                  || event->mimeData()->hasText());
}

// decode the dropped element asking scheduler to download that
void Tray::dropEvent(QDropEvent * event)
{
    KUrl::List list = KUrl::List::fromMimeData(event->mimeData());
    QString str;

    if (!list.isEmpty())
    {
        KUrl::List::Iterator it = list.begin();
        KUrl::List::Iterator itEnd = list.end();

        for( ; it!=itEnd ; ++it )
            Model::addTransfer(*it);
    }
    else
    {
        str = event->mimeData()->text();
        Model::addTransfer(KUrl::fromPathOrURL(str));
    }
}

// filter middle mouse clicks to ask scheduler to paste URL
void Tray::mousePressEvent(QMouseEvent * e)
{
    if (e->button() == Qt::MidButton)
    {
        //Here we paste the transfer
        QString newtransfer = QApplication::clipboard()->text();
        newtransfer = newtransfer.trimmed();

        if(!newtransfer.isEmpty())
            Model::addTransfer(KUrl::fromPathOrURL(newtransfer),"");
    }
    else
        KSystemTray::mousePressEvent(e);
}

// display blinking icon when downloading
void Tray::setDownloading( bool running )
{
    kDebug() << "Tray::setDownloading" << endl;

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
        QImage tmpTrayIcon = baseIcon->toImage();
        KIconEffect::semiTransparent( tmpTrayIcon );
        grayedIcon = new QPixmap( QPixmap::fromImage( tmpTrayIcon ) );
    }
    if ( mergePixels == 0 )
        return blendOverlay( grayedIcon );

    // make up the alternate icon (use hilight color but more saturated)
    if ( !alternateIcon )
    {
        QImage tmpTrayIcon = baseIcon->toImage();
        // eros: this looks cool with dark red blue or green but sucks with
        // other colors (such as kde default's pale pink..). maybe the effect
        // or the blended color has to be changed..
        QColor saturatedColor = palette().color(QPalette::Active, QPalette::Highlight);
        int hue, sat, value;
        saturatedColor.getHsv( &hue, &sat, &value );
        saturatedColor.setHsv( hue, (sat + 510) / 3, value );
        KIconEffect::colorize( tmpTrayIcon, saturatedColor/* Qt::blue */, 0.9 );
        alternateIcon = new QPixmap( QPixmap::fromImage( tmpTrayIcon ) );
    }
    if ( mergePixels >= alternateIcon->height() )
        return blendOverlay( alternateIcon );

    // mix [ grayed <-> colored ] icons
    QPixmap tmpTrayPixmap( *alternateIcon );
    QPainter paint;
    paint.begin( &tmpTrayPixmap );
    paint.drawPixmap( 0, 0, *grayedIcon, 0, 0,
        alternateIcon->width(), alternateIcon->height() - mergePixels );
    paint.end();

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
    QPixmap sourceCropped( opW, opH );
    sourceCropped.fill(Qt::transparent);
    QPainter paint;
    paint.begin( &sourceCropped );
    paint.drawPixmap( 0, 0, *sourcePixmap, opX, opY, opW,opH );
    paint.end();

    // blend the overlay image over the cropped rectangle
    QImage blendedImage = sourceCropped.toImage();
    QImage overlayImage = overlay->toImage();
    KIconEffect::overlay( blendedImage, overlayImage );
    sourceCropped = QPixmap().fromImage( blendedImage );

    // put back the blended rectangle to the original image
    QPixmap sourcePixmapCopy = *sourcePixmap;
    paint.begin( &sourcePixmapCopy );
    paint.drawPixmap( opX, opY, sourceCropped, 0, 0, opW,opH );
    paint.end();

    setPixmap( sourcePixmapCopy ); // @since 3.2
}

#include "tray.moc"
