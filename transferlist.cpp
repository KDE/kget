/***************************************************************************
*                                transferlist.cpp
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

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>

#include "transfer.h"
#include "transferlist.h"
#include "scheduler.h"

TransferList::TransferList(Scheduler * _scheduler)
    : QValueList<Transfer *>(),
      scheduler(_scheduler)
{

}


TransferList::~TransferList()
{

}


Transfer *TransferList::addTransfer(const KURL & _source, const KURL & _dest, bool toBegin)
{
    Transfer *newItem = new Transfer(scheduler, _source, _dest, jobid);
    addTransfer(newItem, toBegin);
        
    return newItem;
}

void TransferList::addTransfer(Transfer * transfer, bool toBegin)
{
    sDebugIn << endl;

    jobid++;
    if(toBegin)
        push_front(transfer);
    else
        push_back(transfer);
    
    qHeapSort(*this);

    sDebugOut << endl;
}

void TransferList::addTransfers(TransferList & transfers, bool toBegin)
{
    sDebugIn << endl;

    iterator it;
    iterator endList = end();
    
    for(it = begin(); it != endList; ++it)
        {
        jobid++;
        if(toBegin)
            push_front(*it);
        else
            push_back(*it);
    }
    qHeapSort(*this);

    sDebugOut << endl;
}

void TransferList::removeTransfer(Transfer * transfer)
{
    sDebugIn << endl;

    remove(transfer);    
    
    sDebugOut << endl;
}

void TransferList::removeTransfers(TransferList & transfers)
{
    sDebugIn << endl;

    TransferList::iterator it;
    TransferList::iterator endList = transfers.end();
    
    for(it = transfers.begin(); it != endList; ++it)
        {
        remove(*it);
    }
    sDebugOut << endl;
}

void TransferList::moveToBegin(Transfer * item, int priority)
{
    sDebugIn << endl;

    if(priority != -1)
        item->setPriority(priority);
    
    //Now I remove the current Item and I reinsert it with addTransfer(..,true)
    remove(item);
    addTransfer(item, true);
    
    sDebugOut << endl;
}

void TransferList::moveToBegin(TransferList & transfers, int priority)
{
    sDebugIn << endl;

    iterator it;
    iterator endList = transfers.end();
    
    if(priority != -1)
        {
        for(it = transfers.begin(); it != endList; ++it)
            {
            (*it)->setPriority(priority);
            removeTransfer(*it);
        }
    }    
    addTransfers(transfers, true);
    
    sDebugOut << endl;
}

void TransferList::moveToEnd(Transfer * item, int priority)
{
    sDebugIn << endl;

    if(priority != -1)
        item->setPriority(priority);
    
    //Now I remove the current Item and I reinsert it with addTransfer(..,true)
    remove(item);
    addTransfer(item, false);

    sDebugOut << endl;
}

void TransferList::moveToEnd(TransferList & transfers, int priority)
{
    sDebugIn << endl;
    
    iterator it;
    iterator endList = transfers.end();
    
    if(priority != -1)
        {
        for(it = transfers.begin(); it != endList; ++it)
            {
            (*it)->setPriority(priority);
            removeTransfer(*it);
        }
    }    
    addTransfers(transfers, false);

    sDebugOut << endl;
}


/*
void TransferList::moveToEnd(Transfer * item)
{
    //        ASSERT(item);

    Transfer *oldlast=static_cast<Transfer*>(lastItem());
    item->moveItem(oldlast);
}
*/


Transfer * TransferList::find(const KURL& _src)
{
    iterator it;
    iterator endList = end();
    
    for(it = begin(); it != endList; ++it)
        {
        if((*it)->getSrc() == _src)
            return *it;
    }
    return 0;
}


void TransferList::readTransfers(const KURL& file)
{
    sDebugIn << endl;
    
    QString tmpFile;

    kdDebug(DKGET) << "AAA" << endl;      
    
    if (KIO::NetAccess::download(file, tmpFile)) {
        KSimpleConfig config(tmpFile);

        kdDebug(DKGET) << "BBB" << endl;
        
        config.setGroup("Common");
        int num = config.readNumEntry("Count", 0);

        Transfer *item;
        KURL src, dest;

        for ( int i = 0; i < num; i++ )
        {
            QString str;

            str.sprintf("Item%d", i);
            config.setGroup(str);

            src  = KURL::fromPathOrURL( config.readPathEntry("Source") );
            dest = KURL::fromPathOrURL( config.readPathEntry("Dest") );
            kdDebug(DKGET) << "CCC" << endl;
           
            item = addTransfer( src, dest); // don't show!

            if (!item->read(&config, i))
                delete item;
            /*else
            {
                // configuration read, now we know the status to determine
                // whether to show or not
                //item->maybeShow();
            }
            */
        }
    }
    sDebugOut << endl;
}

void TransferList::writeTransfers(const QString& file)
{
    sDebug << ">>>>Entering with file =" << file << endl;

    KSimpleConfig config(file);
    int num = size();

    config.setGroup("Common");
    config.writeEntry("Count", num);

    iterator it = begin();

    for (int id = 0; *it; ++it, ++id)
        (*it)->write(&config, id);
    config.sync();

    sDebug << "<<<<Leaving" << endl;
}

void TransferList::about()
{
    iterator it;
    for(it = begin(); it != end(); ++it)
        {
        (*it)->about();
    }
}
