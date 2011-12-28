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

#include <QStandardItemModel>
#include <QList>
#include <QtCore/QMimeData>
#include <QtCore/QWeakPointer>

#include "../kget_export.h"
#include "core/transfer.h"
#include "core/transfergroup.h"
#include "core/handler.h"

class KUrl;

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
        ~ItemMimeData();

        /**
         * Appends a transfer to the list of transfers.
         * The weakpointer is there to check later on, that the transfer still exists
         */
        void appendTransfer(const QWeakPointer<TransferHandler> &transfer);

        /**
         * Returns all appended transfers
         * The weakpointer is there to check later on, that the transfer still exists
         */
        QList<QWeakPointer<TransferHandler> > transfers() const;

    private:
        QList<QWeakPointer<TransferHandler> > m_transfers;
};

class KGET_EXPORT ModelItem : public QStandardItem
{
    public:
        ModelItem(Handler * handler);
        virtual ~ModelItem();

        virtual QVariant data(int role = Qt::UserRole + 1) const = 0;
        void emitDataChanged();
        Handler * handler();
        virtual bool isGroup();
        
        GroupModelItem * asGroup();
        TransferModelItem * asTransfer();

    private:
        Handler * m_handler;
};

class KGET_EXPORT TransferModelItem : public ModelItem
{
    public:
        TransferModelItem(TransferHandler *handler);
        virtual ~TransferModelItem();

        virtual QVariant data(int role = Qt::UserRole + 1) const;

        TransferHandler * transferHandler();

    private:
        TransferHandler * m_transferHandler;
        mutable KIcon m_mimeType;
};

class KGET_EXPORT GroupModelItem : public ModelItem
{
    public:
        GroupModelItem(TransferGroupHandler *handler);
        virtual ~GroupModelItem();
        
        virtual QVariant data(int role = Qt::UserRole + 1) const;
        
        TransferGroupHandler * groupHandler();
        
        virtual bool isGroup();
        
    private:
        TransferGroupHandler * m_groupHandler;
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
        TransferTreeModel(Scheduler * scheduler);
        ~TransferTreeModel();

        void addGroup(TransferGroup * group);
        void delGroup(TransferGroup * group);

        void addTransfers(const QList<Transfer*> &transfers, TransferGroup *group);
        void delTransfers(const QList<Transfer*> &transfers);

        TransferModelItem * itemFromTransferHandler(TransferHandler * handler);
        GroupModelItem * itemFromTransferGroupHandler(TransferGroupHandler * handler);
        ModelItem * itemFromHandler(Handler * handler);

        ModelItem * itemFromIndex(const QModelIndex &index) const;

        void moveTransfer(Transfer * transfer, TransferGroup * destGroup, Transfer * after = 0);
        void moveTransfer(TransferHandler *transfer, TransferGroupHandler *destGroup, TransferHandler *after = 0);

        QList<TransferGroup *> transferGroups();

        TransferGroup * findGroup(const QString & groupName);
        Transfer * findTransfer(const KUrl &src);
        Transfer * findTransferByDestination(const KUrl &dest);
        Transfer * findTransferByDBusObjectPath(const QString & dbusObjectPath);

        void postDataChangedEvent(TransferHandler * transfer);
        void postDataChangedEvent(TransferGroupHandler * group);

        Qt::ItemFlags flags (const QModelIndex & index) const;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;

        //Drag & drop functions
        Qt::DropActions supportedDropActions() const;
        QStringList mimeTypes() const;
        QMimeData * mimeData(const QModelIndexList &indexes) const;
        bool dropMimeData(const QMimeData *mdata,
                          Qt::DropAction action, int row, int column, 
                          const QModelIndex &parent);

        static QString columnName(int column);
        static int column(Transfer::TransferChange flag);
        static int column(TransferGroup::GroupChange flag);

    signals:
        void groupAddedEvent(TransferGroupHandler *);
        void groupRemovedEvent(TransferGroupHandler *);
        void groupsChangedEvent(QMap<TransferGroupHandler *, TransferGroup::ChangesFlags>);
        void transfersAddedEvent(QList<TransferHandler*> transfers);
        void transfersAboutToBeRemovedEvent(const QList<TransferHandler*> &transfers);
        void transfersRemovedEvent(const QList<TransferHandler*> &transfers);
        void transferMovedEvent(TransferHandler *, TransferGroupHandler *);
        void transfersChangedEvent(QMap<TransferHandler *, Transfer::ChangesFlags>);

    private:
        void timerEvent(QTimerEvent *event);

        Scheduler * m_scheduler;

        // Timer related variables
        QList<TransferHandler *> m_changedTransfers;
        QList<TransferGroupHandler *> m_changedGroups;
        
        QList<GroupModelItem*> m_transferGroups;
        QList<TransferModelItem*> m_transfers;

        int m_timerId;
};

#endif
