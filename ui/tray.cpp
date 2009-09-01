/* This file is part of the KDE project

   Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
   Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "ui/tray.h"

#include "settings.h"
#include "mainwindow.h"
#include "ui/newtransferdialog.h"

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kmenu.h>
#include <kdebug.h>

#include <QPainter>
#include <QLabel>
#include <QClipboard>

/** class Tray
  * Reimplmentation of the system tray class adding drag/drop
  * capabilities and the quit action.
  */
Tray::Tray(MainWindow * parent)
    : KSystemTrayIcon(parent),
      grayedIcon( 0 ),
      alternateIcon( 0 ),
      playOverlay( 0 ),
      m_downloading( false )
{
    baseIcon = new QPixmap( KSystemTrayIcon::loadIcon("kget").pixmap(22) );
    // 12x12 pixel overlay looks fine, amarok uses 10x10
    playOverlay = new QPixmap( SmallIcon( "media-playback-start", 12 ) );

    paintIcon();

    // add preferences action to the context menu
    QMenu * cm = contextMenu();
    cm->addAction( parent->actionCollection()->action("new_download") );
    cm->addAction( parent->actionCollection()->action("import_links") );
    cm->addSeparator();
    cm->addAction( parent->actionCollection()->action("start_all_download") );
    cm->addAction( parent->actionCollection()->action("stop_all_download") );
    cm->addSeparator();
    cm->addAction( parent->actionCollection()->action("konqueror_integration") );
    cm->addAction( parent->actionCollection()->action("show_drop_target") );
    cm->addAction( parent->actionCollection()->action("options_configure") );

    // add tooltip telling "I'm kget"
    setToolTip(i18n("KGet - Download Manager"));

    // connecting the "Exit" menu item to the quit() of our app
    connect( this, SIGNAL( quitSelected() ), kapp, SLOT(quit()));
    connect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
                   SLOT( slotActivated( QSystemTrayIcon::ActivationReason ) ) );
}

// dtor: delete internal classes
Tray::~Tray()
{
    delete baseIcon;
    delete grayedIcon;
    delete alternateIcon;
    delete playOverlay;
}

// filter middle mouse clicks to ask scheduler to paste URL
void Tray::slotActivated( QSystemTrayIcon::ActivationReason reason )
{
    if ( reason == QSystemTrayIcon::MiddleClick )
    {
        //Here we paste the transfer
        QString newtransfer = QApplication::clipboard()->text();
        newtransfer = newtransfer.trimmed();

        if (!newtransfer.isEmpty())
            NewTransferDialog::showNewTransferDialog(KUrl(newtransfer));
    }
    /*else if (reason == QSystemTrayIcon::Trigger) {
        // save the main window minimized state if it changes from the sys tray icon
        Settings::setShowMain(parentWidget()->isVisible());
    }*/
}

// display a play icon when downloading
void Tray::setDownloading( bool downloading )
{
    if (downloading == m_downloading)
        return;

    m_downloading = downloading;
    kDebug(5001) << "Tray::setDownloading";

    if (downloading)
        paintIcon( 50, true );
    else
        paintIcon( -1, true );

}

bool Tray::isDownloading()
{
    return m_downloading;
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
        QLabel label; //just a hack to get the palette
        QColor saturatedColor = label.palette().color(QPalette::Active, QPalette::Highlight);
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
    if ( !m_downloading || !playOverlay || playOverlay->isNull() )
        return setIcon( *sourcePixmap );

    // no kdefx functions are helping here.. :-(
    // we have to blend pixmaps with different sizes (blending will be done in
    // the bottom-right corner of source pixmap with a smaller overlay pixmap)
    int opW = playOverlay->width(),
        opH = playOverlay->height(),
        opX = sourcePixmap->width() - opW,
        opY = sourcePixmap->height() - opH;

    QPixmap sourcePixmapCopy = *sourcePixmap;
    QPainter paint( &sourcePixmapCopy );
    paint.drawPixmap( opX, opY, *playOverlay );
    paint.end();

    setIcon( sourcePixmapCopy );
}

#include "tray.moc"
