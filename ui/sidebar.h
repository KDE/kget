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
#include <qpixmapcache.h>
#include <qmap.h>

#include "core/viewinterface.h"

class Sidebar;

class SidebarItem : public QListBoxItem
{
public:
    SidebarItem( Sidebar * sidebar );
    ~SidebarItem();

    int height(const QListBox * lb) const;
    int width(const QListBox * lb) const;

    void setText(const QString& string);

    void addChild(SidebarItem *);
    void removeChild(SidebarItem *);

    void showChildren( bool show = true );
    void select();

    /**
     * This method updates all the pixmaps that are used to draw the items
     */
    void updatePixmaps();

    /**
     * This is public to be accessed by @see Sidebar::paintCell
     */
    void paint ( QPainter * );

protected:
    void setVisible(bool visible = true);

    Sidebar *    m_sidebar;
    QString      m_text;
    bool         m_isVisible;
    bool         m_showChildren;

    //Pixmaps
    QPixmap *    m_pixFunsel;
    QPixmap *    m_pixFsel;
    QPixmap *    m_pixTgrad;
    QPixmap *    m_pixBgrad;
    QPixmap *    m_pixPlus;
    QPixmap *    m_pixMinus;

private:
    QValueList<SidebarItem *> * m_childItems;
};


class DownloadsFolder : public SidebarItem
{
public:
    DownloadsFolder( Sidebar * sidebar );

    /**
     * This is public to be accessed by @see Sidebar::paintCell
     */
    void paint ( QPainter * );
};

class GroupFolder : public SidebarItem
{
public:
    GroupFolder( Sidebar * sidebar );

    /**
     * This is public to be accessed by @see Sidebar::paintCell
     */
    void paint ( QPainter * );
};

class Sidebar : public QListBox, public ViewInterface
{
Q_OBJECT

    friend class SidebarItem;

public:
    Sidebar( QWidget * parent = 0, const char * name = 0 );

protected:
    void paletteChange ( const QPalette & oldPalette );
    void paintCell ( QPainter * p, int row, int col );

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
