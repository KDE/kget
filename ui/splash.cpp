/* This file is part of the KDE project

   Copyright (C) 2004 - 2005 KGet Developers <kget@kde.org>
   Splash.cpp/.h taken from Amarok. Thanks to Amarok's authors, cool piece
   of code.. and our favourite player!
   Copyright (C) 2003 by Christian Muehlhaeuser <muesli@chareit.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <QApplication>
#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QDesktopWidget>

#include "splash.h"


OSDWidget::OSDWidget(const QString& imagePath)
    : QWidget(NULL,
              Qt::WType_TopLevel | Qt::WStyle_StaysOnTop |
              Qt::WStyle_Customize | Qt::WStyle_NoBorder |
              Qt::WStyle_Tool | Qt::WNoAutoErase | Qt::WX11BypassWM)
{
    QImage image( imagePath );
    osdBuffer = image;

    QBitmap bm( image.size() );
    QPainter p( &bm );
    p.drawImage( 0, 0, image.createAlphaMask() );

    QWidget *d = QApplication::desktop()->screen();
    move( (d->width() - image.width ()) / 2,
          (d->height() - image.height()) / 2 );
    resize( osdBuffer.size() );
    setFocusPolicy( Qt::NoFocus );
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
