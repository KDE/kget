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


Group::Group(const Group& g)
    : QObject()
{
    gInfo = g.info();
}

Group::Group(const QString& name)
    : QObject()
{
    gInfo.speed=0;
    gInfo.totalSize=0;
    gInfo.processedSize=0;
    gInfo.percent=0;
    gInfo.name=name;

}

Group::Group(QDomNode * n)
    : QObject()
{
//    gInfo.speed=0;
    gInfo.totalSize=0;
    gInfo.processedSize=0;
    gInfo.percent=0;
    
    read(n);
}

bool Group::read(QDomNode * n)
{
    gInfo.speed=0;
        
    QDomElement e = n->toElement();
    
    gInfo.name = e.attribute("Name");
    
    return true;
}

void Group::write(QDomNode * n) const
{
    sDebugIn << "name:" << gInfo.name << endl;
    
    QDomElement e = n->ownerDocument().createElement("Group");
    n->appendChild(e);
        
    e.setAttribute("Name", gInfo.name);

    sDebugOut << endl;
}

Group::GroupChanges Group::changesFlags(const GroupInterrogator * ti)
{
    if( groupChanges.find(ti) != groupChanges.end() )
        return groupChanges[ti];    
    else
    {
        //In this case the local transferChanges map does not contain 
        //any information related to the given view. So we add it.
        groupChanges[ti]=0xFFFFFFFF;
        return 0xFFFFFFFF;
    }
}

void Group::resetChangesFlags(const GroupInterrogator * ti)
{
    groupChanges[ti]=0;
}

void Group::about() const
{
    kdDebug() << "  Group name= " << gInfo.name << endl; ;
}

bool Group::findTransfer(Transfer * t) const
{
    return (transfersMap.find(t) != transfersMap.end());
}

void Group::addTransfer(Transfer * t)
{
    sDebugIn << "processedSize" << gInfo.processedSize << endl;    

    TransferInfoCache ic = updatedInfoCache(t);
    transfersMap[t] = ic;
    
    gInfo.totalSize+=ic.totalSize;
    gInfo.processedSize+=ic.processedSize;
    gInfo.speed+=ic.speed;    
    updatePercent();

    setGroupChange(Gc_TotalSize);
    setGroupChange(Gc_ProcessedSize);
    setGroupChange(Gc_Percent);
    emit groupChanged(this);

    sDebugOut << endl;    
}

void Group::delTransfer(Transfer * t)
{
    sDebugIn << endl;
    
    TransferInfoCache ic = transfersMap[t];
    
    sDebug << "delTransfer" << gInfo.totalSize << ic.totalSize << endl;
    
    gInfo.totalSize-=ic.totalSize;
    gInfo.processedSize-=ic.processedSize;
    gInfo.speed-=ic.speed;    
    updatePercent();

    transfersMap.erase(t);

    setGroupChange(Gc_TotalSize);
    setGroupChange(Gc_ProcessedSize);
    setGroupChange(Gc_Percent);
    emit groupChanged(this);

    sDebugOut << endl;
}

void Group::changedTransfer(Transfer * t, Transfer::TransferChanges tc)
{
    Transfer::Info ti = t->info();

    switch(tc)
    {
        case Transfer::Tc_ProcessedSize:
            kdDebug() << "Group::changedTransfer" << endl;
            gInfo.processedSize-=transfersMap[t].processedSize;
            gInfo.processedSize+=ti.processedSize;
            updatePercent();
            transfersMap[t] = updatedInfoCache(t);

            setGroupChange(Gc_ProcessedSize);
            setGroupChange(Gc_Percent);
            emit groupChanged(this);
            break;

        case Transfer::Tc_TotalSize:
            kdDebug() << "Group::changedTransfer -> TotalSize" << endl;
            gInfo.totalSize-=transfersMap[t].totalSize;
            gInfo.totalSize+=ti.totalSize;
            updatePercent();
            transfersMap[t] = updatedInfoCache(t);

            setGroupChange(Gc_TotalSize);
            setGroupChange(Gc_Percent);
            emit groupChanged(this);
            break;
    }
}

void Group::setGroupChange(GroupChange p)
{
    QMap<const GroupInterrogator *, GroupChanges>::Iterator it = groupChanges.begin();    
    
    QMap<const GroupInterrogator *, GroupChanges>::Iterator itEnd = groupChanges.end();    
    
    for( ; it!=itEnd; ++it )
        *it |= p;
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

GroupList::GroupList()
    : QObject(),
      m_modifiedGroups(0)
{

}

GroupList::GroupList(const GroupList& list)
    : QObject(),
      m_modifiedGroups(0),
      QValueList<Group *>()
{
    addGroup(list);
}

GroupList::GroupList(const Group& group)
    : QObject(),
      m_modifiedGroups(0)
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

void GroupList::addGroup(const Group& group)
{
    sDebugIn << endl;
    
    Group * newGroup = new Group(group);

    connect(newGroup, SIGNAL(groupChanged(Group *)),
            this,     SLOT(slotGroupChanged(Group *)));

    groupsMap[group.info().name] = newGroup;
    push_front(newGroup);
    
    sDebugOut << endl;
}

void GroupList::addGroup(const GroupList& list)
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

void GroupList::delGroup(const Group& group)
{
    QString groupName = group.info().name;

    groupsMap.erase(groupName);
    if (Group * g = getGroup(groupName))
        remove(g);
}

void GroupList::delGroup(const GroupList& list)
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
    
    if((g = getGroup(groupName)))
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
        QString gName = (*it)->info().group;
        if(groupsMap.contains(gName))
            groupsMap[gName]->addTransfer(*it);
    }
}

void GroupList::slotRemovedTransfers(const TransferList& list)
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();

    for(; it!=endList; ++it)
    {
        QString gName = (*it)->info().group;
        if(groupsMap.contains(gName))
            groupsMap[gName]->delTransfer(*it);
    }
}

void GroupList::slotChangedTransfers(const TransferList& list)
{
    TransferList::constIterator it = list.begin();
    TransferList::constIterator endList = list.end();

//     GroupList modifiedGroups;

    //kdDebug() << "GroupList::slotChangedTransfers" << endl;

    for(; it!=endList; ++it)
    {
        Transfer::TransferChanges tc = (*it)->changesFlags(this);
        QString gName = (*it)->info().group;

        /**
         * the transfer group has changed.
         * Note that we must check for the group change before checking for all
         * the other possible transfer changes.
         */
        if( tc & Transfer::Tc_Group )
        {
            //First let's find the group the transfer previously belonged to
            QMap<QString, Group*>::iterator gIt = groupsMap.begin();
            QMap<QString, Group*>::iterator gItEnd = groupsMap.end();

            for(; gIt!=gItEnd; ++gIt)
            {
                //Delete the transfer from its old group
                if((*gIt)->findTransfer(*it))
                {
                    (*gIt)->delTransfer(*it);
                }
            }

            //Add the transfer to the new group
            QMap<QString, Group*>::iterator newGroup = groupsMap.find(gName);
            if( newGroup!=groupsMap.end() )
            {
                (*newGroup)->addTransfer(*it);
            }
        }

        /**
         * the transfer progress has changed
         */
        if( (tc & Transfer::Tc_ProcessedSize) || (tc & Transfer::Tc_TotalSize) )
        {
            QMap<QString, Group*>::iterator gIter = groupsMap.find(gName);

            if( gIter != groupsMap.end() )
            {
                //The group has been found (it belongs to the GroupList)
                Group * g = *gIter;
                //Notify the group that one of its transfers has changed
                g->changedTransfer(*it, tc);
            }
        }

        (*it)->resetChangesFlags(this);
    }
}

void GroupList::slotGroupChanged( Group * g )
{
    startTimer( 100 );

    if(!m_modifiedGroups)
        m_modifiedGroups = new GroupList();
    m_modifiedGroups->push_back(g);
}

void GroupList::timerEvent( QTimerEvent *e )
{
    killTimers();
    emit groupsChanged(*m_modifiedGroups);
    m_modifiedGroups->clear();
}

#include "group.moc"
