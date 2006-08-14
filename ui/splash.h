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

#ifndef SPLASH_H
#define SPLASH_H

#include <QPixmap>
#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

class Splash : public QWidget
{
    Q_OBJECT

      public:
        Splash(const QString& imagePath);

      public slots:
        void removeSplash(int timeout = 500);

      protected:
        void paintEvent(QPaintEvent*);
        void mousePressEvent(QMouseEvent*);

        static const int SPLASH_DURATION = 15000;

        QPixmap cachedPixmap;
};

#endif
