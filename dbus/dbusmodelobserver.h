/* This file is part of the KDE project

   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DBUS_MODEL_OBSERVER_H
#define DBUS_MODEL_OBSERVER_H

#include "dbustransfergroupobserver.h"
#include "core/observer.h"

#include <QObject>
#include <QVariant>
#include <QList>

class DBusModelObserver : public QObject, public ModelObserver
{
Q_OBJECT
public:
    DBusModelObserver(QObject *parent = 0);

    void addedTransferGroupEvent(TransferGroupHandler *group);
    void removedTransferGroupEvent(TransferGroupHandler *group);

    QVariantMap transfers() const;

private slots:
    void addTransferHandler(TransferHandler *handler);
    void deleteTransferHandler(TransferHandler *handler);
    void slotTransferGroupChanged(TransferGroupHandler *handler);

private:
    QStringList getAttributesFromTransfer(TransferHandler *handler);

    QVariantMap m_transfers;
    DBusTransferGroupObserver m_transferGroupObserver;
    QList<TransferGroupHandler *> m_transferGroupHandlers;
};


#endif
