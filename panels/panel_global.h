/* This file is part of the KDE project
   
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _GLOBALPANEL_H
#define _GLOBALPANEL_H

#include <qframe.h>

class QPaintEvent;
class QPixmap;
class QPalette;
class QWidget;

class GlobalPanel : public QFrame
{
    Q_OBJECT
    public:
        GlobalPanel( QWidget * parent = 0, const char * name = 0 );
        ~GlobalPanel();
    
    protected:
        virtual void paintEvent( QPaintEvent * );
        virtual void paletteChange ( const QPalette & );
        
    private:
        static const uint gradientHeight = 100;
        QPixmap * topLeftPix;
};

#endif
