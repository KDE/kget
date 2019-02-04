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
#include <QList>
#include <QThread>

#include <kio/global.h>
#include <QStandardPaths>


TransferHistoryItem::TransferHistoryItem() : QObject()
{}

TransferHistoryItem::TransferHistoryItem(const Transfer &transfer) : QObject()
{
    setDest(transfer.dest().toLocalFile());
    setSource(transfer.source().url());
    setSize(transfer.totalSize());
    setDateTime(QDateTime::currentDateTime());

    setState(transfer.status());
}

TransferHistoryItem::TransferHistoryItem(const TransferHistoryItem &item) : QObject()
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());
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

TransferHistoryItem& TransferHistoryItem::operator=(const TransferHistoryItem &item)
{
    setDest(item.dest());
    setSource(item.source());
    setState(item.state());
    setSize(item.size());
    setDateTime(item.dateTime());

    return *this;
}

bool TransferHistoryItem::operator==(const TransferHistoryItem& item) const
{
    return dest() == item.dest() && source() == item.source();
}

TransferHistoryStore::TransferHistoryStore() : QObject(),
    m_items()
{
}

TransferHistoryStore::~TransferHistoryStore()
{
}

QList <TransferHistoryItem> TransferHistoryStore::items() const
{
    return m_items;
}

TransferHistoryStore *TransferHistoryStore::getStore()
{
    switch(Settings::historyBackend())
    {
        case TransferHistoryStore::SQLite:
#ifdef HAVE_SQLITE
            return new SQLiteStore(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QStringLiteral("/transferhistory.db"));
            break;
#endif
        case TransferHistoryStore::Xml:
        default:
            return new XmlStore(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QStringLiteral("/transferhistory.kgt"));
    }
}


