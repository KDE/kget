#include <klistview.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qlayout.h>

#include "groupspanel.h"
#include "../group.h"

#include "../scheduler.h"

GroupItem::GroupItem(KListView * parent, Group * g)
    : QListViewItem(parent),
      group(g)
{

}

GroupItem::~GroupItem()
{

}

void GroupItem::updateContents(bool updateAll)
{
    Group::Info info = group->getInfo();
    
    setText(1, "testo");
    
    if(updateAll)
    {
        setText(0, "->"+info.name);
    }
    
/*    if(updateAll || (progressFlags & Transfer::Pc_Priority) )
    {
//         kdDebug() << "UPDATE:  priority" << endl;                
//         setText(0, QString().setNum(info.priority));
        switch(info.priority)
        {
            case 1: 
//                 setText(0, i18n("Highest") );
                setPixmap(0, SmallIcon("2uparrow") ); 
                break;
            case 2: 
//                 setText(0, i18n("High") );
                setPixmap(0, SmallIcon("1uparrow") ); 
                break;
            case 3: 
//                 setText(0, i18n("Normal") );
                setPixmap(0, SmallIcon("1rightarrow") ); 
                break;
            case 4: 
//                 setText(0, i18n("Low") );
                setPixmap(0, SmallIcon("1downarrow") ); 
                break;
            case 5: 
//                 setText(0, i18n("Lowest") );
                setPixmap(0, SmallIcon("2downarrow") ); 
                break;
            case 6: 
//                 setText(0, i18n("Highest") );
                setPixmap(0, SmallIcon("stop") ); 
                break;
        }
    }*/
    
}

void GroupItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    QListViewItem::paintCell(p, cg, column, width, align);
    
    if(column == 1)
    {
/*        Transfer::Info info = transfer->getInfo();
        float rectWidth = (width-4) * info.percent / 100;
        
        p->fillRect(2,2,rectWidth, height()-4, cg.brush(QColorGroup::Highlight));
        p->setPen(cg.foreground());
        p->drawRect(2,2,rectWidth, height()-4);
        p->drawText(2,2,width, height()-4, Qt::AlignCenter, 
                    QString().setNum(info.percent) + "%");*/
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
    listView->setSelectionMode(QListView::Single);
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

void GroupsPanel::schedulerAddedItems( TransferList list)
{

}

void GroupsPanel::schedulerRemovedItems( TransferList list)
{

}

void GroupsPanel::schedulerChangedItems( TransferList list)
{

}

void GroupsPanel::schedulerAddedGroups( const GroupList& list ) 
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        GroupItem * newItem = new GroupItem(listView, *it);
        groupsMap[(*it)->getInfo().name] = newItem;
        
        

        newItem->updateContents(true);
    }
}

void GroupsPanel::schedulerRemovedGroups( const GroupList& list)
{
    GroupList::constIterator it = list.begin();
    GroupList::constIterator endList = list.end();
    
    for(; it != endList; ++it)
    {
        delete(groupsMap[(*it)->getInfo().name]);
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

    QListViewItemIterator it(listView);
    
    for(; *it != 0; it++)
    {
        if( listView->isSelected(*it) )
            list.addGroup( *( static_cast<GroupItem*>(*it) )->getGroup() );
    }
    
    return list;
}

#include "groupspanel.moc"
