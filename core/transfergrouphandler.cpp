/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfergrouphandler.h"

#include "core/transferhandler.h"
#include "core/transfertreemodel.h"
#include "core/transfer.h"
#include "core/observer.h"
#include "core/kget.h"

#include <kdebug.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kicon.h>

TransferGroupHandler::TransferGroupHandler(TransferGroup * group, Scheduler * scheduler)
    : m_group(group),
      m_scheduler(scheduler),
      m_qobject(0)
{
    m_observers.push_back(0);
    m_changesFlags[0]=0xFFFFFFFF;
}

TransferGroupHandler::~TransferGroupHandler()
{
}

void TransferGroupHandler::addObserver(TransferGroupObserver * observer)
{
    kDebug(5001) << "TransferGroupHandler::addObserver";
    m_observers.push_back(observer);
    m_changesFlags[observer]=0xFFFFFFFF;
    kDebug(5001) << "   Now we have " << m_observers.size() << " observers";
}

void TransferGroupHandler::delObserver(TransferGroupObserver * observer)
{
    m_observers.removeAll(observer);
    m_changesFlags.remove(observer);
}

void TransferGroupHandler::start()
{
    kDebug(5001) << "TransferGroupHandler::start()";
    m_group->setStatus( JobQueue::Running );
}

void TransferGroupHandler::stop()
{
    kDebug(5001) << "TransferGroupHandler::stop()";
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
//     kDebug(5001) << "TransferGroupHandler::operator[" << i << "]";

    return (*m_group)[i]->handler();
}

void TransferGroupHandler::setName(const QString &name)
{
    m_group->setName(name);
    postGroupChangedEvent();
}

QVariant TransferGroupHandler::data(int column)
{
//     kDebug(5001) << "TransferGroupHandler::data(" << column << ")";

    switch(column)
    {
        case 0:
            if (!m_group->supportsSpeedLimits() && (m_group->visibleDownloadLimit() != 0 || m_group->visibleUploadLimit() != 0))
                return name() + " - Does not supports SpeedLimits";//FIXME: Do a better text here
            else
                return name();
        case 2:
            if(m_group->size())
                return i18np("1 Item", "%1 Items", m_group->size());
            else
                return QString();
/*            if (totalSize() != 0)
                return KIO::convertSize(totalSize());
            else
                return i18nc("not available", "n/a");*/
        case 3:
//             return QString::number(percent())+'%'; // display progressbar instead
            return QVariant();
        case 4:
            if (downloadSpeed() == 0)
            {
                return QString();
            }
            else
                return i18n("%1/s", KIO::convertSize(downloadSpeed()));
        default:
            return QVariant();
    }
}

TransferGroup::ChangesFlags TransferGroupHandler::changesFlags(TransferGroupObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        return m_changesFlags[observer];
    else
    {
        kDebug(5001) << " TransferGroupHandler::changesFlags() doesn't see you as an observer! ";

        return 0xFFFFFFFF;
    }
}

void TransferGroupHandler::resetChangesFlags(TransferGroupObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        m_changesFlags[observer] = 0;
    else
        kDebug(5001) << " TransferGroupHandler::resetchangesFlags() doesn't see you as an observer! ";
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

const QList<QAction *> & TransferGroupHandler::actions()
{
    createActions();

    return m_actions;
}

KMenu * TransferGroupHandler::popupMenu()
{
    KMenu * popup = new KMenu( 0 );
//     popup->addTitle( i18nc( "%1 is the name of the group", "%1 Group", name() ) );

    createActions();

    popup->addAction( KGet::actionCollection()->action("transfer_group_start") );
    popup->addAction( KGet::actionCollection()->action("transfer_group_stop") );
    popup->addSeparator();
    popup->addAction(KGet::actionCollection()->action("transfer_group_settings"));
    popup->addSeparator();
    if(m_group->name() != i18n("My Downloads")) {
        popup->addAction( KGet::actionCollection()->action("delete_groups") );
        popup->addAction( KGet::actionCollection()->action("rename_groups") );
    }
    popup->addAction( KGet::actionCollection()->action("seticon_groups") );
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
        if(*it)
            (*it)->groupChangedEvent(this);
    }

    m_group->model()->postDataChangedEvent(this);
}

void TransferGroupHandler::postAddedTransferEvent(Transfer * transfer, Transfer * after)
{
    kDebug(5001) << "TransferGroupHandler::postAddedTransferEvent";
    kDebug(5001) << "   number of observers = " << m_observers.size();

    //Here we have to copy the list and iterate on the copy itself, because
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferGroupObserver *> observers = m_observers;

    QList<TransferGroupObserver *>::iterator it = observers.begin();
    QList<TransferGroupObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        if(*it)
        {
            if(after)
                (*it)->addedTransferEvent(transfer->handler(), after->handler());
            else
                (*it)->addedTransferEvent(transfer->handler(), 0);
        }
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
        if(*it)
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
        if(*it)
        {
            if(after)
                (*it)->movedTransferEvent(transfer->handler(), after->handler());
            else
                (*it)->movedTransferEvent(transfer->handler(), 0);
        }
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
        if(*it)
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

    QAction *startAction = KGet::actionCollection()->addAction("transfer_group_start");
    startAction->setText(i18nc("start transfergroup downloads", "Start"));
    startAction->setIcon(KIcon("media-playback-start"));
    QObject::connect(startAction, SIGNAL(triggered()), qObject(), SLOT(slotStart()));
    m_actions.append(startAction);

    QAction *stopAction = KGet::actionCollection()->addAction("transfer_group_stop");
    stopAction->setText(i18nc("stop transfergroup downloads", "Stop"));
    stopAction->setIcon(KIcon("media-playback-pause"));
    QObject::connect(stopAction, SIGNAL(triggered()), qObject(), SLOT(slotStop()));
    m_actions.append(stopAction);

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
