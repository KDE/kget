/***************************************************************************
*                                transferlist.h
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/

#ifndef _TRANSFERLIST_H
#define _TRANSFERLIST_H

#include <qvaluelist.h>

#include <kurl.h>

class Transfer;
class Scheduler;



class TransferList : public QValueList<Transfer *>
{

public:

    typedef QValueListIterator<Transfer *> iterator;
    
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
   
    void addTransfers(TransferList &, bool toBegin = false);

    /**
     * Functions used to remove items from the list
     */
    void removeTransfer(Transfer * transfer);
    void removeTransfers(TransferList & transfers);

        
    /**
     * Functions used to move transfer items inside the list.
     * if priority = -1 the transfer becomes the first item having its
     * priority.
     * if priority != -1 the transfer becomes the first item having
     * priority "priority".
     */
    void moveToBegin(Transfer * item, int priority = -1);
    void moveToBegin(TransferList &, int priority = -1);
    
    /**
     * Functions used to move transfer items inside the list.
     * if priority = -1 the transfer becomes the last item having its
     * priority.
     * if priority != -1 the transfer becomes the last item having
     * priority "priority".
     */
    void moveToEnd(Transfer * item, int priority = -1);
    void moveToEnd(TransferList &, int priority = -1);

    uint getPhasesNum()const
    {
        return phasesNum;
    }
    
    bool contains(Transfer * transfer);
    Transfer * find(const KURL& _src);

    void readTransfers(const KURL& file, Scheduler * scheduler);
    void writeTransfers(const QString& file);

    /**
     * Debug function
     */
    void about();
    
    friend class Transfer;

protected:

    void readConfig();
    void writeConfig();

    // ListView IDs
    int lv_pixmap, lv_filename, lv_resume, lv_count, lv_progress;
    int lv_total, lv_speed, lv_remaining, lv_url;

    uint phasesNum;
    uint jobid;

};


#endif                          // _TRANSFERLIST_H
