/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <qimage.h>
#include <qtoolbutton.h>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QTimerEvent>
#include <QList>

#include <klocale.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kiconeffect.h>
#include <kdebug.h>

#include "core/model.h"
#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "sidebar.h"

SidebarBox::SidebarBox( Sidebar * sidebar, int itemHeight, bool enableAnimations )
    : QWidget( sidebar ),
      m_sidebar(sidebar),
      m_enableAnimations( enableAnimations ),
      m_isHighlighted( false ),
      m_isShown( true ),
      m_itemHeight( itemHeight ),
      m_showChildren( true ),
      m_pixFunsel( 0 ),
      m_pixFsel( 0 ),
      m_pixTgrad( 0 ),
      m_pixBgrad( 0 ),
      m_pixPlus( 0 ),
      m_pixMinus( 0 )
{
    setFixedHeight(0);
    setMinimumWidth(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setUpdatesEnabled(true);
    show();

    setShown(m_isShown);
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
    if(m_childBoxes.empty())
        m_sidebar->insertItem( child, this );
    else
        m_sidebar->insertItem( child, m_childBoxes.last() );

    child->setShown(m_showChildren);
    m_childBoxes.push_back(child);
}

void SidebarBox::removeChild( SidebarBox * child )
{
    m_sidebar->removeItem(child);
    m_childBoxes.remove(child);
}

void SidebarBox::showChildren( bool show )
{
    QList<SidebarBox *>::iterator it = m_childBoxes.begin();
    QList<SidebarBox *>::iterator itEnd = m_childBoxes.end();

    for(; it!=itEnd; it++)
    {
        (*it)->setShown(show);
    }
}

void SidebarBox::setHighlighted(bool highlighted)
{
    m_isHighlighted = highlighted;
}

void SidebarBox::setSelected(bool selected)
{
    m_showChildren = selected;
    showChildren(m_showChildren);
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
            setFixedHeight(m_itemHeight);
        else
            setFixedHeight(0);
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
    QImage  img = DesktopIcon("folder_open", 32).convertToImage();
    KIconEffect::toGamma(img, 0);
    delete m_pixFsel;
    m_pixFsel = new QPixmap(img);

    //m_pixTgrad: top gradient
    delete m_pixTgrad;
    m_pixTgrad = new QPixmap( KImageEffect::gradient( 
                 QSize( 1, 5 ),
                 m_sidebar->palette().active().highlight().light(150),
                 m_sidebar->palette().active().highlight(),
                 KImageEffect::VerticalGradient ) );

    //m_pixBgrad: bottom gradient
    delete m_pixBgrad;
    m_pixBgrad = new QPixmap( KImageEffect::gradient( 
                 QSize( 1, 5 ),
                 m_sidebar->palette().active().highlight(),
                 m_sidebar->palette().active().highlight().light(150),
                 KImageEffect::VerticalGradient ) );

    //m_pixPlus: plus simbol
    delete m_pixPlus;
    m_pixPlus = new QPixmap(SmallIcon("edit_add", 16));

    //m_pixMinus: minus simbol
    delete m_pixMinus;
    m_pixMinus = new QPixmap(SmallIcon("edit_remove", 16));
}

void SidebarBox::paintEvent( QPaintEvent * event )
{
    int w = width();
    int h = height();

    QPainter p(this);
    p.setClipRegion(event->region());

    if(!m_isHighlighted)
        p.fillRect(0,0,w,h, QBrush(m_sidebar->palette().active().base()));
    else
    {
        QColor c = m_sidebar->palette().active().highlight();

        p.fillRect(0,5,w, h-5-4, c);
        p.drawTiledPixmap(0,0, w, 5, *m_pixTgrad);
        p.drawTiledPixmap(0,h-4, w, 4, *m_pixBgrad);
        p.setPen(QPen(c.dark(130),1));
        p.drawLine(0,0,w, 0);
        p.drawLine(0,h-1,w, h-1);
    }
}

void SidebarBox::timerEvent()
{
    kdDebug() << "SidebarBox::timerEvent " << endl;
    if( m_isShown )
    {
        if( (height() + 3) > m_itemHeight )
        {
            setFixedHeight(m_itemHeight);
            m_sidebar->stopTimer(this);
        }
        else
            setFixedHeight( height() + 3 );
    }
    else
    {
        if( (height() - 3) < 0)
        {
            setFixedHeight(0);
            m_sidebar->stopTimer(this);
        }
        else
            setFixedHeight( height() - 3 );
    }
    repaint();
    kdDebug() << "height = " << height() << endl; 
}

DownloadsBox::DownloadsBox( Sidebar * sidebar )
    : SidebarBox( sidebar, 38, false)
{
    m_sidebar->boxHighlighedEvent(this);
}

void DownloadsBox::paintEvent ( QPaintEvent * event )
{
//     kdDebug() << "DownloadsBox paint event" << endl;

    SidebarBox::paintEvent( event );

    QPainter p(this);
    p.setClipRegion(event->region());

    if(m_isHighlighted)
        p.drawPixmap(10, 2, *m_pixFsel);
    else
        p.drawPixmap(10, 2, *m_pixFunsel);

    if(m_showChildren)
        p.drawPixmap(25, 20, *m_pixMinus);
    else
        p.drawPixmap(25, 20, *m_pixPlus);

    QFont f(p.font());
    f.setBold(m_isHighlighted);
    p.setFont( f );
    if (m_isHighlighted)
        p.setPen( m_sidebar->palette().active().highlightedText());
    else
        p.setPen( m_sidebar->palette().active().foreground());
    p.drawText(50, 11, width(), height()-7, Qt::AlignLeft, i18n("Downloads"));
}

GroupBox::GroupBox( TransferGroupHandler * group, DownloadsBox * dbox, Sidebar * sidebar )
    : SidebarBox( sidebar, 38, true ),
      m_downloadsBox( dbox ),
      m_group( group )
{
    m_group->addObserver(this);
    m_downloadsBox->addChild(this);
/*    QToolButton * bt = new QToolButton( sidebar->viewport() );
    QRect area = sidebar->itemRect(this);
    kdDebug() << "x, y = " << area.x() << " " << area.y() << endl;
    bt->setGeometry(area.x()+10, area.y()+8, 64, 64);
    bt->setIcon(SmallIcon("tool_resume"));*/
}

GroupBox::~GroupBox()
{

}

void GroupBox::paintEvent ( QPaintEvent * event )
{
    SidebarBox::paintEvent( event );

    QPainter p(this);
    p.setClipRegion(event->region());

    if(m_isHighlighted)
        p.drawPixmap(30, 2, *m_pixFsel);
    else
        p.drawPixmap(30, 2, *m_pixFunsel);

    QFont f(p.font());
    f.setBold(m_isHighlighted);
    p.setFont( f );
    if(m_isHighlighted)
        p.setPen( m_sidebar->palette().active().highlightedText());
    else
        p.setPen( m_sidebar->palette().active().foreground());
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
    kdDebug() << "GroupBox::transferChangedEvent -> ENTERING" << endl;

    if(transfer->changesFlags(this) & Transfer::Tc_Status)
    {
        kdDebug() << "GroupBox: aaa" << endl;

        if(transfer->status() == Job::Running)
        {
            kdDebug() << "Creating TransferBox: transfer status = "
                      << transfer->statusText() << endl;

            //Better check if we already created the TransferBox for this transfer
            QList<SidebarBox *>::iterator it = m_childBoxes.begin();
            QList<SidebarBox *>::iterator itEnd = m_childBoxes.end();

            for( ; it!=itEnd ; ++it )
            {
                if(static_cast<TransferBox *>(*it)->transfer() == transfer)
                    return;
            }

            TransferBox * g = new TransferBox(transfer, this, m_sidebar);
        }
    }

    kdDebug() << "GroupBox: bbb" << endl;
    transfer->resetChangesFlags(this);

    kdDebug() << "GroupBox::transferChangedEvent -> LEAVING" << endl;
}

TransferBox::TransferBox( TransferHandler * transfer, GroupBox * gBox, Sidebar * sidebar )
    : SidebarBox( sidebar, 20, true ),
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

/*    if(isSelected())
        p.drawPixmap(30, 2, *m_pixFsel);
    else
        p.drawPixmap(30, 2, *m_pixFunsel);

    QFont f(p.font());
    f.setBold(isSelected());
    p.setFont( f );
    if(isSelected())
        p.setPen( m_sidebar->palette().active().highlightedText());
    else
        p.setPen( m_sidebar->palette().active().foreground());
    p.drawText(70, 11, width(m_sidebar), height(m_sidebar)-7, Qt::AlignLeft, m_text);*/
    p.drawText(80, 1, width(), height()-7, Qt::AlignLeft, m_transfer->source().filename());
}

void TransferBox::transferChangedEvent(TransferHandler * transfer)
{
    kdDebug() << "TransferBox::transferChangedEvent() ENTERING" << endl;
    if(transfer->changesFlags(this) & Transfer::Tc_Status)
    {
        if(transfer->status() != Job::Running)
        {
            m_transfer->delObserver(this);
            kdDebug() << "###############  OBSERVER DELETED" << endl;
            m_groupBox->removeChild(this);
            kdDebug() << "TransferBox::transferChangedEvent() LEAVING1" << endl;
            return;
        }
    }
    m_transfer->resetChangesFlags(this);
    kdDebug() << "TransferBox::transferChangedEvent() LEAVING2" << endl;
}


Sidebar::Sidebar( QWidget * parent, const char * name )
    : QWidget( parent, name ),
      m_numBoxes(0),
      m_timerInterval(10),
      m_highlightedBox(0)
{
    setUpdatesEnabled(true);

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_downloadsBox = new DownloadsBox(this);
    insertItem(m_downloadsBox);

    m_layout->addStretch(1);
    m_numBoxes++;

    show();

    Model::addObserver(this);
}

void Sidebar::insertItem( SidebarBox * box, SidebarBox * after )
{
    int index;

    if(after == 0)
        index = m_numBoxes;
    else
        index = m_layout->indexOf(after) + 1;

    if( index != -1 )
        m_layout->insertWidget( index, box );

    m_numBoxes++;
}

void Sidebar::removeItem( SidebarBox * box )
{
    m_layout->removeWidget( box );

    m_numBoxes--;
}

void Sidebar::startTimer( SidebarBox * item )
{
    if( m_activeTimers.size() == 0 )
        m_timerId = QWidget::startTimer( m_timerInterval );

    if(m_activeTimers.find(item) == m_activeTimers.end())
        m_activeTimers.append( item );
}

void Sidebar::stopTimer( SidebarBox * item )
{
    kdDebug() << "Sidebar::stopTimer 111" << endl;
    m_timersToRemove.append( item );
    kdDebug() << "Sidebar::stopTimer 222" << endl;
}

// void Sidebar::paintCell( QPainter * p, int row, int /*col*/ )
// {
//     SidebarBox * i = (SidebarBox *)item( row );
//     int itemWidth = viewport()->width();
//     int itemHeight = i->height( this );
// 
//     // paint pixmap on a back buffer
//     QPixmap backMap( itemWidth, itemHeight );
//     QPainter backPainter( &backMap );
//     i->paint( &backPainter );
//     backPainter.end();
// 
//     // blit the backMap to the screen
//     p->drawPixmap( 0, 0, backMap );
// }

void Sidebar::boxHighlighedEvent(SidebarBox * item)
{
    if(m_highlightedBox)
        m_highlightedBox->setHighlighted(false);

    m_highlightedBox = item;
    m_highlightedBox->setHighlighted( true );
    item->setHighlighted( true );
}

void Sidebar::boxSelectedEvent(SidebarBox * item)
{
    if(m_highlightedBox)
        m_highlightedBox->setSelected(false);

    m_highlightedBox = item;
    m_highlightedBox->setSelected( true );
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
    //Here I have to make a copy of the list to assure that an asyncronous
    //call to stopTimer() doesn't make all crash
    QList<SidebarBox *> timersToRemove = m_timersToRemove;

    QList<SidebarBox *>::iterator it = timersToRemove.begin();
    QList<SidebarBox *>::iterator itEnd = timersToRemove.end();

    for( ; it!=itEnd ; ++it )
    {
        m_activeTimers.remove(*it);
    }

    m_timersToRemove.clear();
    if( m_activeTimers.size() == 0 )
        killTimer(m_timerId);

    it = m_activeTimers.begin();
    itEnd = m_activeTimers.end();

    for( ; it!=itEnd ; ++it )
    {
        (*it)->timerEvent();
    }
}

#include "sidebar.moc"
