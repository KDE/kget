/* This file is part of the KDE project

   Copyright (C) 2007 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef TRANSFERHISTORYSTORE_XML_P_H
#define TRANSFERHISTORYSTORE_XML_P_H

#include <QList>
#include <QThread>

class TransferHistoryItem;
class XmlStore : public TransferHistoryStore
{
    Q_OBJECT
public:
    XmlStore(const QString &url);
    ~XmlStore();

public slots:
    void load();
    void saveItem(const TransferHistoryItem &item);
    void deleteItem(const TransferHistoryItem &item);

    void slotLoadElement(int number, int total, const TransferHistoryItem &item);

private:
    QString m_storeUrl;

    class LoadThread;
    LoadThread *m_loadThread;

    class SaveThread;
    SaveThread *m_saveThread;
};


class XmlStore::LoadThread : public QThread
{
    Q_OBJECT
public:
    LoadThread(QObject *parent, const QString &url);

    void run();

signals:
    void elementLoaded(int number, int total, const TransferHistoryItem &item);

private:
    QString m_url;
};

class XmlStore::SaveThread : public QThread
{
    Q_OBJECT
public:
    SaveThread(QObject *parent, const QString &url, const QList<TransferHistoryItem> &list);
    SaveThread(QObject *parent, const QString &url, const TransferHistoryItem &item);

    void run();

signals:
    void elementLoaded(int number, int total, const TransferHistoryItem &item);

private:
    QString m_url;
    QList <TransferHistoryItem> m_items;
    TransferHistoryItem m_item;
};
#endif
