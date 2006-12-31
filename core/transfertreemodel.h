/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERTREEMODEL_H
#define _TRANSFERTREEMODEL_H

#include <QAbstractItemModel>
#include <QList>

#include <kget_export.h>

class KUrl;

class TransferGroupHandler;
class TransferGroup;
class TransferHandler;
class Transfer;
class Scheduler;

class TransferTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class TransferGroupHandler;
    friend class TransferGroup;
    friend class TransferHandler;
    friend class Transfer;

    public:
        TransferTreeModel(Scheduler * scheduler);
        ~TransferTreeModel();

        void addGroup(TransferGroup * group);
        void delGroup(TransferGroup * group);

        void addTransfer(Transfer * transfer, TransferGroup * group);
        void delTransfer(Transfer * transfer);

        void moveTransfer(Transfer * transfer, TransferGroup * destGroup, Transfer * after=0);

        const QList<TransferGroup *> & transferGroups();

        TransferGroup * findGroup(const QString & groupName);
        Transfer * findTransfer(KUrl src);

        bool KGET_EXPORT isTransferGroup(const QModelIndex & index) const;

        void postDataChangedEvent(TransferHandler * transfer);
        void postDataChangedEvent(TransferGroupHandler * group);

        QModelIndex createIndex(int row, int column, void * ptr = 0) const;

        //QAbstractItemModel functions
        int rowCount(const QModelIndex & parent) const;
        int columnCount(const QModelIndex & parent) const;
        Qt::ItemFlags flags (const QModelIndex & index) const;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;
        QVariant data (const QModelIndex & index, int role) const;
        QModelIndex index(int row, int column, const QModelIndex & parent) const;
        QModelIndex parent(const QModelIndex & index) const;

        //Drag & drop functions
        Qt::DropActions supportedDropActions() const;
        QStringList mimeTypes() const;
        QMimeData * mimeData(const QModelIndexList &indexes) const;
        bool dropMimeData(const QMimeData *mdata,
                          Qt::DropAction action, int row, int column, 
                          const QModelIndex &parent);


    private:
        void timerEvent(QTimerEvent *event);

        QList<TransferGroup *> m_transferGroups;
        Scheduler * m_scheduler;

        // Timer related variables
        QList<TransferHandler *> m_changedTransfers;
        QList<TransferGroupHandler *> m_changedGroups;

        int m_timerId;
};

#endif
