/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kiconloader.h>
#include <qpixmap.h>
#include <qimage.h>

void DlgAppearance::init()
{
    // setup pixmaps
    QPixmap tmp = UserIcon( "target" );
    QImage img = tmp.convertToImage();
    img = img.smoothScale(pixLabel->width(),pixLabel->height());
    tmp.convertFromImage( img );
    pixLabel->setPixmap( tmp );
}
