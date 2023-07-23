/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "core/transferhistorystore.h"
#include "core/transferhistorystore_xml_p.h"
#ifdef HAVE_SQLITE
#include "core/transferhistorystore_sqlite_p.h"
#endif
#include "core/transfer.h"
#include "settings.h"

#include <QDateTime>
#include <QDir>
#include <QThread>

#include <KIO/Global>
#include <QStandardPaths>

TransferHistoryItem::TransferHistoryItem()
    : QObject()
{
}

TransferHistoryItem::TransferHistoryItem(const Transfer &transfer)
    : QObject()
{
    setDest(transfer.dest().toLocalFile());
    setSource(transfer.source().url());
    setSize(transfer.totalSize());
    setDateTime(QDateTime::currentDateTime());

    setState(transfer.status());
}

TransferHistoryItem::TransferHistoryItem(const TransferHistoryItem &item)
    : QObject()
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());
}

bool TransferHistoryItem::isExpired(qint64 expiryAge)
{
    if (expiryAge == -1)
        return false;

    const qint64 lifeSpan = dateTime().secsTo(QDateTime::currentDateTime());
    return (lifeSpan > expiryAge);
}

void TransferHistoryItem::setDest(const QString &dest)
{
    m_dest = dest;
}

void TransferHistoryItem::setSource(const QString &source)
{
    m_source = source;
}

void TransferHistoryItem::setState(int state)
{
    m_state = state;
}

void TransferHistoryItem::setSize(int size)
{
    m_size = size;
}

void TransferHistoryItem::setDateTime(const QDateTime &dateTime)
{
    m_dateTime = dateTime;
}

QString TransferHistoryItem::dest() const
{
    return m_dest;
}

QString TransferHistoryItem::source() const
{
    return m_source;
}

int TransferHistoryItem::state() const
{
    return m_state;
}

int TransferHistoryItem::size() const
{
    return m_size;
}

QDateTime TransferHistoryItem::dateTime() const
{
    return m_dateTime;
}

TransferHistoryItem &TransferHistoryItem::operator=(const TransferHistoryItem &item)
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());

    return *this;
}

bool TransferHistoryItem::operator==(const TransferHistoryItem &item) const
{
    return dest() == item.dest() && source() == item.source();
}

TransferHistoryStore::TransferHistoryStore()
    : QObject()
    , m_items()
    , m_expiryAge(getSettingsExpiryAge())
{
}

TransferHistoryStore::~TransferHistoryStore()
{
}

qint64 TransferHistoryStore::getSettingsExpiryAge()
{
    if (!Settings::automaticDeletionEnabled())
        return -1;

    qint64 timeVal = static_cast<qint64>(Settings::expiryTimeValue());
    switch (Settings::expiryTimeType()) {
    case Day:
        timeVal *= 24;
        [[fallthrough]];
    case Hour:
        timeVal *= 60;
        [[fallthrough]];
    case Minute:
        timeVal *= 60;
    }
    return timeVal;
}

qint64 TransferHistoryStore::expiryAge() const
{
    return m_expiryAge;
}

void TransferHistoryStore::updateExpiryAge(qint64 timeVal)
{
    m_expiryAge = timeVal;
    deleteExpiredItems();
}

void TransferHistoryStore::deleteExpiredItems()
{
    QList<TransferHistoryItem> &items = m_items;
    for (auto &item : items) {
        if (item.isExpired(m_expiryAge))
            deleteItem(item);
    }
}

QList<TransferHistoryItem> TransferHistoryStore::items() const
{
    return m_items;
}

void TransferHistoryStore::settingsChanged()
{
    updateExpiryAge(getSettingsExpiryAge());
}

TransferHistoryStore *TransferHistoryStore::getStore()
{
    // make sure that the DataLocation directory exists (earlier this used to be handled by KStandardDirs)
    if (!QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }
    switch (Settings::historyBackend()) {
    case TransferHistoryStore::SQLite:
#ifdef HAVE_SQLITE
        return new SQLiteStore(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/transferhistory.db"));
        break;
#endif
    case TransferHistoryStore::Xml:
    default:
        return new XmlStore(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/transferhistory.kgt"));
    }
}

#include "moc_transferhistorystore.cpp"
