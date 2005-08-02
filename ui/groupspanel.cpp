/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <klistview.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/global.h>

#include <qlayout.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "groupspanel.h"

#include "core/group.h"
#include "core/scheduler.h"

GroupItem::GroupItem(KListView * parent, Group * g)
    : Q3ListViewItem(parent),
      group(g)
{

}

GroupItem::~GroupItem()
{

}

void GroupItem::updateContents(bool updateAll)
{
    Group::Info info = group->info();
    
    if(updateAll)
    {
        setPixmap(0, SmallIcon("package") );
        setText(0, info.name);
        setText(2, KIO::convertSize(info.totalSize));
    }
}

void GroupItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    Q3ListViewItem::paintCell(p, cg, column, width, align);
    
    if(column == 1)
    {
        Group::Info info = group->info();
        int rectWidth = (int)((width-4) * info.percent / 100);
        
        p->fillRect(2,2,rectWidth, height()-4, cg.brush(QColorGroup::Highlight));
        p->setPen(cg.foreground());
        p->drawRect(2,2,rectWidth, height()-4);
        p->drawText(2,2,width, height()-4, Qt::AlignCenter, 
                    QString().setNum(info.percent) + "%");
    }
}

GroupsPanel::GroupsPanel( QWidget * parent, const char * name)
    : QWidget(parent, name)
{
    listView = new KListView(this);

    listView->addColumn("Group", 80);
    listView->addColumn("Progress", 80);
    listView->addColumn("Size", 50);
    
    listView->setAllColumnsShowFocus(true);
    listView->setSelectionMode(Q3ListView::Single);
    listView->setSorting(0);

    
    newGroupBt = new KPushButton(i18n("New"), this);    
    delGroupBt = new KPushButton(i18n("Delete"), this);
    editGroupBt = new KPushButton(i18n("Edit"), this);
    showGroupBt = new KPushButton(i18n("Show"), this);
    
    QVBoxLayout * layout1 = new QVBoxLayout(this);
    QHBoxLayout * layout2 = new QHBoxLayout(this);

    layout2->addWidget(newGroupBt);
    layout2->addWidget(delGroupBt);
    layout2->addWidget(editGroupBt);
    
    layout1->addWidget(listView);
    layout1->addLayout(layout2);
    layout1->addWidget(showGroupBt);
    
    connect(newGroupBt, SIGNAL(clicked()), this, SLOT(slotNewGroup()));
    connect(delGroupBt, SIGNAL(clicked()), this, SLOT(slotDelGroup()));
}

GroupsPanel::~GroupsPanel()
{

}

void GroupsPanel::schedulerCleared()
{

}

void GroupsPanel::schedulerAddedItems( const TransferList& list)
{

}

void GroupsPanel::schedulerRemovedItems( const TransferList& list)
{

}

void GroupsPanel::schedulerChangedItems( const TransferList& list)
{

}

void GroupsPanel::schedulerAddedGroups( const GroupList& list ) 
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        GroupItem * newItem = new GroupItem(listView, *it);
        groupsMap[(*it)->info().name] = newItem;
        
        

        newItem->updateContents(true);
    }
}

void GroupsPanel::schedulerRemovedGroups( const GroupList& list)
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        delete(groupsMap[(*it)->info().name]);
    }

}

void GroupsPanel::schedulerChangedGroups( const GroupList& list)
{

}

void GroupsPanel::slotNewGroup()
{
    bool ok;    

    QString name = KInputDialog::getText(i18n("Please insert the name for the new group:"), i18n("New group"), i18n("Group name"), &ok, this);

    if((ok))
    {
        if(groupsMap.contains(name))
        {
            KMessageBox::error(this, i18n("There is already a group with this name. \n Please retry with a new group name"));            
            return;
        }
        schedAddGroup(GroupList(Group(name)));
    }
}

void GroupsPanel::slotDelGroup()
{
    kdDebug() << "delGroup" << endl;
    
    GroupList l = getSelectedList();
    if(l.size() != 0)    
        schedDelGroup( getSelectedList() );
}

void GroupsPanel::slotEditGroup()
{

}

void GroupsPanel::slotShowGroup()
{

}

GroupList GroupsPanel::getSelectedList()
{
    if(listView->selectionMode() == KListView::Single)
    {
        if (GroupItem * g = (GroupItem *) listView->selectedItem())
            return( GroupList(*(g)->getGroup()) );
        else    
            return GroupList();
    }
    
    GroupList list;

    Q3ListViewItemIterator it(listView);
    
    for(; *it != 0; it++)
    {
        if( listView->isSelected(*it) )
            list.addGroup( *( static_cast<GroupItem*>(*it) )->getGroup() );
    }
    
    return list;
}

#include "groupspanel.moc"
