/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <qdom.h>
#include <qstring.h>

#include "globals.h"
#include "group.h"


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
    
    push_front(new Group(group));
    
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
    if (Group * g = getGroup(group.info().name))
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

void GroupList::modifyGroup(const QString& groupName, Group * group)
{
    if(Group * g = getGroup(groupName))
        delGroup(*g);
    addGroup(*group);
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

#include "group.moc"
