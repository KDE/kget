/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef TRANSFERHISTORYSTORE_SQLITE_P_H
#define TRANSFERHISTORYSTORE_SQLITE_P_H

#ifdef HAVE_SQLITE

#include "transferhistorystore.h"

#include <QList>
#include <QSqlDatabase>

class TransferHistoryItem;
class SQLiteStore : public TransferHistoryStore
{
    Q_OBJECT
public:
    SQLiteStore(const QString &database);
    ~SQLiteStore() override;

public Q_SLOTS:
    void load() override;
    void clear() override;
    void saveItem(const TransferHistoryItem &item) override;
    void saveItems(const QList<TransferHistoryItem> &items) override;
    void deleteItem(const TransferHistoryItem &item) override;

private:
    QSqlDatabase sql();
    void createTables();

private:
    QString m_dbName;
    QSqlDatabase m_sql;
};
#endif
#endif
