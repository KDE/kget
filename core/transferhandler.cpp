/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transferhandler.h"

#include "core/job.h"
#include "core/jobqueue.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "core/plugin/transferfactory.h"
#include "core/observer.h"
#include "settings.h"
#include "mainwindow.h"
#include "kgetkjobadapter.h"

#include <kdebug.h>
#include <klocale.h>
#include <KPassivePopup>
#include <QAction>
#include <QVariant>

TransferHandler::TransferHandler(Transfer * parent, Scheduler * scheduler)
  : Handler(scheduler, parent),
    m_transfer(parent)
{
    static int dBusObjIdx = 0;
    dBusObjIdx++;

    m_dBusObjectPath = "/KGet/Transfers/" + QString::number(dBusObjIdx);
    
    addObserver(0);

    m_kjobAdapter = new KGetKJobAdapter(this, this);
    KGet::registerKJob(m_kjobAdapter);
}

TransferHandler::~TransferHandler()
{
//    qDeleteAll(m_observers);
}

void TransferHandler::addObserver(TransferObserver * observer)
{
    m_observers.push_back(observer);
    m_changesFlags[observer]=0xFFFFFFFF;
    kDebug(5001) << "TransferHandler: OBSERVERS++ = " << m_observers.size();
}

void TransferHandler::delObserver(TransferObserver * observer)
{
    m_observers.removeAll(observer);
    m_changesFlags.remove(observer);
    kDebug(5001) << "TransferHandler: OBSERVERS-- = " << m_observers.size();
}

void TransferHandler::start()
{
    if(m_transfer->group()->status() == JobQueue::Running)
    {
        m_transfer->setPolicy(Job::None);
        KGet::model()->moveTransfer(m_transfer, m_transfer->group());
    }
    else
    {
        m_transfer->setPolicy(Job::Start);
    }
}

void TransferHandler::stop()
{
    if (m_transfer->group()->status() == JobQueue::Stopped)
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

KIO::filesize_t TransferHandler::totalSize() const
{
    return m_transfer->totalSize();
}

KIO::filesize_t TransferHandler::downloadedSize() const
{
    return m_transfer->downloadedSize();
}

KIO::filesize_t TransferHandler::uploadedSize() const
{
    return m_transfer->uploadedSize();
}

int TransferHandler::percent() const
{
    return m_transfer->percent();
}

int TransferHandler::downloadSpeed() const
{
    return m_transfer->downloadSpeed();
}

int TransferHandler::uploadSpeed() const
{
    return m_transfer->uploadSpeed();
}

QVariant TransferHandler::data(int column)
{
//     kDebug(5001) << "TransferHandler::data(" << column << ")";

    switch(column)
    {
        case 0:
            return dest().fileName();
        case 1:
            return statusText();
        case 2:
            if (totalSize() != 0)
                return KIO::convertSize(totalSize());
            else
                return i18nc("not available", "n/a");
        case 3:
//             return QString::number(percent())+'%'; // display progressbar instead
            return QVariant();
        case 4:
            if (downloadSpeed()==0)
            {
                if (status() == Job::Running)
                    return i18n("Stalled");
                else
                    return QString();
            }
            else
                return i18n("%1/s", KIO::convertSize(downloadSpeed()));
        case 5:
            if (status() == Job::Running)
                return KIO::convertSeconds(remainingTime());
            else
                return QString();
        default:
            return QVariant();
    }
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
        kDebug(5001) << " TransferHandler::changesFlags() doesn't see you as an observer! ";

        return 0xFFFFFFFF;
    }
}

void TransferHandler::resetChangesFlags(TransferObserver * observer)
{
    if( m_changesFlags.find(observer) != m_changesFlags.end() )
        m_changesFlags[observer] = 0;
    else
        kDebug(5001) << " TransferHandler::resetChangesFlags() doesn't see you as an observer! ";
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
//     kDebug(5001) << "TransferHandler::postTransferChangedEvent() ENTERING";
    
    // Here we have to copy the list and iterate on the copy itself, because
    // a view can remove itself as a view while we are iterating over the
    // observers list and this leads to crashes.
    QList<TransferObserver *> observers = m_observers;

    QList<TransferObserver *>::iterator it = observers.begin();
    QList<TransferObserver *>::iterator itEnd = observers.end();

    // Notify the observers
    for(; it!=itEnd; ++it)
    {
        if((*it)/* && KGet::*/)
            (*it)->transferChangedEvent(this);
    }

    // Notify the group
    m_transfer->group()->transferChangedEvent(m_transfer);

    // Notify the TransferTreeModel
    m_transfer->model()->postDataChangedEvent(this);
    
    
    if (m_kjobAdapter)
    {
        m_kjobAdapter->slotUpdateDescription();
    
        if (m_transfer->status() == Job::Finished)
        {
            KGet::unregisterKJob(m_kjobAdapter);
            m_kjobAdapter = 0;
        }
    }

    //kDebug(5001) << "TransferHandler::postTransferChangedEvent() LEAVING";
}

void TransferHandler::postDeleteEvent()
{
    kDebug(5001) << "TransferHandler::postDeleteEvent() ENTERING";

    m_transfer->postDeleteEvent();//First inform the transfer itself

    //Here we have to copy the list and iterate on the copy itself, because
    //a view can remove itself as a view while we are iterating over the
    //observers list and this leads to crashes.
    QList<TransferObserver *> observers = m_observers;

    QList<TransferObserver *>::iterator it = observers.begin();
    QList<TransferObserver *>::iterator itEnd = observers.end();

    for(; it!=itEnd; ++it)
    {
        if(*it && *it != dynamic_cast<TransferObserver*>(m_transfer))
            (*it)->deleteEvent(this);
    }
    if (m_kjobAdapter)
        KGet::unregisterKJob(m_kjobAdapter);
    kDebug(5001) << "TransferHandler::postDeleteEvent() LEAVING";
}

QList<QAction*> TransferHandler::contextActions()
{
    QList<QAction*> actions;
    if (status() != Job::Finished) 
    {
        actions << KGet::actionCollection()->action("start_selected_download")
                << KGet::actionCollection()->action("stop_selected_download");
    }
    actions << KGet::actionCollection()->action("delete_selected_download")
            << KGet::actionCollection()->action("redownload_selected_download");

    return actions;
}

QList<QAction*> TransferHandler::factoryActions()
{
    QList<QAction*> actions;
    foreach(QAction *action, m_transfer->factory()->actions(this))
        actions.append(action);
    return actions;
}
