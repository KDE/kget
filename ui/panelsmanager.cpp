/* This file was taken from the AMAROK project
   The code that follows is by Max Howell, that licensed it under
   the terms of the GNU General Public License as published by the
   Free Software Foundation; version 2. The COPYING file Max referred
   to contained such license.

   All mods : Copyright (C) 2004 KGet2 Developers < >
   Last synced: 2004-May-8
*/

// Maintainer:  Max Howell (C) Copyright 2004
// Copyright:   See COPYING file that comes with this distribution
// Description: The SideBar/MultTabBar/BrowserBar all-in-one spectacular!
//

#include "panelsmanager.h"

#include <qcursor.h>       //for resize cursor
#include <qobjectlist.h>   //coloredObjects()
#include <qpainter.h>      //BrowserBar::TinyButton
#include <qpixmap.h>       //TinyButtons
#include <qsignalmapper.h> //m_mapper
#include <qstyle.h>        //BrowserBar::BrowserBar
#include <qtooltip.h>      //QToolTip::add()

#include <kapplication.h>  //kapp
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>   //multiTabBar icons
#include <klocale.h>
#include <kmultitabbar.h>  //m_tabBar


//USAGE
// 1. create a widget, NAME THE WIDGET!
// 2. use addBrowser() to append it to the bar
// 3. you can retrieve pointers to the widgets you inserted using browser( "name" )

//<mxcl>
//This is much tighter code than KDockWidget and co.
//I think I should look into submitting patches for that class. It's quite messy.
//But it is also more flexible in a docking perspective.

//TODO make browserholder a layout
//TODO perhaps you can add browsers and holder to multitabbar, and then just size that accordingly?
//NOTE the widget widths are saved in their baseSize() property



namespace amaroK {

class Divider : public QWidget
{
public:
    Divider( QWidget *w ) : QWidget( w ) { styleChange( style() ); }

    virtual void paintEvent( QPaintEvent* )
    {
        QPainter p( this );
        parentWidget()->style().drawPrimitive( QStyle::PE_Splitter, &p, rect(), colorGroup(), 0 );
    }

    virtual void styleChange( QStyle& )
    {
        setFixedWidth( style().pixelMetric( QStyle::PM_SplitterWidth, this ) );
    }
};

}


BrowserBar::BrowserBar( QWidget *parent )
  : QWidget( parent, "BrowserBar" )
  , m_playlist( new QVBox( this ) )
  , m_divider( new amaroK::Divider( this ) )
  , m_tabBar( new KMultiTabBar( KMultiTabBar::Vertical, this ) )
  , m_browserHolder( new QWidget( this ) ) //FIXME making this a layout would save mem
  , m_currentBrowser( 0 )
  , m_currentTab( 0 )
  , m_mapper( new QSignalMapper( this ) )
{
    m_pos = m_tabBar->sizeHint().width();

    m_tabBar->setStyle( KMultiTabBar::VSNET );
    m_tabBar->setPosition( KMultiTabBar::Left );
    m_tabBar->showActiveTabTexts( true );
    m_tabBar->setFixedWidth( m_pos );

    m_browserHolder->move( m_pos, 0 );

    new QVBoxLayout( m_browserHolder );

    connect( m_mapper, SIGNAL( mapped( int ) ), SLOT( showHideBrowser( int ) ) );

    m_divider->installEventFilter( this );
    m_divider->setCursor( QCursor(SizeHorCursor) );

    //set the browserbar to an initial state of closed();
    m_browserHolder->hide();
    m_divider->hide();
    //ensure these widgets are at the front of the stack
    m_browserHolder->raise();
    m_divider->raise();
}

BrowserBar::~BrowserBar()
{
    KConfig *config = kapp->config();
    config->setGroup( "SideBar" );

    config->writeEntry( "CurrentPane", m_currentBrowser ? m_currentBrowser->name() : QString::null );

    for( BrowserIterator it = m_browsers.constBegin(), end = m_browsers.constEnd(); it != end; ++it )
        config->writeEntry( (*it)->name(), (*it)->baseSize().width() );
}


void
BrowserBar::adjustWidgetSizes()
{
    const uint w   = width();
    const uint h   = height();
    const uint p   = position();
    const uint ppw = p + m_divider->width();
    const uint tbw = m_tabBar->width();

    //this bool indicates whether or not to draw the playlist offset
    //due to an open tab in overlap mode
    const bool b = /*m_overlapButton->isOn() &&*/ !m_divider->isHidden();

    m_divider->move( p, 0 );

    const uint offset = b ? ppw : tbw + 4; //the 4 is just for aesthetics

    m_browserHolder->resize( p - tbw, h );
    m_playlist->setGeometry( offset, 0, w - offset, h );
}

bool
BrowserBar::eventFilter( QObject*, QEvent *e )
{
    if( !m_currentBrowser ) return false;

    switch( e->type() )
    {
    case QEvent::MouseButtonRelease:

        m_currentBrowser->setBaseSize( m_currentBrowser->size() ); //only necessary to do on mouse release

        //FALL THROUGH

    case QEvent::MouseMove:
    {
        #define e static_cast<QMouseEvent*>(e)

        const uint currentPos = m_pos;
        const uint newPos     = mapFromGlobal( e->globalPos() ).x();

        uint maxWidth   = MAXIMUM_WIDTH;
        uint minWidth   = m_tabBar->width() + m_currentBrowser->minimumWidth();

        if ( minWidth < MINIMUM_WIDTH )
            minWidth = MINIMUM_WIDTH;
        if ( minWidth > maxWidth )
            m_pos = minWidth = maxWidth;
        else if( newPos < minWidth ) m_pos = minWidth;
        else if( newPos < maxWidth ) m_pos = newPos; //TODO allow for widget maximumWidth

        //TODO minimum playlist width must be greater than 10/9 of tabBar width or will be strange behaviour

        if( m_pos != currentPos ) adjustWidgetSizes();

        return true;

        #undef e
    }

    default:
        break;
    }

    return false;
}

bool
BrowserBar::event( QEvent *e )
{
  switch( e->type() )
  {
  case QEvent::LayoutHint:
      setMinimumWidth( m_tabBar->minimumWidth() + m_divider->minimumWidth() + m_playlist->minimumWidth() );
      break;

  case QEvent::Resize:

      m_divider->resize( 0, height() ); //Qt will set width
      m_tabBar->resize( 0, height() ); //Qt will set width
      adjustWidgetSizes();
      return true;

  default:
      break;
  }

  return QWidget::event( e );
}

void
BrowserBar::addBrowser( QWidget *widget, const QString &title, const QString& icon )
{
    //hi, this function is ugly - blame the monstrosity that is KMultiTabBar

    //determine next available id
    const int id = m_tabBar->tabs()->count();
    const QString name( widget->name() );

    widget->reparent( m_browserHolder, QPoint(), false ); //we need to own this widget for it to layout properly
    m_browserHolder->layout()->add( widget );
    widget->hide();
    if( widget->minimumWidth() < 30 ) widget->setMinimumWidth( 30 );

    m_tabBar->appendTab( KGlobal::iconLoader()->loadIcon( icon, KIcon::NoGroup, KIcon::SizeSmall ), id, title );
    QWidget *tab = m_tabBar->tab( id );
    tab->setFocusPolicy( QWidget::NoFocus ); //FIXME you can focus on the tab, but they respond to no input!

    //we use a SignalMapper to show/hide the corresponding browser when tabs are clicked
    connect( tab, SIGNAL( clicked() ), m_mapper, SLOT( map() ) );
    m_mapper->setMapping( tab, id );

    m_browsers.push_back( widget );

    KConfig *config = kapp->config();
    config->setGroup( "SideBar" );
    widget->setBaseSize( config->readNumEntry( name, MINIMUM_WIDTH/*widget->sizeHint().width()*/ ), DEFAULT_HEIGHT );
    if( config->readEntry( "CurrentPane" ) == name ) showHideBrowser( id );
}

void
BrowserBar::showHideBrowser( int index )
{
    //determine the target browser (the widget to show/hide)
    QWidget* const target = index == -1 ? m_currentBrowser : m_browsers[index];

    if( m_currentBrowser ) //then we need to hide the currentBrowser
    {
        m_currentBrowser->hide();
        m_currentTab->setState( false );
    }

    if( target == m_currentBrowser ) //then we need to set the bar to the closed state
    {
        m_currentBrowser = 0;
        m_currentTab  = 0;

        m_browserHolder->hide();
        m_divider->hide();

        //we only need to adjust widget sizes if the overlap button is on
        //as otherwise playlist is right size already (see adjustWidgetSizes())
        adjustWidgetSizes();

    } else if( target ) { //then open up target

        const bool resize = !m_currentBrowser;

        m_currentBrowser = target;
        m_currentTab  = m_tabBar->tab( index );

        //NOTE it is important that the divider is visible before adjust..() is called
        //NOTE the showEvents are processed after we have adjustedSizes below
        m_divider->show();
        m_currentBrowser->show();
        m_currentBrowser->setFocus();
        m_currentTab->setState( true );
        m_browserHolder->show();

        if( resize )
        {
            //we need to resize the browserHolder

            m_pos = m_currentBrowser->baseSize().width() + m_tabBar->width();

            adjustWidgetSizes();
        }
    }
}

QWidget*
BrowserBar::browser( const QCString &widgetName ) const
{
    for( BrowserIterator it = m_browsers.constBegin(), end = m_browsers.constEnd(); it != end; ++it )
        if( widgetName == (*it)->name() )
            return *it;

    return 0;
}

#include "panelsmanager.moc"
