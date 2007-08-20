/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DBUS_TRANSFERGROUP_OBSERVER_H
#define DBUS_TRANSFERGROUP_OBSERVER_H

#include <QObject>

#include "core/observer.h"

class DBusTransferGroupObserver : public QObject, public TransferGroupObserver
{
Q_OBJECT
public:
    DBusTransferGroupObserver(QObject *parent = 0);

    void groupChangedEvent(TransferGroupHandler *group);
    void addedTransferEvent(TransferHandler *transfer, TransferHandler *after);
    void removedTransferEvent(TransferHandler *transfer);
    void movedTransferEvent(TransferHandler *transfer, TransferHandler *after);
    void deleteEvent(TransferGroupHandler *group);

signals:
    void transferAdded(TransferHandler *handler);
    void transferDeleted(TransferHandler *handler);
    void transferGroupChanged(TransferGroupHandler *group);

};

#endif
