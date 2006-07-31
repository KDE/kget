/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>

#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "core/transfer.h"
#include "core/observer.h"
#include "core/kget.h"

TransferGroupHandler::TransferGroupHandler(TransferGroup * group, Scheduler * scheduler)
    : m_group(group),
      m_scheduler(scheduler),
      m_qobject(0)
{
    int numChilds = m_group->model()->rowCount(QModelIndex());

    m_indexes.append(new QPersistentModelIndex(m_group->model()->createIndex(numChilds, 0 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_group->model()->createIndex(numChilds, 1 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_group->model()->createIndex(numChilds, 2 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_group->model()->createIndex(numChilds, 3 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_group->model()->createIndex(numChilds, 4 ,this)));
}

TransferGroupHandler::~TransferGroupHandler()
{
    
}

void TransferGroupHandler::addObserver(TransferGroupObserver * observer)
{
    kDebug() << "TransferGroupHandler::addObserver" << endl;
    m_observers.push_back(observer);
    m_changesFlags[observer]=0xFFFFFFFF;
    kDebug() << "   Now we have " << m_observers.size() << " observers" << endl;
}

void TransferGroupHandler::delObserver(TransferGroupObserver * observer)
{
    m_observers.removeAll(observer);
    m_changesFlags.remove(observer);
}

void TransferGroupHandler::start()
{
    kDebug() << "TransferGroupHandler::start()" << endl;
    m_group->setStatus( JobQueue::Running );
}

void TransferGroupHandler::stop()
{
    kDebug() << "TransferGroupHandler::stop()" << endl;
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

TransferHandler * TransferGroupHandler::operator[] (int i)
{
//     kDebug() << "TransferGroupHandler::operator[" << i << "]" << endl;

    return (*m_group)[i]->handler();
}

QVariant TransferGroupHandler::data(int column)
{
    kDebug() << "TransferGroupHandler::data(" << column << ")" << endl;

    if(column==0)
    {
//         if(name()=="" || name().isEmpty() || name().isNull())
//             return QVariant("empty name");
        return QVariant("_"+name()+"_");
    }
    else if(column==2)
        return QVariant(totalSize());
    else if(column==3)
        return QVariant(percent());
    else if(column==4)
        return QVariant(speed());

    return QVariant();
}

QModelIndex TransferGroupHandler::index(int column)
{
    kDebug() << "TransferGroupHandler::index(" << column << ")" << endl;

    if(column < columnCount())
        return QModelIndex(*m_indexes[column]);
    else
        return QModelIndex();
}

TransferGroup::ChangesFlags TransferGroupHandler::changesFlags(TransferGroupObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        return m_changesFlags[observer];
    else
    {
        kDebug() << " TransferGroupHandler::changesFlags() doesn't see you as an observer! " << endl;

        return 0xFFFFFFFF;
    }
}

void TransferGroupHandler::resetChangesFlags(TransferGroupObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        m_changesFlags[observer] = 0;
    else
        kDebug() << " TransferGroupHandler::resetchangesFlags() doesn't see you as an observer! " << endl;
}

int TransferGroupHandler::indexOf(TransferHandler * transfer)
{
    return m_group->indexOf(transfer->m_transfer);
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

KMenu * TransferGroupHandler::popupMenu()
{
    KMenu * popup = new KMenu( 0 );
    popup->addTitle( i18nc( "%1 is the name of the group", "%1 Group", name() ) );

    createActions();

    popup->addAction( KGet::actionCollection()->action("transfer_group_start") );
    popup->addAction( KGet::actionCollection()->action("transfer_group_stop") );

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
    //Here we have to copy the list and iterate on the copy itself, because
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->groupChangedEvent(this);
    }

    m_group->model()->dataChanged(index(0), index(4));
}

void TransferGroupHandler::postAddedTransferEvent(Transfer * transfer, Transfer * after)
{
    kDebug() << "TransferGroupHandler::postAddedTransferEvent" << endl;
    kDebug() << "   number of observers = " << m_observers.size() << endl;

    //Here we have to copy the list and iterate on the copy itself, because
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
    //Here we have to copy the list and iterate on the copy itself, because
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
    //Here we have to copy the list and iterate on the copy itself, because
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
    //Here we have to copy the list and iterate on the copy itself, because
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

    KAction * a = new KAction( KIcon("player_start"), i18n("Start"),
                               KGet::actionCollection(), "transfer_group_start" );
    QObject::connect( a, SIGNAL( triggered() ), qObject(), SLOT( slotStart() ) );
    m_actions.append( a );

    a = new KAction( KIcon("player_pause"), i18n("Stop"),
                     KGet::actionCollection(), "transfer_group_stop" );
    QObject::connect( a, SIGNAL( triggered() ), qObject(), SLOT( slotStop() ) );
    m_actions.append( a );
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
