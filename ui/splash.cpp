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

#include "splash.h"

#include <QApplication>
#include <QBitmap>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QDesktopWidget>

Splash::Splash(const QString& imagePath)
    : QWidget(0, Qt::SplashScreen | Qt::X11BypassWindowManagerHint)
{
    cachedPixmap = QPixmap(imagePath);
    if (!cachedPixmap.mask().isNull())
    {
        QBitmap mask(cachedPixmap.size());
        mask.fill(Qt::color0);
        QBitmap pixMask = cachedPixmap.mask();
        QPainter p(&mask);
        p.drawPixmap((mask.width() - pixMask.width())/2, (mask.height() - pixMask.height())/2,
                     pixMask);
        setMask(mask);
    }
    else
        setMask(QBitmap());

    QWidget *d = QApplication::desktop()->screen();
    move( (d->width() - cachedPixmap.width ()) / 2,
          (d->height() - cachedPixmap.height()) / 2 );
    resize( cachedPixmap.size() );
    setFocusPolicy( Qt::NoFocus );

    show();
    update();

    QTimer::singleShot( SPLASH_DURATION, this, SLOT(hide()) );
}

void Splash::removeSplash( int timeout )
{
    QTimer::singleShot( timeout, this, SLOT(hide()) );
}

void Splash::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    const QRect r = rect();
    p.drawPixmap(r.x() + (r.width() - cachedPixmap.width())/2,
                 r.y() + (r.height() - cachedPixmap.height())/2,
                 cachedPixmap);
}

void Splash::mousePressEvent( QMouseEvent* )
{
    removeSplash(100);
}

#include "splash.moc"
