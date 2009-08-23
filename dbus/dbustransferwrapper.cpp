/* This file is part of the KDE project

   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "dbustransferwrapper.h"
#include "transferhandler.h"
#include "transfergrouphandler.h"

DBusTransferWrapper::DBusTransferWrapper(TransferHandler * parent)
  : QObject(parent),
    m_transfer(parent)
{
}

DBusTransferWrapper::~DBusTransferWrapper()
{
}

bool DBusTransferWrapper::supportsSpeedLimits()
{
    return m_transfer->supportsSpeedLimits();
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

bool DBusTransferWrapper::isResumable() const
{
    return m_transfer->isResumable();
}

QString DBusTransferWrapper::groupName() const
{
    return m_transfer->group()->name();
}

QString DBusTransferWrapper::source() const
{
    return m_transfer->source().prettyUrl();
}

QString DBusTransferWrapper::dest() const
{
    return m_transfer->dest().prettyUrl();
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
