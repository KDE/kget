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

#include <sys/time.h>


static const uint defaultLength = 100;
static const uint defaultHeight = 35;
static const uint defaultTimeGap = 10;

BandMonWidget::BandMonWidget( enum Direction d, QWidget * parent, const char * name )
    : QWidget( parent, name ), len( defaultLength ), dir( d ),
    scale( AutoPeak ), bCompression( false ), bText( false ), 
    timeGap( defaultTimeGap ), pm( 0 )
{
    samples.setAutoDelete( true );
    clear();
    draw();
}

BandMonWidget::~BandMonWidget()
{
    delete pm;
}

void BandMonWidget::setLength( int length )
{
    len = length;
    pm->resize( minimumSizeHint() );
    draw();
    resize( minimumSizeHint() );
}

QSize BandMonWidget::minimumSizeHint () const
{
    if ( dir == Vertical )
        return QSize( defaultHeight, len );
    return QSize( len, defaultHeight );
}

void BandMonWidget::addSample( float speed )
{    
    if ( ++samples_current >= samples.size() )
        samples_current = 0;

    SpeedSample * s = new SpeedSample();
    s->speed_kbps = speed;
    s->time_stamp = timeStamp();
    samples.insert( samples_current, s );

    draw();
}

void BandMonWidget::clear()
{
    samples.clear();
    samples_current = -1;
    samples.resize( 5 );
}

double BandMonWidget::timeStamp()
{
    struct timeval tv;
    gettimeofday( &tv, 0 );
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void BandMonWidget::draw()
{
    if ( !pm )
        pm = new QPixmap( minimumSizeHint() );
    pm->fill( palette().active().background() );
    
    if ( samples.count() < 2 )
        return;

    double currentTime = timeStamp();
    
    // open a painter over a pixmap and paint it
    QPainter p;
    p.begin( pm );

    
    if ( dir == Horizontal )
    {
        p.setPen( Qt::blue );
        p.setBrush( Qt::darkBlue );
        uint count = samples.count();
        
        uint tmp_index = (count < samples.size()) ? 0 : samples_current + 1;
        if ( tmp_index >= samples.size() )
            tmp_index = 0;
        
        for ( int i = 0; i < count - 1; i++ )
        {
            QPointArray pa( 4 );
            SpeedSample * sP = samples.at( tmp_index++ );
            if ( tmp_index >= samples.size() )
                tmp_index = 0;
            SpeedSample * sN = samples.at( tmp_index );
            double X1 = len * (currentTime - sP->time_stamp) / timeGap;
            double X2 = len * (currentTime - sN->time_stamp) / timeGap;
            pa.setPoint( 0, X1, defaultHeight );
            pa.setPoint( 1, X1, 10 * sP->speed_kbps );
            pa.setPoint( 2, X2, defaultHeight );
            pa.setPoint( 3, X2, 10 * sN->speed_kbps );
            kdWarning() << X1 << " | " << X2 << " | " << timeStamp() << endl;
            p.drawPolygon( pa );
        }

/*    
    
    int pa_index = 0;
    
        float minX = len;
        pa.setPoint( pa_index++, minX, defaultHeight );
        for ( int i = 0; i < samples_count; i++ )
        {
            SpeedSample * ss = samples.at(i);
            float x = len - 5 * pa_index;
            pa.setPoint( pa_index++, x, 10 * ss->speed_kbps );

            if ( len < minX )
                minX = len;
            
            if ( --tmp_index < 0 )
                tmp_index = samples_count - 1;
        }
        pa.setPoint( pa_index++, minX, defaultHeight );
        pa.setPoint( pa_index, len, defaultHeight );
*/  
    } else
    {
        //TODO implement me
    }
    
    p.end();
    
    update();
}

void BandMonWidget::paintEvent( QPaintEvent * e )
{
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
    if ( !w->inherits( "KToolBar" ) ) {
        kdError() << "KWidgetAction::plug: ComboAction must be plugged into KToolBar." << endl;
        return -1;
    }

    KToolBar* toolBar = static_cast<KToolBar*>( w );
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    widget = new QComboBox();
    widget->insertItem( SmallIcon("view_remove"), i18n("Compact") );
    widget->insertItem( SmallIcon("view_detailed"), i18n("Detailed") );
    widget->insertItem( SmallIcon("view_text"), i18n("Old files") );
    //widget->setFixedHeight( ToolBar_HEIGHT - 10 );
    widget->setFocusPolicy(QWidget::NoFocus);
    connect( widget, SIGNAL( activated(int) ), this, SLOT( slotComboActivated(int) ) );

    widget->reparent( toolBar, QPoint() );
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
    if ( !w->inherits( "KToolBar" ) ) {
        kdError() << "KWidgetAction::plug: ViewAsAction must be plugged into KToolBar." << endl;
        return -1;
    }

    KToolBar* toolBar = static_cast<KToolBar*>( w );
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    QLabel * label = new QLabel( text(), (QWidget *)toolBar );
    toolBar->insertWidget( id, 0, label, index );
    toolBar->setItemAutoSized( id, false );

    return containerCount() - 1;
}



SpacerAction::SpacerAction( const QString& text, const KShortcut& cut,
    KActionCollection* ac, const char* name )
    : KAction( text, cut, ac, name ) {}

int SpacerAction::plug( QWidget* w, int index )
{
    if ( !w->inherits( "KToolBar" ) ) {
        kdError() << "KWidgetAction::plug: SpacerAction must be plugged into KToolBar." << endl;
        return -1;
    }

    KToolBar* toolBar = static_cast<KToolBar*>( w );
    int id = KAction::getToolButtonID();
    addContainer( toolBar, id );
    connect( toolBar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    QWidget * widget = new QWidget( toolBar );
    toolBar->insertWidget( id, 0, widget, index );
    toolBar->setItemAutoSized( id, true );

    BMW_TTimer * t = new BMW_TTimer();
    connect( t, SIGNAL( newSample(float) ), widget, SLOT( addSample(float)) );
    return containerCount() - 1;
}


#include "kmainwidget_actions.moc"
