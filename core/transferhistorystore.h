/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef TRANSFERHISTORYSTORE_H
#define TRANSFERHISTORYSTORE_H

#include "kget_export.h"

#include <QObject>
#include <QList>
#include <QMetaType>
#include <QDateTime>

class Transfer;

class KGET_EXPORT TransferHistoryItem : public QObject
{
public:
    TransferHistoryItem();
    TransferHistoryItem(const Transfer &transfer);
    TransferHistoryItem(const TransferHistoryItem &);

    void setDest(const QString &dest);
    void setSource(const QString &source);
    void setState(int state);
    void setSize(int size);
    void setDateTime(const QDateTime &time);

    QString dest() const;
    QString source() const;
    int state() const;
    int size() const;
    QDateTime dateTime() const;

    TransferHistoryItem& operator=(const TransferHistoryItem&);
    bool operator==(const TransferHistoryItem&) const;

private:
    QString m_dest;
    QString m_source;
    int m_state;
    int m_size;
    QDateTime m_dateTime;
};

class KGET_EXPORT TransferHistoryStore : public QObject
{
    Q_OBJECT
public:
    enum Backend {
        Xml = 0,
        SQLite = 1,
        Nepomuk = 2
    };

    TransferHistoryStore();
    ~TransferHistoryStore();

    QList<TransferHistoryItem> items() const;

    static TransferHistoryStore *getStore();

public slots:
    virtual void load() {};
    virtual void clear() {};
    virtual void saveItem(const TransferHistoryItem &item)
    {
        Q_UNUSED(item)
    };

    virtual void saveItems(const QList<TransferHistoryItem> &items)
    {
        foreach(const TransferHistoryItem &item, items) {
            saveItem(item);
        }
    };
    virtual void deleteItem(const TransferHistoryItem &item)
    {
        Q_UNUSED(item)
    };

signals:
    void elementLoaded(int number, int total, const TransferHistoryItem &item);
    void loadFinished();
    void saveFinished();
    void deleteFinished();

protected:
    QList<TransferHistoryItem> m_items;
};



Q_DECLARE_METATYPE(TransferHistoryItem)

#endif
