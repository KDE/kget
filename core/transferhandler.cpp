/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include <QVariant>

#include "core/job.h"
#include "core/jobqueue.h"
#include "core/transferhandler.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"
#include "core/plugin/transferfactory.h"
#include "core/observer.h"

TransferHandler::TransferHandler(Transfer * transfer, Scheduler * scheduler)
    : m_transfer(transfer), m_scheduler(scheduler)
{
    int column = m_transfer->group()->indexOf(m_transfer);

    m_indexes.append(new QPersistentModelIndex(m_transfer->model()->createIndex(column, 0 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_transfer->model()->createIndex(column, 1 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_transfer->model()->createIndex(column, 2 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_transfer->model()->createIndex(column, 3 ,this)));
    m_indexes.append(new QPersistentModelIndex(m_transfer->model()->createIndex(column, 4 ,this)));
}

TransferHandler::~TransferHandler()
{
}

void TransferHandler::addObserver(TransferObserver * observer)
{
    m_observers.push_back(observer);
    m_changesFlags[observer]=0xFFFFFFFF;
    kDebug() << "TransferHandler: OBSERVERS++ = " << m_observers.size() << endl;
}

void TransferHandler::delObserver(TransferObserver * observer)
{
    m_observers.removeAll(observer);
    m_changesFlags.remove(observer);
    kDebug() << "TransferHandler: OBSERVERS-- = " << m_observers.size() << endl;
}

void TransferHandler::start()
{
    if(m_transfer->group()->status() == JobQueue::Running)
    {
	m_transfer->setPolicy(Job::None);
        m_transfer->group()->move(m_transfer, 0);
    }
    else
    {
        m_transfer->setPolicy(Job::Start);
    }
}

void TransferHandler::stop()
{
    if(m_transfer->group()->status() == JobQueue::Stopped)
    {
        m_transfer->setPolicy(Job::None);
    }
    else
    {
	m_transfer->setPolicy(Job::Stop);
    }
}

int TransferHandler::elapsedTime() const
{
    return m_transfer->elapsedTime();
}

int TransferHandler::remainingTime() const
{
    return m_transfer->remainingTime();
}

bool TransferHandler::isResumable() const
{
    return m_transfer->isResumable();
}

unsigned long TransferHandler::totalSize() const
{
    return m_transfer->totalSize();
}

unsigned long TransferHandler::processedSize() const
{
    return m_transfer->processedSize();
}

int TransferHandler::percent() const
{
    return m_transfer->percent();
}

int TransferHandler::speed() const
{
    return m_transfer->speed();
}

QVariant TransferHandler::data(int column)
{
    kDebug() << "TransferHandler::data(" << column << ")" << endl;

    if(column==0)
        return QVariant(source().fileName());
    else if(column==1)
        return QVariant(statusText());
    else if(column==2)
        return QVariant((qulonglong) totalSize());
    else if(column==3)
        return QVariant(percent());
    else if(column==4)
        return QVariant(speed());

    return QVariant();
}

QModelIndex TransferHandler::index(int column)
{
    kDebug() << "TransferHandler::index(" << column << ")" << endl;

    return QModelIndex(*m_indexes[column]);
}

KMenu * TransferHandler::popupMenu(QList<TransferHandler *> transfers)
{
    return m_transfer->factory()->createPopupMenu(transfers);
}

void TransferHandler::setSelected( bool select )
{
    if( (select && !isSelected()) || (!select && isSelected()) )
    {
        m_transfer->m_isSelected = select;
        setTransferChange( Transfer::Tc_Selection, true );
    }
}

bool TransferHandler::isSelected() const
{
    return m_transfer->m_isSelected;
}

Transfer::ChangesFlags TransferHandler::changesFlags(TransferObserver * observer) const
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        return m_changesFlags[observer];
    else
    {
        kDebug() << " TransferHandler::changesFlags() doesn't see you as an observer! " << endl;

        return 0xFFFFFFFF;
    }
}

void TransferHandler::resetChangesFlags(TransferObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        m_changesFlags[observer] = 0;
    else
        kDebug() << " TransferHandler::resetchangesFlags() doesn't see you as an observer! " << endl;
}

void TransferHandler::setTransferChange(ChangesFlags change, bool postEvent)
{
    QMap<TransferObserver *, ChangesFlags>::Iterator it = m_changesFlags.begin();
    QMap<TransferObserver *, ChangesFlags>::Iterator itEnd = m_changesFlags.end();

    for( ; it!=itEnd; ++it )
        *it |= change;

    if(postEvent)
        postTransferChangedEvent();
}

void TransferHandler::postTransferChangedEvent()
{
    kDebug() << "TransferHandler::postTransferChangedEvent() ENTERING" << endl;
    
    // Here we have to copy the list and iterate on the copy itself, because
    // a view can remove itself as a view while we are iterating over the
    // observers list and this leads to crashes.
    QList<TransferObserver *> observers = m_observers;

    QList<TransferObserver *>::iterator it = observers.begin();
    QList<TransferObserver *>::iterator itEnd = observers.end();

    // Notify the observers
    for(; it!=itEnd; ++it)
    {
        (*it)->transferChangedEvent(this);
    }

    // Notify the group
    m_transfer->group()->transferChangedEvent(m_transfer);

    // Notify the TransferTreeModel
    m_transfer->model()->dataChanged(index(0), index(5));

    //kDebug() << "TransferHandler::postTransferChangedEvent() LEAVING" << endl;
}

void TransferHandler::postDeleteEvent()
{
    kDebug() << "TransferHandler::postDeleteEvent() ENTERING" << endl;

    //Here we have to copy the list and iterate on the copy itself, because
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferObserver *> observers = m_observers;

    QList<TransferObserver *>::iterator it = observers.begin();
    QList<TransferObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->deleteEvent(this);
    }
    kDebug() << "TransferHandler::postDeleteEvent() LEAVING" << endl;
}
