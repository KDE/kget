/* This file is part of the KDE project

   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "dbustransferwrapper.h"
#include "transfergrouphandler.h"
#include "verifier.h"

DBusTransferWrapper::DBusTransferWrapper(TransferHandler *parent)
  : QObject(parent),
    m_transfer(parent)
{
    connect(m_transfer, SIGNAL(transferChangedEvent(TransferHandler*,TransferHandler::ChangesFlags)), this, SLOT(slotTransferChanged(TransferHandler*,TransferHandler::ChangesFlags)));
    connect(m_transfer, SIGNAL(capabilitiesChanged()), this, SIGNAL(capabilitiesChanged()));
}

DBusTransferWrapper::~DBusTransferWrapper()
{
}

int DBusTransferWrapper::capabilities() const
{
    return m_transfer->capabilities();
}

void DBusTransferWrapper::start()
{
    m_transfer->start();
}

void DBusTransferWrapper::stop()
{
    m_transfer->stop();
}

int DBusTransferWrapper::status() const
{
    return (int)m_transfer->status();
}

int DBusTransferWrapper::elapsedTime() const
{
    return m_transfer->elapsedTime();
}

int DBusTransferWrapper::remainingTime() const
{
    return m_transfer->remainingTime();
}

QString DBusTransferWrapper::groupName() const
{
    return m_transfer->group()->name();
}

QString DBusTransferWrapper::source() const
{
    return m_transfer->source().pathOrUrl();
}

QString DBusTransferWrapper::dest() const
{
    return m_transfer->dest().pathOrUrl();
}

bool DBusTransferWrapper::setDirectory(const QString &directory)
{
    return m_transfer->setDirectory(KUrl(directory));
}

qulonglong DBusTransferWrapper::totalSize() const
{
    return m_transfer->totalSize();
}

qulonglong DBusTransferWrapper::downloadedSize() const
{
    return m_transfer->downloadedSize();
}

qulonglong DBusTransferWrapper::uploadedSize() const
{
    return m_transfer->uploadedSize();
}

int DBusTransferWrapper::percent() const
{
    return m_transfer->percent();
}

int DBusTransferWrapper::downloadSpeed() const
{
    return m_transfer->downloadSpeed();
}

int DBusTransferWrapper::uploadSpeed() const
{
    return m_transfer->uploadSpeed();
}

void DBusTransferWrapper::setUploadLimit(int ulLimit, int limit)
{
    m_transfer->setUploadLimit(ulLimit, (Transfer::SpeedLimit)limit);
}

void DBusTransferWrapper::setDownloadLimit(int dlLimit, int limit)
{
    m_transfer->setDownloadLimit(dlLimit, (Transfer::SpeedLimit)limit);
}

int DBusTransferWrapper::uploadLimit(int limit) const
{
    return m_transfer->uploadLimit((Transfer::SpeedLimit)limit);
}

int DBusTransferWrapper::downloadLimit(int limit) const
{
    return m_transfer->downloadLimit((Transfer::SpeedLimit)limit);
}

void DBusTransferWrapper::setMaximumShareRatio(double ratio)
{
    m_transfer->setMaximumShareRatio(ratio);
}

double DBusTransferWrapper::maximumShareRatio()
{
    return m_transfer->maximumShareRatio();
}

QString DBusTransferWrapper::statusText() const
{
    return m_transfer->statusText();
}

QDBusVariant DBusTransferWrapper::statusPixmap() const
{
    return QDBusVariant(QVariant::fromValue(m_transfer->statusPixmap()));
}

void DBusTransferWrapper::slotTransferChanged(TransferHandler *transfer, TransferHandler::ChangesFlags changeFlags)
{
    Q_UNUSED(transfer)

    emit transferChangedEvent(changeFlags);
}

QString DBusTransferWrapper::verifier(const QString &file)
{
    Verifier *verifier = m_transfer->verifier(KUrl(file));
    if (verifier) {
        return verifier->dBusObjectPath();
    }

    return QString();
}

bool DBusTransferWrapper::repair(const QString &file)
{
    return m_transfer->repair(KUrl(file));

}

#include "dbustransferwrapper.moc"
