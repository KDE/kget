/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "core/transfer.h"
#include "core/observer.h"

TransferGroupHandler::TransferGroupHandler(TransferGroup * group, Scheduler * scheduler)
    : m_group(group),
      m_scheduler(scheduler)
{
    
}

TransferGroupHandler::~TransferGroupHandler()
{
    
}

void TransferGroupHandler::addObserver(TransferGroupObserver * observer)
{
    kdDebug() << "TransferGroupHandler::addObserver" << endl;
    m_observers.push_back(observer);
    m_changesFlags[observer]=0xFFFFFFFF;
    kdDebug() << "   Now we have " << m_observers.size() << " observers" << endl;
}

void TransferGroupHandler::delObserver(TransferGroupObserver * observer)
{
    m_observers.remove(observer);
    m_changesFlags.remove(observer);
}

void TransferGroupHandler::move(QValueList<TransferHandler *> transfers, TransferHandler * after)
{
    //Check that the given transfer (after) belongs to this group
    if( after && (after->group() != this) )
        return;

    QValueList<TransferHandler *>::iterator it = transfers.begin();
    QValueList<TransferHandler *>::iterator itEnd = transfers.end();

    for( ; it!=itEnd ; ++it )
    {
        //Move the transfers in the JobQueue
        if(after)
            m_group->move( (*it)->m_transfer, after->m_transfer );
        else
            m_group->move( (*it)->m_transfer, 0 );

        after = *it;
    }
}

TransferGroup::ChangesFlags TransferGroupHandler::changesFlags(TransferGroupObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        return m_changesFlags[observer];
    else
    {
        kdDebug() << " TransferGroupHandler::changesFlags() doesn't see you as an observer! " << endl;

        return 0xFFFFFFFF;
    }
}

void TransferGroupHandler::resetChangesFlags(TransferGroupObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        m_changesFlags[observer] = 0;
    else
        kdDebug() << " TransferGroupHandler::resetchangesFlags() doesn't see you as an observer! " << endl;
}

void TransferGroupHandler::setGroupChange(ChangesFlags change, bool postEvent)
{
    QMap<TransferGroupObserver *, ChangesFlags>::Iterator it = m_changesFlags.begin();
    QMap<TransferGroupObserver *, ChangesFlags>::Iterator itEnd = m_changesFlags.end();

    for( ; it!=itEnd; ++it )
        *it |= change;

    if(postEvent)
        postGroupChangedEvent();
}

void TransferGroupHandler::postGroupChangedEvent()
{
    QValueList<TransferGroupObserver *>::iterator it = m_observers.begin();
    QValueList<TransferGroupObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->groupChangedEvent(this);
    }
}

void TransferGroupHandler::postAddedTransferEvent(Transfer * transfer, Transfer * after)
{
    kdDebug() << "TransferGroupHandler::postAddedTransferEvent" << endl;
    kdDebug() << "   number of observers = " << m_observers.size() << endl;

    QValueList<TransferGroupObserver *>::iterator it = m_observers.begin();
    QValueList<TransferGroupObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        if(after)
            (*it)->addedTransferEvent(transfer->handler(), after->handler());
        else
            (*it)->addedTransferEvent(transfer->handler(), 0);
    }
}

void TransferGroupHandler::postRemovedTransferEvent(Transfer * transfer)
{
    QValueList<TransferGroupObserver *>::iterator it = m_observers.begin();
    QValueList<TransferGroupObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->removedTransferEvent(transfer->handler());
    }
}

void TransferGroupHandler::postMovedTransferEvent(Transfer * transfer, Transfer * after)
{
    QValueList<TransferGroupObserver *>::iterator it = m_observers.begin();
    QValueList<TransferGroupObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        if(after)
            (*it)->movedTransferEvent(transfer->handler(), after->handler());
        else
            (*it)->movedTransferEvent(transfer->handler(), 0);
    }
}

void TransferGroupHandler::postDeleteEvent()
{
    QValueList<TransferGroupObserver *>::iterator it = m_observers.begin();
    QValueList<TransferGroupObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->deleteEvent(this);
    }
}
