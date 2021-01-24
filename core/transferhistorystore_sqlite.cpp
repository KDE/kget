/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifdef HAVE_SQLITE
#include "core/transferhistorystore_sqlite_p.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QFile>
    
#include "kget_debug.h"
#include <QDebug>

SQLiteStore::SQLiteStore(const QString &database) : TransferHistoryStore(),
    m_dbName(database),
    m_sql()
{
}

SQLiteStore::~SQLiteStore()
{
    if (m_sql.isOpen()) {
        m_sql.close();
    }
}

void SQLiteStore::load()
{
    m_items.clear();
    if (sql().open()) {
        if (!sql().tables().contains("transfer_history_item")) {
            createTables();
        }

        QSqlQuery query = sql().exec("SELECT * FROM transfer_history_item");

        if (query.lastError().isValid()) {
            qCDebug(KGET_DEBUG) << query.lastError().text();
        }
        else {
            QSqlRecord rec = query.record();

            while (query.next()) {
                TransferHistoryItem item;
                item.setDest(query.value(rec.indexOf("dest")).toString());
                item.setSource(query.value(rec.indexOf("source")).toString());
                item.setState(query.value(rec.indexOf("state")).toInt());
                item.setDateTime(QDateTime::fromSecsSinceEpoch(query.value(rec.indexOf("time")).toUInt()));
                item.setSize(query.value(rec.indexOf("size")).toInt());

                m_items << item;
                Q_EMIT elementLoaded(query.at(), query.size(), item);
            }
        }
    }

    sql().close();

    Q_EMIT loadFinished();
}

void SQLiteStore::clear()
{
    QFile::remove(m_dbName);
}

void SQLiteStore::saveItem(const TransferHistoryItem &item)
{
    saveItems(QList<TransferHistoryItem>() << item);
}

void SQLiteStore::saveItems(const QList<TransferHistoryItem> &items)
{
    if (sql().open()) {
        if (!sql().tables().contains("transfer_history_item")) {
            createTables();
        }

        if (!sql().transaction()) {
            qCWarning(KGET_DEBUG) << "Could not establish a transaction, might be slow.";
        }

        foreach (const TransferHistoryItem &item, items) {
            QSqlQuery query = sql().exec("insert into transfer_history_item(source, dest, size, time, state)"
                                "values ('"+item.source()+"', '"+item.dest()+"', "
                                + QString::number(item.size()) + ", "
                                + QString::number(item.dateTime().toSecsSinceEpoch()) + ", '"
                                + QString::number(item.state())+"')");

            if (query.lastError().isValid()) {
                qCDebug(KGET_DEBUG) << query.lastError().text();
            }
            m_items << item;
        }

        if (!sql().commit()) {
            qCWarning(KGET_DEBUG) << "Could not commit changes.";
        }
    }
    sql().close();

    Q_EMIT saveFinished();
}

void SQLiteStore::deleteItem(const TransferHistoryItem &item)
{
    if (sql().open()) {
        if (!sql().tables().contains("transfer_history_item")) {
            createTables();
        }

        QSqlQuery query = sql().exec("delete from transfer_history_item where "
                                            " source = '" + item.source() + "';");

        if (query.lastError().isValid()) {
            qCDebug(KGET_DEBUG) << query.lastError().text();
        }

        sql().commit();
        m_items.removeAll(item);
    }
    sql().close();

    Q_EMIT deleteFinished();
}

QSqlDatabase SQLiteStore::sql()
{
    if (!m_sql.isValid()) {
        m_sql = QSqlDatabase::addDatabase("QSQLITE");
        m_sql.setDatabaseName(m_dbName);
    }

    return m_sql;
}

void SQLiteStore::createTables()
{
    QSqlQuery query = sql().exec("CREATE TABLE transfer_history_item(dest VARCHAR NOT NULL, "
                                "source VARCHAR NOT NULL, size int NOT NULL, time int not null, "
                                "state int, PRIMARY KEY(dest, source));");

    if (query.lastError().isValid()) {
        qCDebug(KGET_DEBUG) << query.lastError().text();
    }
}

#endif

