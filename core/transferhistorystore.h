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

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QObject>

class Transfer;

class KGET_EXPORT TransferHistoryItem : public QObject
{
public:
    TransferHistoryItem();
    TransferHistoryItem(const Transfer &transfer);
    TransferHistoryItem(const TransferHistoryItem &);

    bool isExpired(qint64 expiryAge);

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

    TransferHistoryItem &operator=(const TransferHistoryItem &);
    bool operator==(const TransferHistoryItem &) const;

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
    };

    enum Time {
        Day = 0,
        Hour = 1,
        Minute = 2,
        Second = 3,
    };

    TransferHistoryStore();
    ~TransferHistoryStore() override;

    QList<TransferHistoryItem> items() const;

    qint64 expiryAge() const;

    static TransferHistoryStore *getStore();
    static qint64 getSettingsExpiryAge();

    void settingsChanged();

public Q_SLOTS:
    virtual void load()
    {
    }
    virtual void clear()
    {
    }
    virtual void saveItem(const TransferHistoryItem &item)
    {
        Q_UNUSED(item)
    }
    virtual void saveItems(const QList<TransferHistoryItem> &items)
    {
        foreach (const TransferHistoryItem &item, items) {
            saveItem(item);
        }
    }
    virtual void deleteItem(const TransferHistoryItem &item)
    {
        Q_UNUSED(item)
    }

Q_SIGNALS:
    void elementLoaded(int number, int total, const TransferHistoryItem &item);
    void loadFinished();
    void saveFinished();
    void deleteFinished();

protected:
    void deleteExpiredItems();
    void updateExpiryAge(qint64 expiry);

    QList<TransferHistoryItem> m_items;
    qint64 m_expiryAge;
};

Q_DECLARE_METATYPE(TransferHistoryItem)

#endif
