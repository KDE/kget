/*
  Copyright (C) 2004 KGet2 Developers < >
  Splash.cpp/.h taken from Amarok. Thanks to Amarok's authors, cool piece
  of code.. and our favourite player!

  osd.h   -  Provides an interface to a plain QWidget, which is independent of KDE
  begin:     Fre Sep 26 2003
  copyright: (C) 2003 by Christian Muehlhaeuser
  email:     muesli@chareit.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#ifndef SPLASH_H
#define SPLASH_H

#include <qpixmap.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>

class OSDWidget : public QWidget
{
    Q_OBJECT

      public:
        OSDWidget(const QString& imagePath);

      public slots:
        void removeOSD(int timeout = 500);

      protected:
        void paintEvent(QPaintEvent*);
        void mousePressEvent(QMouseEvent*);

        static const int SPLASH_DURATION = 15000; 

        QPixmap osdBuffer;
};

#endif
