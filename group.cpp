/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <qdom.h>
#include <qstring.h>
#include <qmap.h>

#include "globals.h"
#include "group.h"
#include "transferlist.h"


Group::Group(const QString& name)
{
    gInfo.speed=0;
    gInfo.totalSize=0;
    gInfo.processedSize=0;
    gInfo.percent=50;
    gInfo.name=name;

}

Group::Group(QDomNode * n)
{
    read(n);
}

Group::TransferInfoCache Group::updatedInfoCache(Transfer * t)
{
    Transfer::Info ti = t->info();
    TransferInfoCache ic;
    
    ic.totalSize = ti.totalSize;
    ic.processedSize = ti.processedSize;
    ic.speed = ti.speed;
    
    return ic;
}

void Group::updatePercent()
{
    kdDebug() << "updatePercent" << gInfo.processedSize << " / " << gInfo.totalSize << endl;
    if(gInfo.totalSize != 0)
        gInfo.percent = 100 * gInfo.processedSize / gInfo.totalSize;
    else
        gInfo.percent = 0;
}    

void Group::addTransfer(Transfer * t)
{
    sDebugIn << endl;    

    TransferInfoCache ic = updatedInfoCache(t);
    transfersMap[t] = ic;
    
    gInfo.totalSize+=ic.totalSize;
    gInfo.processedSize+=ic.processedSize;
    gInfo.speed+=ic.speed;    
    updatePercent();

    sDebugOut << endl;    
}

void Group::delTransfer(Transfer * t)
{
    sDebugIn << endl;
    
    TransferInfoCache ic = transfersMap[t];
    
    gInfo.totalSize-=ic.totalSize;
    gInfo.processedSize-=ic.processedSize;
    gInfo.speed-=ic.speed;    
    updatePercent();

    transfersMap.erase(t);

    sDebugOut << endl;
}

void Group::changedTransfer(Transfer * t, Transfer::TransferChanges tc)
{
    switch(tc)
    {
        case Transfer::Tc_ProcessedSize:
            Transfer::Info ti = t->info();
            gInfo.processedSize-=transfersMap[t].processedSize;
            gInfo.processedSize+=ti.processedSize;
            updatePercent();
            
            transfersMap[t] = updatedInfoCache(t);
            break;            
    }
}

bool Group::read(QDomNode * n)
{
    gInfo.speed=0;
        
    QDomElement e = n->toElement();
    
    gInfo.name = e.attribute("Name");
    gInfo.totalSize = e.attribute("TotalSize").toInt();
    gInfo.processedSize = e.attribute("ProcessedSize").toInt();
    gInfo.percent = e.attribute("Percent").toULong();
    
    return true;
}

void Group::write(QDomNode * n) const
{
    sDebugIn << "name:" << gInfo.name << endl;
    
    QDomElement e = n->ownerDocument().createElement("Group");
    n->appendChild(e);

    
        
    e.setAttribute("Name", gInfo.name);
    e.setAttribute("TotalSize", gInfo.totalSize);    
    e.setAttribute("ProcessedSize", gInfo.processedSize);    
    e.setAttribute("Percent", gInfo.percent);

    sDebugOut << endl;
}

void Group::about() const
{
    kdDebug() << "  Group name= " << gInfo.name << endl; ;
}

GroupList::GroupList()
{

}

GroupList::GroupList(const GroupList & list)
{
    addGroups(list);
}

GroupList::GroupList(Group group)
{
    addGroup(group);
}

Group * GroupList::getGroup(const QString& groupName) const
{
    constIterator it = begin();
    constIterator endList = end();
    
    for(;it!=endList; ++it)
    {
        if((*it)->info().name == groupName)
            return *it;
    }
    return 0;
}

void GroupList::addGroup(Group group)
{
    sDebugIn << endl;
    
    Group * newGroup = new Group(group);
    
    groupsMap[group.info().name] = newGroup;
    push_front(newGroup);
    
    sDebugOut << endl;
}

void GroupList::addGroups(const GroupList& list)
{
    sDebugIn << endl;
    
    constIterator it = list.begin();
    constIterator endList = list.end();
    
    for(;it!=endList; ++it)
    {
        addGroup(**it);
    }
    
    sDebugOut << endl;
}

void GroupList::delGroup(Group group)
{
    QString groupName = group.info().name;

    groupsMap.erase(groupName);
    if (Group * g = getGroup(groupName))
        remove(g);
}

void GroupList::delGroups(const GroupList& list)
{
    sDebugIn << endl;
    
    constIterator it = list.begin();
    constIterator endList = list.end();
    
    for(;it!=endList; ++it)
    {
        delGroup(**it);
    }
    
    sDebugOut << endl;
}

void GroupList::modifyGroup(const QString& groupName, Group group)
{
    Group * g;
    
    if(g = getGroup(groupName))
        delGroup(*g);
    addGroup(group);
}

bool GroupList::read(QDomDocument * doc)
{
    sDebugIn << endl;

    QDomElement root = doc->documentElement();
    QDomNodeList nodeList = root.elementsByTagName("Group");
    
    int nItems = nodeList.length();
    
    for(int i=0; i<nItems; i++)
    {
        Group g(&nodeList.item(i));
        addGroup(g);
    }
    
    
    sDebugOut << endl;
    return true;
}


void GroupList::write(QDomDocument * doc) const
{
    sDebugIn << endl;
    
    QDomElement * e = &doc->documentElement();
    
    constIterator it = begin();
    constIterator endList = end();

    for (; it != endList; ++it)
        (*it)->write(e);
   
    sDebugOut << endl;
}

void GroupList::about() const
{
    constIterator it = begin();
    constIterator endList = end();
    
    kdDebug() << "GroupList" << endl;
    
    for (; it != endList; ++it)
        (*it)->about();
}

void GroupList::slotAddedTransfers(const TransferList& list)
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();

    for(; it!=endList; ++it)
    {
        QString tName = (*it)->info().group;
        if(groupsMap.contains(tName))
            groupsMap[tName]->addTransfer(*it);
    }    
}

void GroupList::slotRemovedTransfers(const TransferList& list)
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();

    for(; it!=endList; ++it)
    {
        QString tName = (*it)->info().group;
        if(groupsMap.contains(tName))
            groupsMap[tName]->delTransfer(*it);
    }    
}

void GroupList::slotChangedTransfers(const TransferList& list)
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();

    for(; it!=endList; ++it)
    {
        QString tName = (*it)->info().group;
        if(groupsMap.contains(tName))
        {
            Transfer::TransferChanges tc = (*it)->changesFlags(this);
            groupsMap[tName]->changedTransfer(*it, tc);
            (*it)->resetChangesFlags(this);
        }
    }    
}

#include "group.moc"
