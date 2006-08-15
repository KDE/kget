/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QPainter>
#include <QPaintEvent>
#include <QDebug>

#include <klocale.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kiconeffect.h>
#include <kdebug.h>

#include "core/kget.h"
#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "sidebar.h"

Button::Button(QWidget * w)
    : QToolButton(w)
{

}

void Button::paintEvent ( QPaintEvent * event )
{
    QPainter p(this);

    p.setBackgroundMode(Qt::TransparentMode);
    p.drawPixmap(0, 0, SmallIcon("player_play"));
}


SidebarBox::SidebarBox( Sidebar * sidebar, int headerHeight,
                        int footerHeight, bool enableAnimations )
    : QWidget( sidebar ),
      m_enableAnimations( enableAnimations ),
      m_isHighlighted( false ),
      m_isShown( true ),
      m_showChildren( true ),
      m_headerHeight( headerHeight ),
      m_footerHeight( footerHeight ),
      m_pixFunsel( 0 ),
      m_pixFsel( 0 ),
      m_pixTgrad( 0 ),
      m_pixBgrad( 0 ),
      m_pixPlus( 0 ),
      m_pixMinus( 0 ),
      m_sidebar(sidebar)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    m_layout->setEnabled(true);
    setLayout(m_layout);

    m_headerSpacer = new QWidget(this);
    m_footerSpacer = new QWidget(this);

    m_headerSpacer->setFixedSize(0,0);
    m_footerSpacer->setFixedSize(0,0);

    m_headerSpacer->setUpdatesEnabled(false);
    m_footerSpacer->setUpdatesEnabled(false);

    m_layout->addWidget(m_headerSpacer);
    m_layout->addWidget(m_footerSpacer);

    m_layout->update();

    //setAttribute(Qt::WA_ContentsPropagated);
    setMouseTracking(true);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumWidth(150);

    updatePixmaps();
}

SidebarBox::~SidebarBox()
{
    delete m_pixFunsel;
    delete m_pixFsel;
    delete m_pixTgrad;
    delete m_pixBgrad;
    delete m_pixPlus;
    delete m_pixMinus;
}

void SidebarBox::addChild( SidebarBox * child )
{
    m_layout->insertWidget(m_childBoxes.size()+1, child);

    child->setShown(m_showChildren);
    m_childBoxes.append(child);
}

void SidebarBox::removeChild( SidebarBox * child )
{
    m_layout->removeWidget(child);

    m_childBoxes.removeAll(child);
    m_sidebar->stopTimer(child);
    delete(child);
}

void SidebarBox::showChildren( bool show )
{
    kDebug(5001) << "SidebarBox::showChildren" << endl;
    m_showChildren = show;

    QList<SidebarBox *>::iterator it = m_childBoxes.begin();
    QList<SidebarBox *>::iterator itEnd = m_childBoxes.end();

    for(; it!=itEnd; it++)
    {
        (*it)->setShown(show);
    }
}

void SidebarBox::setHighlighted(bool highlighted)
{
    if(m_isHighlighted != highlighted)
    {
        m_isHighlighted = highlighted;

        if(m_isHighlighted)
            setBackgroundRole(QPalette::Highlight);
        else
            setBackgroundRole(QPalette::NoRole);

        repaint();
    }
}

void SidebarBox::repaintChildren()
{
    foreach(SidebarBox * it, m_childBoxes)
    {
        it->repaintChildren();
    }
    repaint();
}

void SidebarBox::setShown( bool show )
{
    m_isShown = show;

    if( m_enableAnimations )
    {
        m_sidebar->startTimer(this);
    }
    else
    {
        if(show)
        {
            m_headerSpacer->setFixedSize(0, m_headerHeight);
            m_footerSpacer->setFixedSize(0, m_footerHeight);
        }
        else
        {
            m_headerSpacer->setFixedSize(0, 0);
            m_footerSpacer->setFixedSize(0, 0);
        }
    }
}

void SidebarBox::setAnimationEnabled(bool enable)
{
    m_enableAnimations = enable;
}

void SidebarBox::paletteChange ( const QPalette & oldPalette )
{
    updatePixmaps();
}


void SidebarBox::updatePixmaps()
{
    //Here we create all the necessary pixmaps
    //m_pixFunsel: unselected folder
    delete m_pixFunsel;
    m_pixFunsel = new QPixmap(DesktopIcon("folder", 32));

    //m_pixFsel: selected folder
    QImage  img = DesktopIcon("folder_open", 32).toImage();
    KIconEffect::toGamma(img, 0);
    delete m_pixFsel;
    m_pixFsel = new QPixmap(QPixmap::fromImage(img));

    //m_pixTgrad: top gradient
    delete m_pixTgrad;
    m_pixTgrad = new QPixmap( QPixmap::fromImage( KImageEffect::gradient(
                 QSize( 1, 5 ),
                 m_sidebar->palette().color(QPalette::Active, QPalette::Highlight).light(150),
                 m_sidebar->palette().color(QPalette::Active, QPalette::Highlight),
                 KImageEffect::VerticalGradient ) ) );

    //m_pixBgrad: bottom gradient
    delete m_pixBgrad;
    m_pixBgrad = new QPixmap( QPixmap::fromImage( KImageEffect::gradient(
                 QSize( 1, 5 ),
                 m_sidebar->palette().color(QPalette::Active, QPalette::Highlight),
                 m_sidebar->palette().color(QPalette::Active, QPalette::Highlight).light(150),
                 KImageEffect::VerticalGradient ) ) );

    //m_pixPlus: plus simbol
    delete m_pixPlus;
//    m_pixPlus = new QPixmap(SmallIcon("viewmag+", 16));
    m_pixPlus = new QPixmap(SmallIcon("1rightarrow", 16));

    //m_pixMinus: minus simbol
    delete m_pixMinus;
//    m_pixMinus = new QPixmap(SmallIcon("viewmag-", 16));
    m_pixMinus = new QPixmap(SmallIcon("1downarrow", 16));
}

void SidebarBox::paintEvent( QPaintEvent * event )
{
    QPainter p(this);
    p.setClipRegion(event->region());

    if(m_isHighlighted)
    {
        int w = width();
        int h = height();

        p.fillRect(QRect(0,5,w, h-9), QBrush(palette().color(QPalette::Active, QPalette::Highlight)));
        p.drawTiledPixmap(0,0, w, 5, *m_pixTgrad);
        p.drawTiledPixmap(0,h-4, w, 4, *m_pixBgrad);
        p.setPen(QPen(palette().color(QPalette::Active, QPalette::Highlight).dark(130),1));
        p.drawLine(0,0,w, 0);
        p.drawLine(0,h-1,w, h-1);

        p.drawPixmap(30, 2, *m_pixFsel);
    }
    else
        p.drawPixmap(30, 2, *m_pixFunsel);
}

void SidebarBox::timerEvent()
{
    int headerHeight = m_headerSpacer->height();
    int footerHeight = m_footerSpacer->height();

    int newHeaderHeight = 0;
    int newFooterHeight = 0;

    if( m_isShown )
    {
        bool headerIncrement = (headerHeight + 4) < m_headerHeight;
        bool footerIncrement = (footerHeight + 4) < m_footerHeight;

        if(headerIncrement)
            newHeaderHeight = headerHeight + 4;
        else
            newHeaderHeight = m_headerHeight;

        if(footerIncrement)
            newFooterHeight = footerHeight + 4;
        else
            newFooterHeight = m_footerHeight;

        if((!headerIncrement) && (!footerIncrement))
            m_sidebar->stopTimer(this);
    }
    else
    {
        bool headerDecrement = (headerHeight - 4) > 0;
        bool footerDecrement = (footerHeight - 4) > 0;

        if(headerDecrement)
            newHeaderHeight = headerHeight - 4;
        else
            newHeaderHeight = 0;

        if(footerDecrement)
            newFooterHeight = footerHeight - 4;
        else
            newFooterHeight = 0;

        if((!headerDecrement) && (!footerDecrement))
            m_sidebar->stopTimer(this);
    }

    m_headerSpacer->setFixedSize(0, newHeaderHeight);
    m_footerSpacer->setFixedSize(0, newFooterHeight);
}

void SidebarBox::mouseMoveEvent ( QMouseEvent * event )
{
//     kDebug(5001) << "SidebarBox::MouseMoveEvent" << endl;
    if(!m_isHighlighted)
        m_sidebar->boxHighlightedEvent(this);
}

void SidebarBox::mousePressEvent ( QMouseEvent * event )
{
    kDebug(5001) << "MousePressEvent" << endl;
    if(!m_isHighlighted)
        m_sidebar->boxHighlightedEvent(this);
}

void SidebarBox::mouseReleaseEvent ( QMouseEvent * event )
{
    kDebug(5001) << "MouseReleaseEvent" << endl;
}

void SidebarBox::mouseDoubleClickEvent ( QMouseEvent * event )
{
    kDebug(5001) << "MouseDoubleClickEvent" << endl;
    m_showChildren=!m_showChildren;
    m_sidebar->boxSelectedEvent(this, m_showChildren);
}

void SidebarBox::enterEvent ( QEvent * event )
{
    kDebug(5001) << "enterEvent" << endl;
//    m_sidebar->boxHighlightedEvent(this);
}

void SidebarBox::leaveEvent ( QEvent * event )
{
    kDebug(5001) << "leaveEvent" << endl;
}

DownloadsBox::DownloadsBox( Sidebar * sidebar )
    : SidebarBox( sidebar, 34, 4, false)
{
    m_sidebar->boxHighlightedEvent(this);
    setShown(true);
    showChildren(true);
}

void DownloadsBox::paintEvent ( QPaintEvent * event )
{
    kDebug(5001) << "DownloadsBox paint event" << endl;

    SidebarBox::paintEvent( event );

    QPainter p(this);
    p.setClipRegion(event->region());

    int w = width();
    int h = height();

    if(m_showChildren)
        p.drawPixmap(25, 20, *m_pixMinus);
    else
        p.drawPixmap(25, 20, *m_pixPlus);

    QFont f(p.font());
    f.setBold(m_isHighlighted);
    p.setFont( f );
    if (m_isHighlighted)
        p.setPen( m_sidebar->palette().color(QPalette::Active, QPalette::HighlightedText));
    else
        p.setPen( m_sidebar->palette().color(QPalette::Active, QPalette::Foreground));
    p.drawText(50, 11, width(), height()-7, Qt::AlignLeft, i18n("Downloads"));
}

GroupBox::GroupBox( TransferGroupHandler * group, DownloadsBox * dbox, Sidebar * sidebar )
    : SidebarBox( sidebar, 38, 0, true ),
      m_downloadsBox( dbox ),
      m_group( group )
{
/*    Button * t = new Button(this);
    t->setGeometry(10, 10, 30, 30);*/
//     t->setIcon(SmallIcon("player_play"));
    

    m_group->addObserver(this);
    m_downloadsBox->addChild(this);
}

GroupBox::~GroupBox()
{

}

void GroupBox::paintEvent ( QPaintEvent * event )
{
    kDebug(5001) << "GroupBox::paintEvent   " << m_group->name() << endl;

    SidebarBox::paintEvent( event );

    QPainter p(this);
    p.setClipRegion(event->region());

    QFont f(p.font());
    f.setBold(m_isHighlighted);
    p.setFont( f );
    if(m_isHighlighted)
        p.setPen( m_sidebar->palette().color(QPalette::Active, QPalette::HighlightedText));
    else
        p.setPen( m_sidebar->palette().color(QPalette::Active, QPalette::Foreground));
    p.drawText(70, 11, width(), height()-7, Qt::AlignLeft, m_group->name());
}

void GroupBox::groupChangedEvent(TransferGroupHandler * group)
{

}

void GroupBox::addedTransferEvent(TransferHandler * transfer, TransferHandler * after)
{
    transfer->addObserver(this);
}

void GroupBox::transferChangedEvent(TransferHandler * transfer)
{
    //kDebug(5001) << "GroupBox::transferChangedEvent -> ENTERING" << endl;

    if(transfer->changesFlags(this) & Transfer::Tc_Status)
    {
        if(transfer->status() == Job::Running)
        {
            kDebug(5001) << "Creating TransferBox: transfer status = "
                      << transfer->statusText() << endl;

            //Better check if we already created the TransferBox for this transfer
            foreach(SidebarBox * it, m_childBoxes)
            {
                if(static_cast<TransferBox *>(it)->transfer() == transfer)
                    return;
            }
            //TransferBox * g = new TransferBox(transfer, this, m_sidebar);
        }
    }

    transfer->resetChangesFlags(this);

    //kDebug(5001) << "GroupBox::transferChangedEvent -> LEAVING" << endl;
}

TransferBox::TransferBox( TransferHandler * transfer, GroupBox * gBox, Sidebar * sidebar )
    : SidebarBox( sidebar, 20, 0, true ),
      m_groupBox( gBox ),
      m_transfer( transfer )
{
    m_transfer->addObserver(this);
    m_groupBox->addChild(this);
}

TransferBox::~TransferBox()
{

}

void TransferBox::paintEvent ( QPaintEvent * event )
{
    SidebarBox::paintEvent( event );

    QPainter p(this);
    p.setClipRegion(event->region());

    p.drawPixmap(20, 1, m_transfer->statusPixmap());
    p.drawText(40, 1, width(), height()-7, Qt::AlignLeft, m_transfer->source().fileName());
}

void TransferBox::transferChangedEvent(TransferHandler * transfer)
{
    kDebug(5001) << "TransferBox::transferChangedEvent() ENTERING" << endl;
    if(transfer->changesFlags(this) & Transfer::Tc_Status)
    {
        if(transfer->status() != Job::Running)
        {
            m_transfer->delObserver(this);
            kDebug(5001) << "###############  OBSERVER DELETED" << endl;
            m_groupBox->removeChild(this);
            kDebug(5001) << "TransferBox::transferChangedEvent() LEAVING1" << endl;
            return;
        }
        else
        {
            repaint();
        }
    }
    m_transfer->resetChangesFlags(this);
    kDebug(5001) << "TransferBox::transferChangedEvent() LEAVING2" << endl;
}


Sidebar::Sidebar( QWidget * parent )
    : QWidget( parent ),
      m_timerInterval(20),
      m_highlightedBox( 0 )
{
    setUpdatesEnabled(true);

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_downloadsBox = new DownloadsBox(this);
    m_layout->insertWidget( 0, m_downloadsBox );

    m_layout->addStretch(1);

    show();

    KGet::addObserver(this);
}

void Sidebar::startTimer( SidebarBox * item )
{
    if( m_activeTimers.size() == 0 )
        m_timerId = QWidget::startTimer( m_timerInterval );

    if(m_activeTimers.indexOf(item) == -1)
        m_activeTimers.append( item );
}

void Sidebar::stopTimer( SidebarBox * item )
{
    m_timersToRemove.append( item );
}

void Sidebar::boxHighlightedEvent(SidebarBox * item)
{
    item->setHighlighted( true );

    if ( m_highlightedBox )
        m_highlightedBox->setHighlighted(false);

    m_highlightedBox = item;
}

void Sidebar::boxSelectedEvent(SidebarBox * item, bool selected)
{
    kDebug(5001) << "box selected " << selected << endl;
    item->showChildren(selected);
}

void Sidebar::addedTransferGroupEvent(TransferGroupHandler * group)
{
    GroupBox * g = new GroupBox(group, m_downloadsBox, this);
    //TODO get rid of the m_groupsMap. Use the deleted event instead
    m_groupsMap[group->name()] = g;
}

void Sidebar::removedTransferGroupEvent(TransferGroupHandler * group)
{
/*    m_downloadsBox->removeChild(m_groupsMap[group->name()]);
    removeItem(m_downloadsBox);
    //TODO get rid of the m_groupsMap
    m_groupsMap.remove(group->name());*/
}

void Sidebar::timerEvent( QTimerEvent * e )
{
    kDebug(5001) << "timerEvent" << endl;

    //Here I have to make a copy of the list to assure that an asyncronous
    //call to stopTimer() doesn't make all crash
    QList<SidebarBox *> timersToRemove = m_timersToRemove;

    QList<SidebarBox *>::iterator it = timersToRemove.begin();
    QList<SidebarBox *>::iterator itEnd = timersToRemove.end();

    for( ; it!=itEnd ; ++it )
    {
        m_activeTimers.removeAll(*it);
    }

    m_timersToRemove.clear();
    if( m_activeTimers.size() == 0 )
        killTimer(m_timerId);

    layout()->setEnabled(false);

    foreach(SidebarBox * it, m_activeTimers)
    {
        it->timerEvent();
    }

    layout()->setEnabled(true);
    layout()->update();
}

#include "sidebar.moc"
