/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _GROUPS_PANEL_H
#define _GROUPS_PANEL_H

#include "core/viewinterface.h"

#include <QWidget>
#include <QMap>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <klistview.h>
#include <kiconloader.h>

class KListView;
class KPushButton;
class QVBoxLayout;
class QHBoxLayout;
class GroupList;
class Group;
class GroupsPanel;

class GroupItem : public Q3ListViewItem
{
    public:
    GroupItem(KListView * parent, Group * g);
    ~GroupItem();

    void updateContents(bool updateAll=false);
    Group * getGroup() const {return group;}    
    
    void paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align);
    
    private:
    Group * group;
    GroupsPanel * view;
};

class GroupsPanel : public QWidget, public ViewInterface
{
Q_OBJECT
    public:
    
    GroupsPanel( QWidget * parent = 0, const char * name = 0 );
    ~GroupsPanel();

    public slots:
    void slotNewGroup();
    void slotDelGroup();
    void slotEditGroup();
    void slotShowGroup();

	virtual void schedulerCleared();
	virtual void schedulerAddedItems( const TransferList& );
	virtual void schedulerRemovedItems( const TransferList& );
	virtual void schedulerChangedItems( const TransferList& );
    virtual void schedulerAddedGroups( const GroupList& );
    virtual void schedulerRemovedGroups( const GroupList& );
    virtual void schedulerChangedGroups( const GroupList& );
    
    private:
    GroupList getSelectedList();
    
    KPushButton * newGroupBt;
    KPushButton * delGroupBt;
    KPushButton * editGroupBt;
    
    KPushButton * showGroupBt;
    
    KListView * listView;
    QVBoxLayout * layout1;
    QHBoxLayout * layout2;

    QMap<QString, GroupItem *> groupsMap;
};

#endif
