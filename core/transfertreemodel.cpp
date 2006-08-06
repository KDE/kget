/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>

#include "transfertreemodel.h"
#include "core/transfergrouphandler.h"
#include "core/transfergroup.h"
#include "core/transferhandler.h"
#include "core/transfer.h"

TransferTreeModel::TransferTreeModel(Scheduler * scheduler)
    : QAbstractItemModel(),
      m_scheduler(scheduler)
{
    
}

TransferTreeModel::~TransferTreeModel()
{

}

void TransferTreeModel::addTransfer(Transfer * transfer, TransferGroup * group)
{
    beginInsertRows(createIndex(m_transferGroups.indexOf(group), 0, group->handler()), group->size(), group->size());

    group->append(transfer);

    endInsertRows();
}

void TransferTreeModel::delTransfer(Transfer * transfer)
{
    TransferGroup * group = transfer->group();

    beginRemoveRows(createIndex(m_transferGroups.indexOf(group), 0, group->handler()), group->size()-1, group->size()-1);

    transfer->group()->remove( transfer );

    endRemoveRows();
}

void TransferTreeModel::addGroup(TransferGroup * group)
{
    beginInsertRows(QModelIndex(), m_transferGroups.size(), m_transferGroups.size());

    m_transferGroups.append(group);

    endInsertRows();
}

void TransferTreeModel::delGroup(TransferGroup * group)
{
    beginRemoveRows(QModelIndex(), m_transferGroups.size()-1, m_transferGroups.size()-1);

    m_transferGroups.removeAll(group);

    endRemoveRows();
}

const QList<TransferGroup *> & TransferTreeModel::transferGroups()
{
    return m_transferGroups;
}

TransferGroup * TransferTreeModel::findGroup(const QString & groupName)
{
    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    for(; it!=itEnd ; ++it)
    {
        if( (*it)->name() == groupName )
        {
            return *it;
        }
    }
    return 0;
}

Transfer * TransferTreeModel::findTransfer(KUrl src)
{
    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    Transfer * t;

    for(; it!=itEnd ; ++it)
    {
        if( ( t = (*it)->findTransfer(src) ) )
            return t;
    }
    return 0;
}

void TransferTreeModel::postDataChangedEvent(TransferHandler * transfer)
{
    TransferGroupHandler * group = transfer->group();

    emit dataChanged(createIndex(group->indexOf(transfer), 0, transfer),
                     createIndex(group->indexOf(transfer), transfer->columnCount(), transfer));
}

void TransferTreeModel::postDataChangedEvent(TransferGroupHandler * group)
{
    emit dataChanged(createIndex(m_transferGroups.indexOf(group->m_group), 0, group),
                     createIndex(m_transferGroups.indexOf(group->m_group), group->columnCount(), group));
}

QModelIndex TransferTreeModel::createIndex(int row, int column, void * ptr) const
{
    static int i = 0;

    i++;

    kDebug() << "TransferTreeModel::createIndex() " << i << endl;

    return QAbstractItemModel::createIndex(row, column, ptr);
}

int TransferTreeModel::rowCount(const QModelIndex & parent) const{
    kDebug() << "TransferTreeModel::rowCount()" << endl;

    if(!parent.isValid())
    {
        kDebug() << "      (ROOT)  -> return " << m_transferGroups.size() << endl;
        return m_transferGroups.size();
    }

    void * pointer = parent.internalPointer();

    if(isTransferGroup(pointer))
    {
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

        kDebug() << "      (GROUP:" << group->name() << ") -> return " << group->size() << endl;

        return group->size();
    }

    return 0;
}

int TransferTreeModel::columnCount(const QModelIndex & parent) const
{
    kDebug() << "TransferTreeModel::columnCount()" << endl;

    if(!parent.isValid())
    {
        //Here we should return rootItem->columnCount(); .. but we don't
        //have a root Item... bah
        return 5;
    }

    void * pointer = parent.internalPointer();

    if(isTransferGroup(pointer))
    {
        //The given index refers to a group object
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);
        return group->columnCount();
    }

    //The given index refers to a group object
    TransferHandler * transfer = static_cast<TransferHandler *>(pointer);
    return transfer->columnCount();
}


Qt::ItemFlags TransferTreeModel::flags (const QModelIndex & index) const
{
    kDebug() << "TransferTreeModel::flags()" << endl;
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TransferTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (section)
    {
        case 0:
            return QVariant(i18n("File"));
        case 1:
            return QVariant(i18n("Status"));
        case 2:
            return QVariant(i18n("Size"));
        case 3:
            return QVariant(i18n("Progress"));
        case 4:
            return QVariant(i18n("Speed"));
    }
}


QVariant TransferTreeModel::data(const QModelIndex & index, int role) const
{
    kDebug() << "TransferTreeModel::data()" << endl;

    if (!index.isValid())
    {
        kDebug() << "           (ROOT)" << endl;
        return QVariant();
    }

    if (role != Qt::DisplayRole)
    {
        kDebug() << "           not display role" << endl;
        return QVariant();
    }

    void * pointer = index.internalPointer();

    if(isTransferGroup(pointer))
    {
        kDebug() << "           (GROUP)" << endl;
        //The given index refers to a group object
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);
        return group->data(index.column());
    }

    kDebug() << "           (TRANSFER)" << endl;

    //The given index refers to a transfer object
    TransferHandler * transfer = static_cast<TransferHandler *>(pointer);
    return transfer->data(index.column());
}

QModelIndex TransferTreeModel::index(int row, int column, const QModelIndex & parent) const
{
    kDebug() << "TransferTreeModel::index()  ( " << row << " , " << column << " )" << endl;

    if(!parent.isValid())
    {
        kDebug() << "TransferTreeModel::index() -> group ( " << row << " , " << column << " )   Groups=" << m_transferGroups.size() << endl;
        //Look for the specific group
        if(row < m_transferGroups.size() && row >= 0)
        {
//             return m_transferGroups[row]->handler()->index(column);
            return createIndex(row, column, m_transferGroups[row]->handler());

        }
        else
            return QModelIndex();
    }

    void * pointer = parent.internalPointer();

    if(isTransferGroup(pointer))
    {
        //The given parent refers to a TransferGroup object
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

        //Look for the specific transfer
        if(row < group->size() && row >= 0)
        {
            kDebug() << "aa      row=" << row << endl;
            (*group)[row];
            kDebug() << "bb" << endl;
//             return (*group)[row]->index(column); 
            return createIndex(row, column, (*group)[row]);
        }
        else
            return QModelIndex();
    }

    //If here, the given parent is a Transfer object which hasn't any child
    return QModelIndex();
}

QModelIndex TransferTreeModel::parent(const QModelIndex & index ) const
{
    kDebug() << "TransferTreeModel::parent()" << endl;

//     kDebug() << "111" << endl;

    if(!index.isValid())
        return QModelIndex();

//     kDebug() << "222" << endl;

    void * pointer = index.internalPointer();

    if(!isTransferGroup(pointer))
    {
//         kDebug() << "333" << endl;
        //The given index refers to a Transfer item
//         TransferHandler * transfer = static_cast<TransferHandler *>(pointer);
        TransferGroupHandler * group = static_cast<TransferHandler *>(pointer)->group();
//         kDebug() << "444" << endl;
//         return transfer->group()->index(0);
        return createIndex(m_transferGroups.indexOf(group->m_group), 0, group);
    }

//     kDebug() << "555" << endl;
    return QModelIndex();
}

bool TransferTreeModel::isTransferGroup(void * pointer) const
{
    kDebug() << "TransferTreeModel::isTransferGroup()" << endl;

    foreach(TransferGroup * group, m_transferGroups)
    {
//         kDebug() << "TransferTreeModel::isTransferGroup   -> ITERATION" << endl;
        if(group->handler() == pointer)
        {
            kDebug() << "TransferTreeModel::isTransferGroup   -> return TRUE" << endl;
            return true;
        }
    }

    kDebug() << "TransferTreeModel::isTransferGroup   -> return FALSE" << endl;
    return false;
}

#include "transfertreemodel.moc"
