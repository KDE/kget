/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERTREEMODEL_H
#define TRANSFERTREEMODEL_H

#include <QList>
#include <QMimeData>
#include <QPointer>
#include <QStandardItemModel>
#include <QUrl>

#include "core/handler.h"
#include "core/transfer.h"
#include "core/transfergroup.h"
#include "kget_export.h"

class TransferGroupHandler;
class TransferGroup;
class TransferHandler;
class Transfer;
class Scheduler;
class TransferModelItem;
class GroupModelItem;

class ItemMimeData : public QMimeData
{
    Q_OBJECT
public:
    ItemMimeData();
    ~ItemMimeData() override;

    /**
     * Appends a transfer to the list of transfers.
     * The pointer is there to check later on, that the transfer still exists
     */
    void appendTransfer(const QPointer<TransferHandler> &transfer);

    /**
     * Returns all appended transfers
     * The pointer is there to check later on, that the transfer still exists
     */
    QList<QPointer<TransferHandler>> transfers() const;

private:
    QList<QPointer<TransferHandler>> m_transfers;
};

class KGET_EXPORT ModelItem : public QStandardItem
{
public:
    ModelItem(Handler *handler);
    ~ModelItem() override;

    QVariant data(int role = Qt::UserRole + 1) const override = 0;
    void emitDataChanged();
    Handler *handler();
    virtual bool isGroup();

    GroupModelItem *asGroup();
    TransferModelItem *asTransfer();

private:
    Handler *m_handler;
};

class KGET_EXPORT TransferModelItem : public ModelItem
{
public:
    TransferModelItem(TransferHandler *handler);
    ~TransferModelItem() override;

    QVariant data(int role = Qt::UserRole + 1) const override;

    TransferHandler *transferHandler();

private:
    TransferHandler *m_transferHandler;
    mutable QIcon m_mimeType;
};

class KGET_EXPORT GroupModelItem : public ModelItem
{
public:
    GroupModelItem(TransferGroupHandler *handler);
    ~GroupModelItem() override;

    QVariant data(int role = Qt::UserRole + 1) const override;

    TransferGroupHandler *groupHandler();

    bool isGroup() override;

private:
    TransferGroupHandler *m_groupHandler;
};

class KGET_EXPORT TransferTreeModel : public QStandardItemModel
{
    Q_OBJECT

    friend class TransferGroupHandler;
    friend class TransferGroup;
    friend class TransferHandler;
    friend class Transfer;

public:
    enum Columns {
        Name,
        Status,
        Size,
        Progress,
        Speed,
        RemainingTime
    };
    TransferTreeModel(Scheduler *scheduler);
    ~TransferTreeModel() override;

    void addGroup(TransferGroup *group);
    void delGroup(TransferGroup *group);

    void addTransfers(const QList<Transfer *> &transfers, TransferGroup *group);
    void delTransfers(const QList<Transfer *> &transfers);

    TransferModelItem *itemFromTransferHandler(TransferHandler *handler);
    GroupModelItem *itemFromTransferGroupHandler(TransferGroupHandler *handler);
    ModelItem *itemFromHandler(Handler *handler);

    ModelItem *itemFromIndex(const QModelIndex &index) const;

    void moveTransfer(Transfer *transfer, TransferGroup *destGroup, Transfer *after = nullptr);
    void moveTransfer(TransferHandler *transfer, TransferGroupHandler *destGroup, TransferHandler *after = nullptr);

    QList<TransferGroup *> transferGroups();

    TransferGroup *findGroup(const QString &groupName);
    Transfer *findTransfer(const QUrl &src);
    Transfer *findTransferByDestination(const QUrl &dest);
    Transfer *findTransferByDBusObjectPath(const QString &dbusObjectPath);

    void postDataChangedEvent(TransferHandler *transfer);
    void postDataChangedEvent(TransferGroupHandler *group);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Drag & drop functions
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *mdata, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    static QString columnName(int column);
    static int column(Transfer::TransferChange flag);
    static int column(TransferGroup::GroupChange flag);

Q_SIGNALS:
    void groupAddedEvent(TransferGroupHandler *);
    void groupRemovedEvent(TransferGroupHandler *);
    void groupsChangedEvent(QMap<TransferGroupHandler *, TransferGroup::ChangesFlags>);
    void transfersAddedEvent(QList<TransferHandler *> transfers);
    void transfersAboutToBeRemovedEvent(const QList<TransferHandler *> &transfers);
    void transfersRemovedEvent(const QList<TransferHandler *> &transfers);
    void transferMovedEvent(TransferHandler *, TransferGroupHandler *);
    void transfersChangedEvent(QMap<TransferHandler *, Transfer::ChangesFlags>);

private:
    void timerEvent(QTimerEvent *event) override;

    Scheduler *m_scheduler;

    // Timer related variables
    QList<TransferHandler *> m_changedTransfers;
    QList<TransferGroupHandler *> m_changedGroups;

    QList<GroupModelItem *> m_transferGroups;
    QList<TransferModelItem *> m_transfers;

    int m_timerId;
};

#endif
