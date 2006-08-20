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
#include <kiconloader.h>

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

    kDebug(5001) << "TransferTreeModel::createIndex() " << i << endl;

    return QAbstractItemModel::createIndex(row, column, ptr);
}

int TransferTreeModel::rowCount(const QModelIndex & parent) const{
    kDebug(5001) << "TransferTreeModel::rowCount()" << endl;

    if(!parent.isValid())
    {
        kDebug(5001) << "      (ROOT)  -> return " << m_transferGroups.size() << endl;
        return m_transferGroups.size();
    }

    void * pointer = parent.internalPointer();

    if(isTransferGroup(pointer))
    {
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

        kDebug(5001) << "      (GROUP:" << group->name() << ") -> return " << group->size() << endl;

        return group->size();
    }

    return 0;
}

int TransferTreeModel::columnCount(const QModelIndex & parent) const
{
    kDebug(5001) << "TransferTreeModel::columnCount()" << endl;

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
    kDebug(5001) << "TransferTreeModel::flags()" << endl;
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant TransferTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section)
        {
            case 0:
                return i18n("Name");
            case 1:
                return i18n("Status");
            case 2:
                return i18n("Size");
            case 3:
                return i18n("Progress");
            case 4:
                return i18n("Speed");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant TransferTreeModel::data(const QModelIndex & index, int role) const
{
    kDebug(5001) << "TransferTreeModel::data()" << endl;

    if (!index.isValid())
    {
        kDebug(5001) << "           (ROOT)" << endl;
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::DecorationRole)
    {
        void * pointer = index.internalPointer();

        if(isTransferGroup(pointer))
        {
            if (role == Qt::DisplayRole)
            {
                kDebug(5001) << "           (GROUP)" << endl;
                //The given index refers to a group object
                TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);
                return group->data(index.column());
            }
            else //Qt::DecorationRole -> icon
            {
                if (index.column() == 0)
                    return SmallIcon("folder", 22);
                else
                    return QVariant();
            }
        }

        kDebug(5001) << "           (TRANSFER)" << endl;

        //The given index refers to a transfer object
        TransferHandler * transfer = static_cast<TransferHandler *>(pointer);

        if (role == Qt::DisplayRole)
            return transfer->data(index.column());
        else //Qt::DecorationRole -> icon
        {
            switch (index.column())
            {
                case 0:
                    return KIO::pixmapForURL(transfer->source(), 0, K3Icon::Desktop, 16);
                case 1:
                    return transfer->statusPixmap();
                default:
                    return QVariant();
            }
        }
    }

    if ((index.column() == 2 || index.column() == 4) && Qt::TextAlignmentRole == role) //numbers right aligned
        return Qt::AlignRight;

    return QVariant();
}

QModelIndex TransferTreeModel::index(int row, int column, const QModelIndex & parent) const
{
    kDebug(5001) << "TransferTreeModel::index()  ( " << row << " , " << column << " )" << endl;

    if(!parent.isValid())
    {
        kDebug(5001) << "TransferTreeModel::index() -> group ( " << row << " , " << column << " )   Groups=" << m_transferGroups.size() << endl;
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
            kDebug(5001) << "aa      row=" << row << endl;
            (*group)[row];
            kDebug(5001) << "bb" << endl;
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
    kDebug(5001) << "TransferTreeModel::parent()" << endl;

//     kDebug(5001) << "111" << endl;

    if(!index.isValid())
        return QModelIndex();

//     kDebug(5001) << "222" << endl;

    void * pointer = index.internalPointer();

    if(!isTransferGroup(pointer))
    {
//         kDebug(5001) << "333" << endl;
        //The given index refers to a Transfer item
//         TransferHandler * transfer = static_cast<TransferHandler *>(pointer);
        TransferGroupHandler * group = static_cast<TransferHandler *>(pointer)->group();
//         kDebug(5001) << "444" << endl;
//         return transfer->group()->index(0);
        return createIndex(m_transferGroups.indexOf(group->m_group), 0, group);
    }

//     kDebug(5001) << "555" << endl;
    return QModelIndex();
}

bool TransferTreeModel::isTransferGroup(void * pointer) const
{
    kDebug(5001) << "TransferTreeModel::isTransferGroup()" << endl;

    foreach(TransferGroup * group, m_transferGroups)
    {
//         kDebug(5001) << "TransferTreeModel::isTransferGroup   -> ITERATION" << endl;
        if(group->handler() == pointer)
        {
            kDebug(5001) << "TransferTreeModel::isTransferGroup   -> return TRUE" << endl;
            return true;
        }
    }

    kDebug(5001) << "TransferTreeModel::isTransferGroup   -> return FALSE" << endl;
    return false;
}

#include "transfertreemodel.moc"
