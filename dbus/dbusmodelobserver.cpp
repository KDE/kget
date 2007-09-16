/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#include "dbusmodelobserver.h"
#include "core/kget.h"
#include "core/transfergrouphandler.h"
#include "core/transferhandler.h"
#include "core/job.h"

#include <QObject>

DBusModelObserver::DBusModelObserver(QObject *parent) 
    : m_transfers(),
    m_transferGroupObserver(this),
    m_transferGroupHandlers()
{
    Q_UNUSED(parent)

    // Add the observer to kget to see all operations through the transferGroups
    KGet::addObserver(this);

    QObject::connect(&m_transferGroupObserver, SIGNAL(transferAdded(TransferHandler *)),
                     this, SLOT(addTransferHandler(TransferHandler *)));
    QObject::connect(&m_transferGroupObserver, SIGNAL(transferDeleted(TransferHandler *)) ,
                    this, SLOT(deleteTransferHandler(TransferHandler *)));
    QObject::connect(&m_transferGroupObserver, SIGNAL(transferGroupChanged(TransferGroupHandler *)),
                    this, SLOT(slotTransferGroupChanged(TransferGroupHandler *)));
}

void DBusModelObserver::addedTransferGroupEvent(TransferGroupHandler *group)
{
    group->addObserver(&m_transferGroupObserver);

    m_transferGroupHandlers.append(group);
}

void DBusModelObserver::removedTransferGroupEvent(TransferGroupHandler *group)
{
    group->delObserver(&m_transferGroupObserver);

    m_transferGroupHandlers.removeAll(group);
}

QVariantMap DBusModelObserver::transfers() const
{
    return m_transfers;
}

int DBusModelObserver::transfersSpeed() const
{
    return m_speed;
}

void DBusModelObserver::addTransferHandler(TransferHandler *handler)
{
    m_transfers[handler->source().pathOrUrl()] = getAttributesFromTransfer(handler);
}

void DBusModelObserver::deleteTransferHandler(TransferHandler *handler)
{
    m_transfers.remove(handler->source().pathOrUrl());
}

void DBusModelObserver::slotTransferGroupChanged(TransferGroupHandler *handler)
{
    // TODO: update the this transfers in the  transfers map
    foreach(TransferHandler *t_handler, handler->transfers()) {
        m_transfers[t_handler->source().pathOrUrl()] = getAttributesFromTransfer(t_handler);
    }

    m_speed = handler->speed();
//    emit transfersChanged(m_transfers);
}

QStringList DBusModelObserver::getAttributesFromTransfer(TransferHandler *handler)
{
    QStringList attributes;
    QString status = (handler->status () == Job::Running) ? "1" : "0";

    attributes << handler->source().fileName();
    attributes << QString::number(handler->percent());
    attributes << QString::number(handler->totalSize());
    attributes << status;
    attributes << QString::number(handler->speed());

    return attributes;
}
