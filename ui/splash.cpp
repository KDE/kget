/*
  Copyright (C) 2004 KGet2 Developers < >
  Splash.cpp/.h taken from Amarok. Thanks to Amarok's authors, cool piece
  of code.. and our favourite player!

  osd.cpp -  Provides an interface to a plain QWidget, which is independent of KDE
  begin:     Fre Sep 26 2003
  copyright: (C) 2003 by Christian Muehlhaeuser
  email:     muesli@chareit.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include <qapplication.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qtimer.h>

#include "splash.h"


OSDWidget::OSDWidget(const QString& imagePath)
    : QWidget(NULL, "osd",
              WType_TopLevel | WStyle_StaysOnTop |
              WStyle_Customize | WStyle_NoBorder |
              WStyle_Tool | WNoAutoErase | WX11BypassWM)
{    
    QImage image( imagePath );
    osdBuffer = image;

    QBitmap bm( image.size() );
    QPainter p( &bm );
    p.drawImage( 0, 0, image.createAlphaMask() );

    QWidget *d = QApplication::desktop();
    move( (d->width() - image.width ()) / 2,
          (d->height() - image.height()) / 2 );
    resize( osdBuffer.size() );
    setFocusPolicy( NoFocus );
    setMask( bm );

    show();
    update();

    QTimer::singleShot( SPLASH_DURATION, this, SLOT(hide()) );
}

void OSDWidget::removeOSD( int timeout )
{
    QTimer::singleShot( timeout, this, SLOT(hide()) );
}

void OSDWidget::paintEvent( QPaintEvent * e )
{
    QPainter p( this );
    QRect rect = e->rect();
    p.drawPixmap( rect.topLeft(), osdBuffer, rect );
    p.end();
}

void OSDWidget::mousePressEvent( QMouseEvent* )
{
    removeOSD(100);
}

#include "splash.moc"
