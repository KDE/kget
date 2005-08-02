/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <klocale.h>

#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "core/transfer.h"
#include "core/observer.h"
#include "core/model.h"
//Added by qt3to4:
#include <QList>

TransferGroupHandler::TransferGroupHandler(TransferGroup * group, Scheduler * scheduler)
    : m_group(group),
      m_scheduler(scheduler),
      m_qobject(0)
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

void TransferGroupHandler::start()
{
    kdDebug() << "TransferGroupHandler::start()" << endl;
    m_group->setStatus( JobQueue::Running );
}

void TransferGroupHandler::stop()
{
    kdDebug() << "TransferGroupHandler::stop()" << endl;
    m_group->setStatus( JobQueue::Stopped );
}

void TransferGroupHandler::move(QList<TransferHandler *> transfers, TransferHandler * after)
{
    //Check that the given transfer (after) belongs to this group
    if( after && (after->group() != this) )
        return;

    QList<TransferHandler *>::iterator it = transfers.begin();
    QList<TransferHandler *>::iterator itEnd = transfers.end();

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

const QList<TransferHandler *> TransferGroupHandler::transfers()
{
    QList<TransferHandler *> transfers;

    TransferGroup::iterator it = m_group->begin();
    TransferGroup::iterator itEnd = m_group->end();

    for( ; it!=itEnd ; ++it )
    {
        transfers.append((static_cast<Transfer *>(*it))->handler());
    }
    return transfers;
}

const QList<KAction *> & TransferGroupHandler::actions()
{
    createActions();

    return m_actions;
}

KPopupMenu * TransferGroupHandler::popupMenu()
{
    KPopupMenu * popup = new KPopupMenu( 0 );
    popup->insertTitle( name() + " " + i18n("group") );

    createActions();

    Model::actionCollection()->action("transfer_group_start")->plug( popup );
    Model::actionCollection()->action("transfer_group_stop")->plug( popup );

    return popup;
}

QObjectInterface * TransferGroupHandler::qObject()
{
    if( !m_qobject )
        m_qobject = new QObjectInterface(this);

    return m_qobject;
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
    //Here we have to copy the list and iterate on the copy itself, becouse
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->groupChangedEvent(this);
    }
}

void TransferGroupHandler::postAddedTransferEvent(Transfer * transfer, Transfer * after)
{
    kdDebug() << "TransferGroupHandler::postAddedTransferEvent" << endl;
    kdDebug() << "   number of observers = " << m_observers.size() << endl;

    //Here we have to copy the list and iterate on the copy itself, becouse
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

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
    //Here we have to copy the list and iterate on the copy itself, becouse
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->removedTransferEvent(transfer->handler());
    }
}

void TransferGroupHandler::postMovedTransferEvent(Transfer * transfer, Transfer * after)
{
    //Here we have to copy the list and iterate on the copy itself, becouse
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

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
    //Here we have to copy the list and iterate on the copy itself, becouse
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->deleteEvent(this);
    }
}

void TransferGroupHandler::createActions()
{
    if( !m_actions.empty() )
        return;

    //Calling this function we make sure the QObjectInterface object 
    //has been created (if not it will create it)
    qObject();

    m_actions.append( new KAction( i18n("Start"), "tool_resume",
                                   qObject(), SLOT( slotStart() ),
                                   Model::actionCollection(),
                                   "transfer_group_start") );

    m_actions.append( new KAction( i18n("Stop"), "tool_pause",
                                   qObject(), SLOT( slotStop() ),
                                   Model::actionCollection(),
                                   "transfer_group_stop") );
}



QObjectInterface::QObjectInterface(TransferGroupHandler * handler)
    : m_handler(handler)
{

}

void QObjectInterface::slotStart()
{
    m_handler->start();
}

void QObjectInterface::slotStop()
{
    m_handler->stop();
}

#include "transfergrouphandler.moc"
