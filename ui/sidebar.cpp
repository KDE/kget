/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qpainter.h>
#include <qvbox.h>
#include <qimage.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kiconeffect.h>
#include <kdebug.h>

#include "sidebar.h"


SidebarItem::SidebarItem( Sidebar * sidebar )
    : QListBoxItem( ),
      m_sidebar(sidebar),
      m_showChildren( true ),
      m_isVisible( true ),
      m_pixFunsel( 0 ),
      m_pixFsel( 0 ),
      m_pixTgrad( 0 ),
      m_pixBgrad( 0 ),
      m_pixPlus( 0 ),
      m_pixMinus( 0 )

{
    setCustomHighlighting(true);
    m_childItems = new QValueList<SidebarItem *>();
    updatePixmaps();
}

SidebarItem::~SidebarItem()
{
    delete m_childItems;

    delete m_pixFunsel;
    delete m_pixFsel;
    delete m_pixTgrad;
    delete m_pixBgrad;
    delete m_pixPlus;
    delete m_pixMinus;
}

int SidebarItem::height(const QListBox * lb) const
{
    if(m_isVisible)
        return 38;
    else
        return 0;
}

int SidebarItem::width(const QListBox * lb) const
{
    return lb->viewport()->width();
}

void SidebarItem::addChild( SidebarItem * child )
{
    if(m_childItems->empty())
        m_sidebar->insertItem( child, this );
    else 
        m_sidebar->insertItem( child, m_childItems->last() );

    child->setVisible(m_showChildren);
    m_childItems->push_back(child);
}

void SidebarItem::removeChild( SidebarItem * child )
{
    m_sidebar->removeItem(m_sidebar->index(child));
    m_childItems->remove(child);
}

void SidebarItem::setText( const QString& text )
{
    m_text = text;
}

void SidebarItem::showChildren( bool show )
{
    QValueList<SidebarItem *>::iterator it = m_childItems->begin();
    QValueList<SidebarItem *>::iterator itEnd = m_childItems->end();

    for(; it!=itEnd; it++)
    {
        (*it)->setVisible(show);
    }
}

void SidebarItem::select()
{
    showChildren(m_showChildren = !m_showChildren);
}

void SidebarItem::setVisible(bool visible)
{
    m_isVisible = visible;
}

void SidebarItem::updatePixmaps()
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

void SidebarItem::paint( QPainter * p)
{
    int w = width(m_sidebar);
    int h = height(m_sidebar);

    if(!isSelected())
        p->fillRect(0,0,w,h, QBrush(m_sidebar->palette().active().base()));
    else
    {
        QColor c = m_sidebar->palette().active().highlight();

        p->fillRect(0,5,w, h-5-4, c);
        p->drawTiledPixmap(0,0, w, 5, *m_pixTgrad);
        p->drawTiledPixmap(0,h-4, w, 4, *m_pixBgrad);
        p->setPen(QPen(c.dark(130),1));
        p->drawLine(0,0,w, 0);
        p->drawLine(0,h-1,w, h-1);
    }
}

DownloadsFolder::DownloadsFolder( Sidebar * sidebar )
    : SidebarItem( sidebar )
{

}

void DownloadsFolder::paint ( QPainter * p )
{
    SidebarItem::paint(p);

    if(isSelected())
        p->drawPixmap(10, 2, *m_pixFsel);
    else
        p->drawPixmap(10, 2, *m_pixFunsel);

    if(m_showChildren)
        p->drawPixmap(25, 20, *m_pixMinus);
    else
        p->drawPixmap(25, 20, *m_pixPlus);

    QFont f(p->font());
    f.setBold(isSelected());
    p->setFont( f );
    if (isSelected())
        p->setPen( m_sidebar->palette().active().highlightedText());
    else
        p->setPen( m_sidebar->palette().active().foreground());
    p->drawText(50, 11, width(m_sidebar), height(m_sidebar)-7, Qt::AlignLeft, m_text);
}

GroupFolder::GroupFolder( Sidebar * sidebar )
    : SidebarItem( sidebar )
{

}

void GroupFolder::paint( QPainter * p )
{
    SidebarItem::paint(p);

    if(isSelected())
        p->drawPixmap(30, 2, *m_pixFsel);
    else
        p->drawPixmap(30, 2, *m_pixFunsel);

    QFont f(p->font());
    f.setBold(isSelected());
    p->setFont( f );
    if(isSelected())
        p->setPen( m_sidebar->palette().active().highlightedText());
    else
        p->setPen( m_sidebar->palette().active().foreground());
    p->drawText(70, 11, width(m_sidebar), height(m_sidebar)-7, Qt::AlignLeft, m_text);
}

Sidebar::Sidebar( QWidget * parent, const char * name )
    : QListBox( parent, name )
{
    connect(this, SIGNAL(selected(QListBoxItem *)),
            this, SLOT(slotItemSelected(QListBoxItem *)));

    setColumnMode( FitToWidth );

    m_dItem = new DownloadsFolder(this);
    m_dItem->setText(i18n("Downloads"));

    insertItem(m_dItem);
}

void Sidebar::paletteChange ( const QPalette & oldPalette )
{
    for(int i=count()-1; i>=0; i--)
    {
        static_cast<SidebarItem*>(item(i))->updatePixmaps();
    }
}

void Sidebar::paintCell( QPainter * p, int row, int /*col*/ )
{
    SidebarItem * i = (SidebarItem *)item( row );
    int itemWidth = viewport()->width();
    int itemHeight = i->height( this );

    // paint pixmap on a back buffer
    QPixmap backMap( itemWidth, itemHeight );
    QPainter backPainter( &backMap );
    i->paint( &backPainter );
    backPainter.end();

    // blit the backMap to the screen
    p->drawPixmap( 0, 0, backMap );
}

void Sidebar::slotItemSelected(QListBoxItem * item)
{
    (static_cast<SidebarItem *>(item))->select();
    triggerUpdate(true);
}

void Sidebar::schedulerAddedGroups( const GroupList& list )
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();

    for(; it != endList; ++it)
    {
        QString groupName = (*it)->info().name;

        GroupFolder * g = new GroupFolder(this);
        g->setText(groupName);

        m_groupsMap[groupName] = g;

        m_dItem->addChild(g);
    }
}

void Sidebar::schedulerRemovedGroups( const GroupList& list )
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();

    for(; it != endList; ++it)
    {
        m_dItem->removeChild(m_groupsMap[(*it)->info().name]);
        m_groupsMap.remove((*it)->info().name);
    }
}

#include "sidebar.moc"
