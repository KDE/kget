#include <qpainter.h>
#include <qvbox.h>
#include <qimage.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kimageeffect.h>
#include <kiconeffect.h>

#include "sidebar.h"

SidebarItem::SidebarItem( QWidget * parent, Sidebar * sidebar )
    : QWidget( parent ),
      m_sidebar(sidebar),
      m_showChildren( true ),
      m_isSelected( false )
{
    setFixedSize(200, 37);

    m_childItems = new QValueList<SidebarItem *>();
    m_sidebar->m_items.push_back(this);

    connect(this,      SIGNAL(selected(SidebarItem *)),
            m_sidebar, SLOT(slotItemSelected(SidebarItem *)) );
}

void SidebarItem::addChild( SidebarItem * child )
{
    m_childItems->push_front(child);
    child->setShown( m_showChildren );
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
        (*it)->setShown( show );
    }
}

void SidebarItem::setSelected( bool selected )
{
    if( m_isSelected != selected )
    {
        m_isSelected = selected;
        repaint(); 
    }
}

void SidebarItem::mouseDoubleClickEvent ( QMouseEvent * e )
{
    showChildren( m_showChildren=!m_showChildren );
    repaint();
    emit( selected(this) );
}

void SidebarItem::mousePressEvent ( QMouseEvent * e )
{
    emit( selected(this) );
}

void SidebarItem::paintEvent ( QPaintEvent * p)
{
    QPainter pt(this);

    static QPixmap * topGradient = new QPixmap(
            KImageEffect::gradient( 
            QSize( 1, 5 ),
            palette().active().highlight().light(150),
            palette().active().highlight(),
            KImageEffect::VerticalGradient ) );

    static QPixmap * bottomGradient = new QPixmap(
            KImageEffect::gradient( 
            QSize( 1, 5 ),
            palette().active().highlight(),
            palette().active().highlight().light(150),
            KImageEffect::VerticalGradient ) );

    if(!m_isSelected)
        pt.fillRect(0,0,width(),height(), QBrush(Qt::white));
    else
    {
        pt.fillRect(0,5,width(), height()-5-4, palette().active().highlight());
        pt.drawTiledPixmap(0,0, width(), 5, *topGradient);
        pt.drawTiledPixmap(0,height()-4, width(), 4, *bottomGradient);
        pt.setPen(QPen(palette().active().highlight().dark(130),1));
        pt.drawLine(0,0,width(), 0);
        pt.drawLine(0,height()-1,width(), height()-1);
    }
}

DownloadsFolder::DownloadsFolder( QWidget * parent, Sidebar * sidebar )
    : SidebarItem( parent, sidebar )
{

}

void DownloadsFolder::paintEvent ( QPaintEvent * p )
{
    SidebarItem::paintEvent(p);

    QPainter pt(this);

    QPixmap icon = DesktopIcon("folder", 32);

    if(m_isSelected)
    {
        QImage  img = DesktopIcon("folder", 32).convertToImage();
        KIconEffect::toGamma(img, 0);
        pt.drawPixmap(10, 2, QPixmap(img));
    }
     else
    {
        pt.drawPixmap(10, 2, icon);
    }

    if(m_showChildren)
        pt.drawPixmap(25, 20, SmallIcon("edit_remove", 16));
    else
        pt.drawPixmap(25, 20, SmallIcon("edit_add", 16));

    QFont f(pt.font());
    f.setBold(true);
    pt.setFont( f );
    pt.drawText(50, 11, width(), height()-7, Qt::AlignLeft, m_text);
}

GroupFolder::GroupFolder( QWidget * parent, Sidebar * sidebar )
    : SidebarItem( parent, sidebar )
{

}

void GroupFolder::paintEvent ( QPaintEvent * p )
{
    SidebarItem::paintEvent(p);

    QPainter pt(this);

    QPixmap icon = DesktopIcon("folder", 32);

    if(m_isSelected)
    {
        QImage  img = DesktopIcon("folder", 32).convertToImage();
        KIconEffect::toGamma(img, 0);
        pt.drawPixmap(30, 2, QPixmap(img));
    }
    else
    {
        pt.drawPixmap(30, 2, icon);
    }

    pt.drawText(70, 11, width(), height()-7, Qt::AlignLeft, m_text);
}

Sidebar::Sidebar( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    viewport()->setBackgroundMode( Qt::PaletteBase );

    m_layout = new QVBox( viewport() );
    m_layout->setFixedWidth( viewport()->width() );
    addChild(m_layout);

    m_dItem = new DownloadsFolder(m_layout, this);
    m_dItem->setText(i18n("Downloads"));
}

void Sidebar::schedulerAddedGroups( const GroupList& list )
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();

    for(; it != endList; ++it)
    {
        QString groupName = (*it)->info().name;

        GroupFolder * g = new GroupFolder(m_layout, this);
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
        m_items.remove(m_groupsMap[(*it)->info().name]);
        delete(m_groupsMap[(*it)->info().name]);
    }
}

void Sidebar::slotItemSelected( SidebarItem * item )
{
    QValueList<SidebarItem *>::iterator it = m_items.begin();
    QValueList<SidebarItem *>::iterator endList = m_items.end();

    for( ;it != endList; ++it )
    {
        (*it)->setSelected( (*it) == item );
    }
}

#include "sidebar.moc"
