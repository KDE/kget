/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/transfertreemodel.h"

#include "core/kget.h"
#include "core/transfertreeselectionmodel.h"
#include "core/transfergrouphandler.h"
#include "core/transfergroup.h"
#include "core/transferhandler.h"
#include "core/transfer.h"
#include "settings.h"

#include <typeinfo>
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
    beginRemoveRows(QModelIndex(), m_transferGroups.indexOf(group), m_transferGroups.indexOf(group));

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

    // Remove index corresponding to when it's created.
    int remove_index = group->indexOf(transfer);
    beginRemoveRows(createIndex(m_transferGroups.indexOf(group), 0, group->handler()), remove_index, remove_index);

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

Transfer *TransferTreeModel::findTransferByDestination(const KUrl &dest)
{
    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    Transfer *t;
    for(; it!=itEnd; ++it) {
        if((t = (*it)->findTransferByDestination(dest))) {
            return t;
        }
    }

    return 0;
}

bool TransferTreeModel::isTransferGroup(const QModelIndex & index) const
{
//     kDebug(5001) << "TransferTreeModel::isTransferGroup()";

    void * pointer = index.internalPointer();

    Handler *ph = static_cast<Handler*>(pointer);
    //kDebug() << "ph's typeid is" << typeid(*ph).name();
    //kDebug() << "TransferGroupHandler's typeid is" << typeid(*(m_transferGroups[0]->handler())).name();
    if (typeid(*ph) == typeid(*(m_transferGroups[0]->handler())))
    {
        return true;
    }
    return false;
}

void TransferTreeModel::postDataChangedEvent(TransferHandler * transfer)
{
    if(m_timerId == -1)
        m_timerId = startTimer(500);

    m_changedTransfers.append(transfer);
}

void TransferTreeModel::postDataChangedEvent(TransferGroupHandler * group)
{
    if(m_timerId == -1)
        m_timerId = startTimer(500);

    m_changedGroups.append(group);
}

QModelIndex TransferTreeModel::createIndex(int row, int column, void * ptr) const
{
    static int i = 0;

    i++;

//     kDebug(5001) << "TransferTreeModel::createIndex() " << i;

    return QAbstractItemModel::createIndex(row, column, ptr);
}

int TransferTreeModel::rowCount(const QModelIndex & parent) const{
//     kDebug(5001) << "TransferTreeModel::rowCount()";

    if(!parent.isValid())
    {
//         kDebug(5001) << "      (ROOT)  -> return " << m_transferGroups.size();
        return m_transferGroups.size();
    }

    if(isTransferGroup(parent))
    {
        void * pointer = parent.internalPointer();

        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

//         kDebug(5001) << "      (GROUP:" << group->name() << ") -> return " << group->size();

        return group->size();
    }

    return 0;
}

int TransferTreeModel::columnCount(const QModelIndex & parent) const
{
//     kDebug(5001) << "TransferTreeModel::columnCount()";

    if(!parent.isValid())
    {
        //Here we should return rootItem->columnCount(); .. but we don't
        //have a root Item... bah
        return 6;
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
//     kDebug(5001) << "TransferTreeModel::flags()";
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

    //flags |= Qt::ItemIsDropEnabled;

    // We can edit all the groups but the default one
    if(index.row() > 0) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

QVariant TransferTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) 
    {
        switch (section)
        {
            case 0:
                return i18nc("name of download", "Name");
            case 1:
                return i18nc("status of download", "Status");
            case 2:
                return i18nc("size of download", "Size");
            case 3:
                return i18nc("progress of download", "Progress");
            case 4:
                return i18nc("speed of download", "Speed");
            case 5:
                return i18nc("remaining time of download", "Remaining Time");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant TransferTreeModel::data(const QModelIndex & index, int role) const
{
//     kDebug(5001) << "TransferTreeModel::data()";

    if (!index.isValid())
    {
//         kDebug(5001) << "           (ROOT)";
        return QVariant();
    }

    // KextendableItemDelegate::ShowExtensionIndicatorRole
    // tells the KExtendableItemDelegate which column contains the extender icon
    if (role == Qt::UserRole + 200 && !isTransferGroup(index)) {
        if (index.column () == 0 && Settings::showExpandableTransferDetails()) {
            return true;
        }
        else {
            return false;
        }
    }

    if (role == Qt::DisplayRole || role == Qt::DecorationRole)
    {
        void * pointer = index.internalPointer();

        if(isTransferGroup(index))
        {
            TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);
            if (role == Qt::DisplayRole)
            {
//                 kDebug(5001) << "           (GROUP)";
                //The given index refers to a group object
                return group->data(index.column());
            }
            else //Qt::DecorationRole -> icon
            {
                if (index.column() == 0)
                    return group->pixmap();
                else
                    return QVariant();
            }
        }

//         kDebug(5001) << "           (TRANSFER)";

        //The given index refers to a transfer object
        TransferHandler * transfer = static_cast<TransferHandler *>(pointer);

        if (role == Qt::DisplayRole)
            return transfer->data(index.column());
        else //Qt::DecorationRole -> icon
        {
            switch (index.column())
            {
                case 0:
                    return KIO::pixmapForUrl(transfer->dest(), 0, KIconLoader::Desktop, 16);
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
                case 0: // name
                    return Qt::AlignVCenter;
                case 2: // size
                    return Qt::AlignCenter;
                case 4: // speed
                    return Qt::AlignCenter;
                case 3: //progress
                    return Qt::AlignCenter;
                default:
                    return QVariant(Qt::AlignLeft | Qt::AlignBottom);
            }
        }

        switch (index.column())
        {
            case 0: // name
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            case 2: // size
                return Qt::AlignCenter;
            case 4: // speed
                return Qt::AlignCenter;
            case 3: //progress
                return Qt::AlignCenter;
            default:
                return Qt::AlignCenter;
        }
    }

    return QVariant();
}

QModelIndex TransferTreeModel::index(int row, int column, const QModelIndex & parent) const
{
//     kDebug(5001) << "TransferTreeModel::index()  ( " << row << " , " << column << " )";

    if(!parent.isValid())
    {
//         kDebug(5001) << "TransferTreeModel::index() -> group ( " << row << " , " << column << " )   Groups=" << m_transferGroups.size();
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
        //The given parent refers to a TransferGroupHandler object
        void * pointer = parent.internalPointer();
        TransferGroupHandler * group = static_cast<TransferGroupHandler *>(pointer);

        //Look for the specific transfer
        if(row < group->size() && row >= 0)
        {
//             kDebug(5001) << "aa      row=" << row;
            (*group)[row];
//             kDebug(5001) << "bb";
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
//     kDebug(5001) << "TransferTreeModel::parent()";

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

    foreach (const QModelIndex &index, indexes) 
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

    kDebug(5001) << "TransferTreeModel::dropMimeData    DATA:";
    kDebug(5001) << stringList;


    for(int i=0; i < rows; i++)
    {
//         TransferGroup * group = findGroup(stringList[i]);

//         TransferGroup * destGroup = static_cast<TransferGroup *>(index(row, column, parent).internalPointer());

        TransferGroup * destGroup = findGroup(data(parent, Qt::DisplayRole).toString());

        TransferHandler * transferHandler = (TransferHandler *) stringList[++i].toInt(0, 16);

        if(destGroup)
            moveTransfer(transferHandler->m_transfer, destGroup);
    }
    return true;
}

QString TransferTreeModel::columnName(int column)
{
    switch(column) {
        case TransferTreeModel::Name:
            return i18n("Name");
        case TransferTreeModel::Status:
            return i18n("Status");
        case TransferTreeModel::Size:
            return i18n("Size");
        case TransferTreeModel::Progress:
            return i18n("Progress");
        case TransferTreeModel::Speed:
            return i18n("Speed");
        case TransferTreeModel::RemainingTime:
            return i18n("Remaining Time");
        default:
            return QString();
    }
}

void TransferTreeModel::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
//     kDebug(5001) << "TransferTreeModel::timerEvent";

    QList<TransferHandler *> updatedTransfers;
    QList<TransferGroupHandler *> updatedGroups;

    foreach(TransferHandler * transfer, m_changedTransfers)
    {
        if(!updatedTransfers.contains(transfer))
        {
            TransferGroupHandler * group = transfer->group();
            Transfer::ChangesFlags changesFlags = transfer->changesFlags(0);

            for(int i=0; i<8; i++)
            {
                if(((changesFlags >> i) & 0x00000001) == 1)
                {
                    QModelIndex index = createIndex(group->indexOf(transfer), i, transfer);
                    emit dataChanged(index,index);
                }
            }

            transfer->resetChangesFlags(0);
            updatedTransfers.append(transfer);
        }
    }

    foreach(TransferGroupHandler * group, m_changedGroups)
    {
        if(!updatedGroups.contains(group))
        {
            TransferGroup::ChangesFlags changesFlags = group->changesFlags(0);

            for(int i=0; i<8; i++)
            {
                if(((changesFlags >> i) & 0x00000001) == 1)
                {
                    QModelIndex index = createIndex(m_transferGroups.indexOf(group->m_group), i, group);
                    emit dataChanged(index,index);
                }
            }

            group->resetChangesFlags(0);
            updatedGroups.append(group);
        }
    }

    m_changedTransfers.clear();
    m_changedGroups.clear();

    killTimer(m_timerId);
    m_timerId = -1;
}

#include "transfertreemodel.moc"
