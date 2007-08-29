/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dbustransfergroupobserver.h"

#include <QObject>
#include <KDebug>

DBusTransferGroupObserver::DBusTransferGroupObserver(QObject *parent)
  : QObject(parent)
{
}

void DBusTransferGroupObserver::groupChangedEvent(TransferGroupHandler *group)
{
    emit transferGroupChanged(group);
}

void DBusTransferGroupObserver::addedTransferEvent(TransferHandler *transfer, TransferHandler *after)
{
    Q_UNUSED(after);
    emit transferAdded(transfer);
}

void DBusTransferGroupObserver::removedTransferEvent(TransferHandler *transfer)
{
    emit transferDeleted(transfer);
}

void DBusTransferGroupObserver::movedTransferEvent(TransferHandler *transfer, TransferHandler *after)
{
    Q_UNUSED(transfer)
    Q_UNUSED(after)
}

void DBusTransferGroupObserver::deleteEvent(TransferGroupHandler *group)
{
    Q_UNUSED(group)
}
