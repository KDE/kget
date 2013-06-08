/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef TRANSFERHISTORYSTORE_NEPOMUK_P_H
#define TRANSFERHISTORYSTORE_NEPOMUK_P_H

#include "transferhistorystore.h"

#include <QList>

namespace Nepomuk2
{
    namespace Query
    {
        class Result;
    }
}

class TransferHistoryItem;
class NepomukStore : public TransferHistoryStore
{
    Q_OBJECT
public:
    NepomukStore(const QString &database);
    ~NepomukStore();

public slots:
    void load();
    void loadResult(const QList<Nepomuk2::Query::Result>& entries);
    void clear();
    void saveItem(const TransferHistoryItem &item);
    void deleteItem(const TransferHistoryItem &item);
};
#endif
 
