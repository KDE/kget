/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERLIST_H
#define _TRANSFERLIST_H

#include <qvaluelist.h>

#include <kurl.h>

class Scheduler;
class GroupList;


class TransferList : public QValueList<Transfer *>
{

public:

    typedef QValueListIterator<Transfer *> iterator;
    typedef QValueListConstIterator<Transfer *> constIterator;

    TransferList(Transfer * transfer = 0);
    virtual ~TransferList();

    /**
     * Functions used to add transfer items to the list.
     * if toBegin = true the item is added at the begin of the group of items
     * having the same priority.
     * if toBegin = false the item is added at the end of the group of items
     * having the same priority
     */
    void addTransfer(Transfer * transfer, bool toBegin = false);
    void addTransfers(const TransferList &, bool toBegin = false);

    /**
     * Functions used to remove items from the list
     */
    void removeTransfer(Transfer * transfer);
    void removeTransfers(const TransferList & transfers);

    bool contains(Transfer * transfer) const;
    Transfer * find(const KURL& _src);

    void readTransfers(const QString& filename, Scheduler * scheduler, GroupList * g);
    void writeTransfers(const QString& filename, Scheduler * scheduler);

    /**
     * Debug function
     */
    void about();
    
    friend class Transfer;

protected:

    void readConfig();
    void writeConfig();
};


#endif                          // _TRANSFERLIST_H
