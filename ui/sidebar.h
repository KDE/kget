/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _SIDEBAR_H
#define _SIDEBAR_H

#include <qlistbox.h>
#include <qwidget.h>
#include <qvaluelist.h>
#include <qmap.h>

#include "core/viewinterface.h"

class Sidebar;

class SidebarItem : public QListBoxItem
{
public:
    SidebarItem( Sidebar * sidebar );

    int height(const QListBox * lb) const;
    int width(const QListBox * lb) const;

    void setText(const QString& string);

    void addChild(SidebarItem *);
    void removeChild(SidebarItem *);

    void showChildren( bool show = true );
    void select();

protected:
    void setVisible(bool visible = true);
    void paint ( QPainter * );

    Sidebar * m_sidebar;
    QString   m_text;
    bool      m_isVisible;
    bool      m_showChildren;

private:
    QValueList<SidebarItem *> * m_childItems;
};


class DownloadsFolder : public SidebarItem
{
public:
    DownloadsFolder( Sidebar * sidebar );

protected:
    void paint ( QPainter * );
};

class GroupFolder : public SidebarItem
{
public:
    GroupFolder( Sidebar * sidebar );

protected:
    void paint ( QPainter * );
};

class Sidebar : public QListBox, public ViewInterface
{
Q_OBJECT

    friend class SidebarItem;

public:
    Sidebar( QWidget * parent = 0, const char * name = 0 );

public slots:
    void slotItemSelected(QListBoxItem *);

    //Methods reimplemented from the ViewInterface class
    void schedulerAddedGroups( const GroupList& );
    void schedulerRemovedGroups( const GroupList& );

private:
    DownloadsFolder * m_dItem;
    QMap<QString, GroupFolder *> m_groupsMap;
};

#endif
