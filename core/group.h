/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _GROUP_H
#define _GROUP_H

#include <qobject.h>
#include <qvaluelist.h>

#include "interrogator.h"
#include "transfer.h"

class QString;
class QDomDocument;
class QDomNode;

class TransferList;

/**
 * class Group: 
 *   This class abstracts the concept of transfer group by means of which
 *   the user can sort his transfers into categories.
 *   Moreover this class calculates informations such as:
 *   -> the size obtained by the sum of all the transfer's size
 *   -> the size obtained by the sum of all the transfer's processed size
 *   -> the global progress percentage within the group
 *   -> the global speed within the group
 *   This is useful becouse saves each view from calculating this data and
 *   allows each one to get this info simply calling the getInfo() method.
 */
class Group : public QObject
{
Q_OBJECT

friend class GroupList;

public:
    typedef int GroupChanges;

    enum GroupChange
    {
        Gc_None          = 0x00000000,
        Gc_TotalSize     = 0x00000001,
        Gc_ProcessedSize = 0x00000002,
        Gc_Percent       = 0x00000004
    };

    typedef struct Info
    {
        QString name;

        unsigned long totalSize;
        unsigned long processedSize;
        int percent;
        int speed;
    };

    Group(const Group& g);
    Group(const QString& name);
    Group(QDomNode * n);

    bool read(QDomNode * n);
    void write(QDomNode * n) const;

    const Info& info() const {return gInfo;}
    void setName(const QString& name);

    GroupChanges changesFlags(const GroupInterrogator *);
    void resetChangesFlags(const GroupInterrogator *);

    void about() const;

    signals:
    void groupChanged(Group *);

private:
    /**
     * This struct becomes necessary to handle, calculate and update 
     * correctly the informations coming from the transfers. For example,
     * if we only take an integer rapresenting the total processedSize
     * of the group we should, with every change in the transfers'  
     * processedSize property, iterate over all the transfers summing up 
     * all the processedSize int. Instead, in this way, we can perform 
     * this operation in O(1), but we use more memory.
     */
    typedef struct TransferInfoCache
    {
        unsigned long totalSize;
        unsigned long processedSize;
        int speed;
    };

    bool findTransfer(Transfer *) const;

    void addTransfer(Transfer *);
    void delTransfer(Transfer *);
    void changedTransfer(Transfer *, Transfer::TransferChanges);

    void setGroupChange(GroupChange);

    TransferInfoCache updatedInfoCache(Transfer *);
    void updatePercent();

    Info gInfo;
    QMap <Transfer*, TransferInfoCache> transfersMap;
    QMap <const GroupInterrogator *, GroupChanges> groupChanges;
};

class GroupList : public QObject,
                  public QValueList<Group *>,
                  public TransferInterrogator
{
Q_OBJECT

public:
    typedef QValueListIterator<Group *> iterator;
    typedef QValueListConstIterator<Group *> constIterator;

    GroupList();
    GroupList(const GroupList&);
    GroupList(const Group& group);

    Group * getGroup(const QString& groupName) const;

    void addGroup(const Group& group);
    void addGroup(const GroupList& list);

    void delGroup(const Group& group);
    void delGroup(const GroupList& list);

    void modifyGroup(const QString& groupName, Group group);

    bool read(QDomDocument * doc);
    void write(QDomDocument * doc) const;

    void about() const;

public slots:
    void slotAddedTransfers(const TransferList&);
    void slotRemovedTransfers(const TransferList&);
    void slotChangedTransfers(const TransferList&);

    void slotGroupChanged(Group *);

signals:
    void groupsChanged(const GroupList&);

private:
    void timerEvent( QTimerEvent *e );

    QMap<QString, Group*> groupsMap;

    //We use this list to cache the groupChanged signals. This prevents
    //us from emitting too many signals to the scheduler (and this means also
    //to each view!). All the signals emitted during a 100 ms interval (from
    //the first one) are grouped in only one signal.
    GroupList * m_modifiedGroups;
};

#endif
