/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "core/transfertreemodel.h"

#include "core/kget.h"
#include "core/transfertreeselectionmodel.h"
#include "core/transfergrouphandler.h"
#include "core/transfergroup.h"
#include "core/transferhandler.h"
#include "core/transfer.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kiconloader.h>

#include <qmimedata.h>

TransferTreeModel::TransferTreeModel(Scheduler * scheduler)
    : QAbstractItemModel(),
      m_scheduler(scheduler),
      m_timerId(-1)
{

}

TransferTreeModel::~TransferTreeModel()
{

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
    m_changedGroups.removeAll(group->handler());

    endRemoveRows();
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

    group->remove(transfer);
    m_changedTransfers.removeAll(transfer->handler());

    endRemoveRows();
}

void TransferTreeModel::moveTransfer(Transfer * transfer, TransferGroup * destGroup, Transfer * after)
{
    if( (after) && (destGroup != after->group()) )
        return;

    int removePosition = transfer->group()->indexOf(transfer);
    int insertPosition = destGroup->size();
    int groupIndex = m_transferGroups.indexOf(destGroup);

    beginRemoveRows(createIndex(groupIndex, 0, destGroup->handler()), removePosition, removePosition);

    beginInsertRows(createIndex(m_transferGroups.indexOf(destGroup), 0, destGroup->handler()), insertPosition, insertPosition);

    if(destGroup == transfer->group())
    {
        if(after)
            destGroup->move(transfer, after);
        else
            destGroup->move(transfer, static_cast<Transfer *>(destGroup->last()));
    }
    else
    {
        transfer->group()->remove(transfer);

        if(destGroup->size() != 0)
            destGroup->insert(transfer, static_cast<Transfer *>(destGroup->last()));
        else
            destGroup->append(transfer);

        transfer->m_jobQueue = destGroup;
    }

    endInsertRows();
    endRemoveRows();

    KGet::selectionModel()->clearSelection();
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

Transfer * TransferTreeModel::findTransfer(const KUrl &src)
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

bool TransferTreeModel::isTransferGroup(const QModelIndex & index) const
{
//     kDebug(5001) << "TransferTreeModel::isTransferGroup()" << endl;

    void * pointer = index.internalPointer();

    foreach(TransferGroup * group, m_transferGroups)
    {
        if(group->handler() == pointer)
        {
            return true;
        }
    }

    return false;
}

void TransferTreeModel::postDataChangedEvent(TransferHandler * transfer)
{
    if(m_timerId == -1)
        m_timerId = startTimer(200);

    m_changedTransfers.append(transfer);
}

void TransferTreeModel::postDataChangedEvent(TransferGroupHandler * group)
{
    if(m_timerId == -1)
        m_timerId = startTimer(200);

    m_changedGroups.append(group);
}

QModelIndex TransferTreeModel::createIndex(int row, int column, void * ptr) const
{
    static int i = 0;

    i++;

//     kDebug(5001) << "TransferTreeModel::createIndex() " << i << endl;

    return QAbstractItemModel::createIndex(row, column, ptr);
}

int TransferTreeModel::rowCount(const QModelIndex & parent) const{
//     kDebug(5001) << "TransferTreeModel::rowCount()" << endl;

    if(!parent.isValid())
    {
//         kDebug(5001) << "      (ROOT)  -> return " << m_transferGroups.size() << endl;
        return m_transferGroups.size();
    }

    if(isTransferGroup(parent))
    {
        void * pointer = parent.internalPointer();

        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

//         kDebug(5001) << "      (GROUP:" << group->name() << ") -> return " << group->size() << endl;

        return group->size();
    }

    return 0;
}

int TransferTreeModel::columnCount(const QModelIndex & parent) const
{
//     kDebug(5001) << "TransferTreeModel::columnCount()" << endl;

    if(!parent.isValid())
    {
        //Here we should return rootItem->columnCount(); .. but we don't
        //have a root Item... bah
        return 5;
    }

    void * pointer = parent.internalPointer();

    if(isTransferGroup(parent))
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
//     kDebug(5001) << "TransferTreeModel::flags()" << endl;
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if(isTransferGroup(index))
    {
        if(index.column() == 0)
            flags |= Qt::ItemIsDropEnabled;
    }
    else
        flags |= Qt::ItemIsDragEnabled;

    return flags;
}

QVariant TransferTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
    {
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
//     kDebug(5001) << "TransferTreeModel::data()" << endl;

    if (!index.isValid())
    {
//         kDebug(5001) << "           (ROOT)" << endl;
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::DecorationRole)
    {
        void * pointer = index.internalPointer();

        if(isTransferGroup(index))
        {
            if (role == Qt::DisplayRole)
            {
//                 kDebug(5001) << "           (GROUP)" << endl;
                //The given index refers to a group object
                TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);
                return group->data(index.column());
            }
            else //Qt::DecorationRole -> icon
            {
                if (index.column() == 0)
                    return SmallIcon("todolist", 32);
                else
                    return QVariant();
            }
        }

//         kDebug(5001) << "           (TRANSFER)" << endl;

        //The given index refers to a transfer object
        TransferHandler * transfer = static_cast<TransferHandler *>(pointer);

        if (role == Qt::DisplayRole)
            return transfer->data(index.column());
        else //Qt::DecorationRole -> icon
        {
            switch (index.column())
            {
                case 0:
                    return KIO::pixmapForUrl(transfer->source(), 0, K3Icon::Desktop, 16);
                case 1:
                    return transfer->statusPixmap();
                default:
                    return QVariant();
            }
        }
    }

    if (role == Qt::TextAlignmentRole)
    {
        if(isTransferGroup(index))
        {
            switch (index.column())
            {
                case 2: // size
                case 4: // speed
                    return QVariant(Qt::AlignRight  | Qt::AlignBottom);
                case 3: //progress
                    return QVariant(Qt::AlignHCenter  | Qt::AlignBottom);
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignBottom);
            }
        }

        switch (index.column())
        {
            case 2: // size
            case 4: // speed
                return QVariant(Qt::AlignRight | Qt::AlignVCenter);
            case 3: //progress
                return Qt::AlignCenter;
            default:
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    return QVariant();
}

QModelIndex TransferTreeModel::index(int row, int column, const QModelIndex & parent) const
{
//     kDebug(5001) << "TransferTreeModel::index()  ( " << row << " , " << column << " )" << endl;

    if(!parent.isValid())
    {
//         kDebug(5001) << "TransferTreeModel::index() -> group ( " << row << " , " << column << " )   Groups=" << m_transferGroups.size() << endl;
        //Look for the specific group
        if(row < m_transferGroups.size() && row >= 0)
        {
            return createIndex(row, column, m_transferGroups[row]->handler());

        }
        else
            return QModelIndex();
    }

    if(isTransferGroup(parent))
    {
        //The given parent refers to a TransferGroup object
        void * pointer = parent.internalPointer();
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

        //Look for the specific transfer
        if(row < group->size() && row >= 0)
        {
//             kDebug(5001) << "aa      row=" << row << endl;
            (*group)[row];
//             kDebug(5001) << "bb" << endl;
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
//     kDebug(5001) << "TransferTreeModel::parent()" << endl;

    if(!index.isValid())
        return QModelIndex();

    if(!isTransferGroup(index))
    {
        //The given index refers to a Transfer item
        void * pointer = index.internalPointer();
        TransferGroupHandler * group = static_cast<TransferHandler *>(pointer)->group();
        return createIndex(m_transferGroups.indexOf(group->m_group), 0, group);
    }

    return QModelIndex();
}

Qt::DropActions TransferTreeModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TransferTreeModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

QMimeData * TransferTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) 
    {
        if (index.isValid())
        {
            if(index.column() == 0 && index.parent().isValid())
            {
                stream << data(index.parent(), Qt::DisplayRole).toString();
                stream << QString::number((qulonglong) index.internalPointer(),16);
            }
        }
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

bool TransferTreeModel::dropMimeData(const QMimeData * mdata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!mdata->hasFormat("application/vnd.text.list"))
        return false;

    if (column > 0)
        return false;

    if (parent.isValid())
        kDebug(5001) << "TransferTreeModel::dropMimeData" << " " << row << " " 
                                                          << column << endl;

    QByteArray encodedData = mdata->data("application/vnd.text.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList stringList;
    int rows = 0;

    while (!stream.atEnd()) 
    {
        QString text;
        stream >> text;
        stringList << text;
        ++rows;
    }

    kDebug(5001) << "TransferTreeModel::dropMimeData    DATA:" << endl;
    kDebug(5001) << stringList << endl;


    for(int i=0; i < rows; i++)
    {
//         TransferGroup * group = findGroup(stringList[i]);

//         TransferGroup * destGroup = static_cast<TransferGroup *>(index(row, column, parent).internalPointer());

        TransferGroup * destGroup = findGroup(data(parent, Qt::DisplayRole).toString());

        TransferHandler * transferHandler = (TransferHandler *) stringList[++i].toInt(0, 16);

        moveTransfer(transferHandler->m_transfer, destGroup);
    }
    return true;
}

void TransferTreeModel::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
//     kDebug(5001) << "TransferTreeModel::timerEvent" << endl;

    QList<TransferHandler *> updatedTransfers;
    QList<TransferGroupHandler *> updatedGroups;

    foreach(TransferHandler * transfer, m_changedTransfers)
    {
        if(!updatedTransfers.contains(transfer))
        {
            TransferGroupHandler * group = transfer->group();

            emit dataChanged(createIndex(group->indexOf(transfer), 0, transfer),
                            createIndex(group->indexOf(transfer), transfer->columnCount(), transfer));

            updatedTransfers.append(transfer);
        }
    }

    foreach(TransferGroupHandler * group, m_changedGroups)
    {
        if(!updatedGroups.contains(group))
        {
            emit dataChanged(createIndex(m_transferGroups.indexOf(group->m_group), 0, group),
                            createIndex(m_transferGroups.indexOf(group->m_group), group->columnCount(), group));

            updatedGroups.append(group);
        }
    }

    m_changedTransfers.clear();
    m_changedGroups.clear();

    killTimer(m_timerId);
    m_timerId = -1;
}

#include "transfertreemodel.moc"
