/* This file is part of the KDE project
   Copyright (C) 2004 KGet2x developers < >
                      Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "kmainwidget_actions.h"
#include "kmainwidget.h"

#include <kapplication.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <qwidget.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qrect.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qpainter.h>
#include <qwhatsthis.h>


//Remember : popup , compression , enable text..

BandMonWidget::BandMonWidget( bool v, QWidget * parent, const char * name )
    : QWidget( parent, name ), pixelLength( 100 ),
    isVertical( v ), pm( 0 )
{
    setBackgroundMode( Qt::NoBackground );

    samples.resize( pixelLength, 0 );
    samples_current = pixelLength - 1;

    timer = new QTimer();
    connect( timer, SIGNAL( timeout() ), this, SLOT( slotScroll() ) );
    
    setFrequency( 10 );
    setMaximum( 5 );
    
    drawPixmap();
}

BandMonWidget::~BandMonWidget()
{
    delete timer;
    delete pm;
}

void BandMonWidget::setFrequency( float samples_per_second )
{
    samplesPerSecond = samples_per_second;
    timer->start( (int)( 1000.0 / samples_per_second ) );
}

void BandMonWidget::setMaximum( float max_kbps )
{
    maximum = max_kbps;
}

void BandMonWidget::setLength( int /*length*/ )
{
/*  TODO this
    samples.clear();
    samples.resize( pixelLength = length, 0 );
    samples_current = -1;
    QSize s = minimumSizeHint()
    pm->resize( s );
    drawPixmap();
    resize( s );
*/
}

QSize BandMonWidget::minimumSizeHint () const
{
    if ( isVertical )
        return QSize( ToolBar_HEIGHT, pixelLength );
    return QSize( pixelLength, ToolBar_HEIGHT );
}

void BandMonWidget::addSample( float speed )
{    
    samples[ samples_current ] = speed;
}

void BandMonWidget::clear()
{
    for ( int i = samples.size() - 1; i >= 0; i-- )
        samples[ i ] = 0.0;
    samples_current = pixelLength - 1;
}

void BandMonWidget::slotScroll()
{
    drawPixmap();
    if ( ++samples_current >= (int)samples.size() )
        samples_current = 0;
    update();
}

void BandMonWidget::drawPixmap()
{
    if ( !pm || pm->size() != size() )
    {
        delete pm;
        pm = new QPixmap( size() );
    }
    
    // open a painter over the internal pixmap
    QPainter p;
    p.begin( pm );

    // fill background if not handled by style
    p.fillRect( pm->rect(), palette().active().background() );

    // draw a line on the bottom
    QColor color = palette().active().button();
    p.setPen( color );
    p.drawLine( 0, height() - 3 , pixelLength - 1, height() - 3 );

    // draw the graph
    p.setPen( color.dark( 160 ) );
    int count = samples.size();
    int tmp_index = samples_current + 1;
    if ( tmp_index >= count )
        tmp_index = 0;    
    if ( !isVertical )
    {
        int y_base = height() - 4;
        int y_ext = y_base - 3;

        for ( int i = 0; i < count; i++ )
        {
            float h = samples[ tmp_index ];
            if ( h > maximum )
            {
                QColor oldcol = p.pen().color();
                p.setPen( color.dark(140) );
                p.drawLine( i, y_base, i, y_base - y_ext );
                p.setPen( oldcol );
            }
            else
                p.drawLine( i, y_base, i, y_base - y_ext * (h / maximum) );
            if ( ++tmp_index >= count )
                tmp_index = 0;
        }
    } else { /*TODO this*/ }
    
    // draw the mean line
    p.setPen( color.light( 160 ) );
    float mean = 0;
    for ( int i = samples.size() - 1; i >= 0 ; i-- )
        mean += samples[ i ];
    mean /= samples.size();
    if ( !isVertical )
    {
        int y_base = height() - 4;
        int y_ext = y_base - 3;
        float k = mean / maximum;
        
        if ( k < 1 )
            p.drawLine( 0, y_base - y_ext * k, pixelLength - 1, y_base - y_ext * k );
        else
            p.drawLine( 0, y_base - y_ext, pixelLength - 1, y_base - y_ext );
    } else { /*TODO this*/ }
    
    p.end();
}

void BandMonWidget::paintEvent( QPaintEvent * e )
{
    if ( !pm )
        return;        
    QPainter p;
    p.begin( this );
    QRect r = e->rect();
    p.drawPixmap( r.topLeft(), *pm, r );
    p.end();
}



ComboAction::ComboAction( const QString& text, const KShortcut& cut,
    KActionCollection* ac, KMainWidget *mw, const char* name )
    : KAction( text, cut, ac, name ), widget( 0 ), parent( mw )
{
    connect( parent, SIGNAL( viewModeChanged(int) ), this, SLOT( slotViewModeChanged(int) ) );
}

int ComboAction::plug( QWidget* w, int index )
{
    KToolBar* toolBar = dynamic_cast<KToolBar*>( w );
    if ( !toolBar )
        return -1;
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    widget = new QComboBox( toolBar );
    widget->insertItem( SmallIcon("view_remove"), i18n("Compact") );
    widget->insertItem( SmallIcon("view_detailed"), i18n("Detailed") );
    widget->insertItem( SmallIcon("view_text"), i18n("Old files") );
    widget->setFocusPolicy(QWidget::NoFocus);
    connect( widget, SIGNAL( activated(int) ), this, SLOT( slotComboActivated(int) ) );

    toolBar->insertWidget( id, 0, widget, index );
    toolBar->setItemAutoSized( id, false /*true*/ );

    QWhatsThis::add( widget, whatsThis() );
    return containerCount() - 1;
}

void ComboAction::slotComboActivated( int index )
{
    parent->setViewMode( (enum KMainWidget::ViewMode)index );
}

void ComboAction::slotViewModeChanged( int index )
{
    if ( widget )
        widget->setCurrentItem( index );
}



ViewAsAction::ViewAsAction( const QString& text, const KShortcut& cut,
    KActionCollection* ac, const char* name )
    : KAction( text, cut, ac, name ) {}

int ViewAsAction::plug( QWidget* w, int index )
{
    KToolBar* toolBar = dynamic_cast<KToolBar*>( w );
    if ( !toolBar )
        return -1;
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    //use "kde toolbar widget" as the name to be styled!
    QLabel * label = new QLabel( text(), (QWidget *)toolBar, "kde toolbar widget" );
    toolBar->insertWidget( id, 0, label, index );
    toolBar->setItemAutoSized( id, false );

    return containerCount() - 1;
}



SpacerAction::SpacerAction( const QString& text, const KShortcut& cut,
    KActionCollection* ac, const char* name )
    : KAction( text, cut, ac, name ) {}

int SpacerAction::plug( QWidget* w, int index )
{
    KToolBar* toolBar = dynamic_cast<KToolBar*>( w );
    if ( !toolBar )
        return -1;
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    //use "kde toolbar widget" as the name to be styled!
    QWidget * widget = new QWidget( toolBar, "kde toolbar widget" );
    toolBar->insertWidget( id, 0, widget, index );
    toolBar->setItemAutoSized( id, true );

    return containerCount() - 1;
}


BandAction::BandAction( const QString& text, const KShortcut& cut,
    KActionCollection* ac, const char* name )
    : KAction( text, cut, ac, name ) {}

int BandAction::plug( QWidget* w, int index )
{
    KToolBar* toolBar = dynamic_cast<KToolBar*>( w );
    if ( !toolBar )
        return -1;
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    QWidget * widget = new BandMonWidget( false, toolBar );
    toolBar->insertWidget( id, 0, widget, index );
    toolBar->setItemAutoSized( id, false );

    BMW_TTimer * t = new BMW_TTimer();
    connect( t, SIGNAL( newSample(float) ), widget, SLOT( addSample(float)) );
    return containerCount() - 1;
}

#include "kmainwidget_actions.moc"
