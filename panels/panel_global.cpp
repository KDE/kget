/* This file is part of the KDE project
   
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kimageeffect.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qsize.h>
#include <qrect.h>
#include <qfont.h>
#include <qpalette.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "panel_global.h"
#include "browserbar.h" //for MAXIMUM_WIDTH
#include "globals.h" // for KGETVERSION


GlobalPanel::GlobalPanel( QWidget * parent , const char * name )
    : QFrame( parent, name ), topLeftPix( 0 )
{
    // we draw all contents ourselves
    setBackgroundMode( Qt::NoBackground );

    // build the gradient pixmap
    QColor cTop = palette().active().highlight().light(110);
    QColor cBottom = palette().active().background();
    topLeftPix = new QPixmap(
        KImageEffect::gradient( QSize( BrowserBar::MAXIMUM_WIDTH, gradientHeight ),
        cTop, cBottom, KImageEffect::VerticalGradient )
    );
    
    // frame properties
    setLineWidth( 2 );
    setFrameShape( StyledPanel );
    setFrameShadow( Sunken );
    
    // contents
    QVBoxLayout * bl = new QVBoxLayout( this, 6, 11 );
    QLabel * l = new QLabel( "KGet " + QString(KGETVERSION), this );
    l->setEraseColor( cTop );
    QFont bigFont = l->font();
    bigFont.setPointSize( bigFont.pointSize() + 6 );
    l->setFont( bigFont );
    bl->addWidget( l );
    
    QPushButton * b = new QPushButton( "*test1*", this );
    bl->addWidget( b );
    
    bl->addStretch( 1 );
}

GlobalPanel::~GlobalPanel()
{
    delete topLeftPix;
}

void GlobalPanel::paletteChange ( const QPalette & oldpalette )
{
    if ( palette().active().background() == oldpalette.active().background() &&
         palette().active().highlight() == oldpalette.active().highlight() )
        return;
    QColor cTop = palette().active().highlight().light(110);
    QColor cBottom = palette().active().background();
    QPixmap * pix = topLeftPix;
    topLeftPix = new QPixmap(
        KImageEffect::gradient( QSize( BrowserBar::MAXIMUM_WIDTH, gradientHeight ),
        cTop, cBottom, KImageEffect::VerticalGradient )
    );
    delete pix;
}

void GlobalPanel::paintEvent( QPaintEvent * e )
{
    QPainter p;
    p.begin( this );
    p.setClipRect( e->rect() );

    // draw the outer frame
    QFrame::drawFrame( &p );

    // paint the inner area (top gradient)
    QRect r = contentsRect().intersect( e->rect() );
    QRect rArea = r.intersect( QRect( 0,0, topLeftPix->width(),topLeftPix->height() ) );
    if ( rArea.isValid() )
        p.drawPixmap( r.topLeft(), *topLeftPix, rArea );

    // paint the remaining inner area with window color
    if ( r.top() < (int)gradientHeight )
        r.setTop( gradientHeight );
    if ( r.height() > 0 )
        p.fillRect( r, palette().active().background() );
 
    p.end();
}

#include "panel_global.moc"
