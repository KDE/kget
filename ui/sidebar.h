#ifndef _SIDEBAR_H
#define _SIDEBAR_H

#include <qscrollview.h>
#include <qwidget.h>
#include <qvaluelist.h>
#include <qmap.h>

#include "core/viewinterface.h"

class Sidebar;

class SidebarItem : public QWidget
{
    Q_OBJECT

public:
    SidebarItem( QWidget * parent, Sidebar * sidebar );

    void addChild(SidebarItem *);
    void setText(const QString& string);

public slots:
    void showChildren( bool show = true );
    void setSelected( bool selected = true );

signals:
    void selected(SidebarItem *);

protected:
    void mouseDoubleClickEvent ( QMouseEvent * e );
    void mousePressEvent( QMouseEvent * e );
    void paintEvent ( QPaintEvent * );

    Sidebar * m_sidebar;
    QString   m_text;
    bool      m_showChildren;
    bool      m_isSelected;

private:
    QValueList<SidebarItem *> * m_childItems;
    QMap<QString, class GroupFolder *> m_groupsMap;
};


class DownloadsFolder : public SidebarItem
{
Q_OBJECT

public:
    DownloadsFolder( QWidget * parent, Sidebar * sidebar );

protected:
    void paintEvent ( QPaintEvent * );
};

class GroupFolder : public SidebarItem
{
Q_OBJECT

public:
    GroupFolder( QWidget * parent, Sidebar * sidebar );

protected:
    void paintEvent ( QPaintEvent * );
};

class Sidebar : public QScrollView, public ViewInterface
{
Q_OBJECT

    friend class SidebarItem;

public:
    Sidebar( QWidget * parent = 0, const char * name = 0 );

    //Methods reimplemented from the ViewInterface class
    void schedulerAddedGroups( const GroupList& );
    void schedulerRemovedGroups( const GroupList& );

public slots:
    void slotItemSelected(SidebarItem *);

private:
    class QVBox *     m_layout;
    DownloadsFolder * m_dItem;
    QMap<QString, GroupFolder *> m_groupsMap;
    QValueList<SidebarItem *> m_items;
};

#endif
