/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

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
#include "transferadaptor.h"
#include "dbus/dbustransferwrapper.h"
#include "settings.h"
#include "transfergroupscheduler.h"

#include <kdebug.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kiconloader.h>

#include <qmimedata.h>

ModelItem::ModelItem(Handler * handler)
  : QStandardItem(),
    m_handler(handler)
{
}

ModelItem::~ModelItem()
{
}

void ModelItem::emitDataChanged()
{
    QStandardItem::emitDataChanged();
}

Handler * ModelItem::handler()
{
    return m_handler;
}

bool ModelItem::isGroup()
{
    return false;
}

GroupModelItem * ModelItem::asGroup()
{
    return dynamic_cast<GroupModelItem*>(this);
}

TransferModelItem * ModelItem::asTransfer()
{
    return dynamic_cast<TransferModelItem*>(this);
}


TransferModelItem::TransferModelItem(TransferHandler *handler)
  : ModelItem(handler),
    m_transferHandler(handler)
{
}

TransferModelItem::~TransferModelItem()
{
}

QVariant TransferModelItem::data(int role) const
{
    if (role == Qt::DisplayRole)
        return m_transferHandler->data(column());
    else if (role == Qt::DecorationRole)
    {
        switch (column())
        {
            case 0:
                return KIO::pixmapForUrl(m_transferHandler->dest(), 0, KIconLoader::Desktop, 16);
            case 1:
                return m_transferHandler->statusPixmap();
        }
    }
    if (role == Qt::TextAlignmentRole)
    {
        switch (column())
        {
            case 0: // name
                return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
            default:
                return Qt::AlignCenter;
        }
    }
    // KextendableItemDelegate::ShowExtensionIndicatorRole
    // tells the KExtendableItemDelegate which column contains the extender icon
    if (role == Qt::UserRole + 200)
    {
        if (column() == 0)
            return true;
        else
            return false;
    }
    return QVariant();
}

TransferHandler * TransferModelItem::transferHandler()
{
    return m_transferHandler;
}


GroupModelItem::GroupModelItem(TransferGroupHandler *handler)
  : ModelItem(handler),
    m_groupHandler(handler)
{
}

GroupModelItem::~GroupModelItem()
{
}

QVariant GroupModelItem::data(int role) const
{
    if (role == Qt::DisplayRole) {
        return m_groupHandler->data(column());
    }
    if (role == Qt::TextAlignmentRole)
    {
        switch (column())
        {
            case 0: // name
                return Qt::AlignVCenter;
            case 2: // size
            case 3: // speed
            case 4: //progress
                return Qt::AlignCenter;
            default:
                return QVariant(Qt::AlignLeft | Qt::AlignBottom);
        }
    }
    if (role == Qt::DecorationRole && column() == 0)
        return m_groupHandler->pixmap();
    return QVariant();
}

TransferGroupHandler * GroupModelItem::groupHandler()
{
    //kDebug() << m_groupHandler->name();
    return m_groupHandler;
}

bool GroupModelItem::isGroup()
{
    return true;
}


TransferTreeModel::TransferTreeModel(Scheduler * scheduler)
    : QStandardItemModel(),
      m_scheduler(scheduler),
      m_timerId(-1)
{
    m_transferGroups.clear();
    m_transfers.clear();
}

TransferTreeModel::~TransferTreeModel()
{

}

void TransferTreeModel::addGroup(TransferGroup * group)
{
    QList<QStandardItem*> items;
    for (int i = 0; i != group->handler()->columnCount(); i++)
        items << new GroupModelItem(group->handler());

    appendRow(items);

    m_transferGroups.append(static_cast<GroupModelItem*>(items.first()));

    emit groupAddedEvent(group->handler());

    KGet::m_scheduler->addQueue(group);
}

void TransferTreeModel::delGroup(TransferGroup * group)
{
    GroupModelItem * item = itemFromTransferGroupHandler(group->handler());
    if (item)
        takeRow(item->row());
    else
        return;

    m_changedGroups.removeAll(group->handler());

    emit groupRemovedEvent(group->handler());

    KGet::m_scheduler->delQueue(group);

    m_transferGroups.removeAll(item);
}

void TransferTreeModel::addTransfer(Transfer * transfer, TransferGroup * group)
{
    group->append(transfer);

    QList<QStandardItem*> items;
    for (int i = 0; i != transfer->handler()->columnCount(); i++)
        items << new TransferModelItem(transfer->handler());

    itemFromTransferGroupHandler(group->handler())->appendRow(items);

    emit transferAddedEvent(transfer->handler(), group->handler());
    
    m_transfers.append(static_cast<TransferModelItem*>(items.first()));

    DBusTransferWrapper * wrapper = new DBusTransferWrapper(transfer->handler());
    new TransferAdaptor(wrapper);
    QDBusConnection::sessionBus().registerObject(transfer->handler()->dBusObjectPath(), wrapper);
}

void TransferTreeModel::delTransfer(Transfer * transfer)
{
    TransferModelItem * item = itemFromTransferHandler(transfer->handler());

    if (!item)
        return;

    emit transferAboutToBeRemovedEvent(transfer->handler(), transfer->group()->handler());
    
    item->parent()->takeRow(item->row());

    QDBusConnection::sessionBus().unregisterObject(transfer->handler()->dBusObjectPath());

    TransferGroup * group = transfer->group();

    group->remove(transfer);
    m_changedTransfers.removeAll(transfer->handler());

    m_transfers.removeAll(item);

    emit transferRemovedEvent(transfer->handler(), group->handler());
}

TransferModelItem * TransferTreeModel::itemFromTransferHandler(TransferHandler * handler)
{
    foreach (TransferModelItem * item, m_transfers)
    {
        if (handler == item->transferHandler())
            return item;
    }
    return 0;
}

GroupModelItem * TransferTreeModel::itemFromTransferGroupHandler(TransferGroupHandler * handler)
{
    foreach (GroupModelItem * item, m_transferGroups)
    {
        if (handler == item->groupHandler())
            return item;
    }
    return 0;
}
    
ModelItem * TransferTreeModel::itemFromHandler(Handler * handler)
{
    if (qobject_cast<TransferHandler*>(handler)) {
        return itemFromTransferHandler(qobject_cast<TransferHandler*>(handler));
    }
    return itemFromTransferGroupHandler(qobject_cast<TransferGroupHandler*>(handler));
}

ModelItem * TransferTreeModel::itemFromIndex(const QModelIndex &index) const
{
    QStandardItem *item = QStandardItemModel::itemFromIndex(index);
    if (item)
        return dynamic_cast<ModelItem*>(item);
    return 0;
}

void TransferTreeModel::moveTransfer(Transfer * transfer, TransferGroup * destGroup, Transfer * after)
{
    if( (after) && (destGroup != after->group()) )
        return;
    
    int position = transfer->group()->indexOf(transfer);
    
    TransferGroup * oldGroup = transfer->group();

    bool sameGroup = false;

    if (destGroup == transfer->group())
    {
        sameGroup = true;
        if (after)
            destGroup->move(transfer, after);
        else
            destGroup->move(transfer, 0);
    }
    else
    {
        transfer->group()->remove(transfer);

        if (after)
            destGroup->insert(transfer, after);
        else
            destGroup->prepend(transfer);

        transfer->m_jobQueue = destGroup;
    }
    QList<QStandardItem*> items = itemFromHandler(oldGroup->handler())->takeRow(position);
    itemFromHandler(destGroup->handler())->insertRow(destGroup->indexOf(transfer), items);
    
    if (!sameGroup)
        emit transferMovedEvent(transfer->handler(), destGroup->handler());

    KGet::selectionModel()->clearSelection();
}

QList<TransferGroup *> TransferTreeModel::transferGroups()
{
    QList<TransferGroup*> transferGroups;
    foreach (GroupModelItem * item, m_transferGroups) {
        transferGroups << item->groupHandler()->m_group;
    }

    return transferGroups;
}

TransferGroup * TransferTreeModel::findGroup(const QString & groupName)
{
    foreach (GroupModelItem * group, m_transferGroups)
    {
        if (group->groupHandler()->name() == groupName)
            return group->groupHandler()->m_group;
    }
    return 0;
}

Transfer * TransferTreeModel::findTransfer(const KUrl &src)
{
    /*foreach (TransferGroup * group, transferGroups())
    {
        Transfer * t = group->findTransfer(src);
        if (t)
            return t;
    }*/
    foreach (TransferModelItem * transfer, m_transfers)
    {
        if (transfer->transferHandler()->source() == src)
            return transfer->transferHandler()->m_transfer;
    }
    return 0;
}

Transfer *TransferTreeModel::findTransferByDestination(const KUrl &dest)
{
    /*foreach (TransferGroup * group, transferGroups())
    {
        Transfer * t = group->findTransferByDestination(dest);
        if (t)
            return t;
    }*/
    foreach (TransferModelItem * transfer, m_transfers)
    {
        if (transfer->transferHandler()->dest() == dest)
            return transfer->transferHandler()->m_transfer;
    }
    return 0;
}

Transfer * TransferTreeModel::findTransferByDBusObjectPath(const QString & dbusObjectPath)
{
    foreach (TransferModelItem * transfer, m_transfers)
    {
        if (transfer->transferHandler()->dBusObjectPath() == dbusObjectPath)
            return transfer->transferHandler()->m_transfer;
    }
    return 0;
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

Qt::ItemFlags TransferTreeModel::flags (const QModelIndex & index) const
{
//     kDebug(5001) << "TransferTreeModel::flags()";
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (!index.parent().isValid())
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
        return columnName(section);
    }

    return QVariant();
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

    QModelIndexList sortedIndexes = indexes;
    qSort(sortedIndexes.begin(), sortedIndexes.end(), qGreater<QModelIndex>());
    foreach (const QModelIndex &index, sortedIndexes) 
    {
        if (index.isValid())
        {
            if(index.column() == 0 && index.parent().isValid())
            {
                stream << data(index.parent(), Qt::DisplayRole).toString();
                stream << QString::number((qulonglong) itemFromIndex(index)->handler(), 16);
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


    Transfer * after = 0;
    for(int i=0; i < rows; i++)
    {
//         TransferGroup * group = findGroup(stringList[i]);

//         TransferGroup * destGroup = static_cast<TransferGroup *>(index(row, column, parent).internalPointer());

        TransferGroup * destGroup = findGroup(data(parent, Qt::DisplayRole).toString());

        TransferHandler * transferHandler = (TransferHandler *) stringList[++i].toInt(0, 16);
        
        if (destGroup) {
            bool b = destGroup->size() > row && row - 1 >= 0;
            if (b)
                kDebug(5001) << "TRANSFER AFTER:" << destGroup->operator[](row - 1)->source();
            else
                kDebug(5001) << "TRANSFER AFTER NOT EXISTING";

            if (!after) {
                bool droppedInsideGroup = parent.isValid();
                bool rowValid = (row - 1 >= 0) && (destGroup->size() >= row);
                if (droppedInsideGroup && rowValid) {
                    after = destGroup->operator[](row - 1);//insert at the correct position
                }
            }
            moveTransfer(transferHandler->m_transfer, destGroup, after);
         }
    }
    return true;
}

QString TransferTreeModel::columnName(int column)
{
    switch(column) {
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
    }
    return QString();
}

int TransferTreeModel::column(Transfer::TransferChange flag)
{
    switch(flag) {
        case Transfer::Tc_FileName:
            return 0;
        case Transfer::Tc_Status:
            return 1;
        case Transfer::Tc_TotalSize:
            return 2;
        case Transfer::Tc_Percent:
            return 3;
        case Transfer::Tc_DownloadSpeed:
            return 4;
        case Transfer::Tc_RemainingTime:
            return 5;
        default:
            return -1;
    }
    return -1;
}

int TransferTreeModel::column(TransferGroup::GroupChange flag)
{
    switch(flag) {
        case TransferGroup::Gc_GroupName:
            return 0;
        case TransferGroup::Gc_Status:
            return 1;
        case TransferGroup::Gc_TotalSize:
            return 2;
        case TransferGroup::Gc_Percent:
            return 3;
        case TransferGroup::Gc_DownloadSpeed:
            return 4;
        default:
            return -1;
    }
    return -1;
}

void TransferTreeModel::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
//     kDebug(5001) << "TransferTreeModel::timerEvent";

    QMap<TransferHandler *, Transfer::ChangesFlags> updatedTransfers;
    QMap<TransferGroupHandler *, TransferGroup::ChangesFlags> updatedGroups;

    foreach (TransferHandler * transfer, m_changedTransfers)
    {
        if (!updatedTransfers.contains(transfer)) {
            TransferGroupHandler * group = transfer->group();
            ModelItem * item = itemFromHandler(group);
            Transfer::ChangesFlags changesFlags = transfer->changesFlags();

            emit transfer->transferChangedEvent(transfer, changesFlags);
            
            int row = group->indexOf(transfer);

//             kDebug(5001) << "CHILD = " << item->child(row, column(Transfer::Tc_FileName));
            
            // Now, check that model child items already exist (there are some cases when the transfer
            // can notify for changes before the gui has been correctly initialized)
            if(item->child(row, 0)) {
                if (changesFlags & Transfer::Tc_FileName)
                    static_cast<ModelItem*>(item->child(row, column(Transfer::Tc_FileName)))->emitDataChanged();
                if (changesFlags & Transfer::Tc_Status)
                    static_cast<ModelItem*>(item->child(row, column(Transfer::Tc_Status)))->emitDataChanged();
                if (changesFlags & Transfer::Tc_TotalSize)
                    static_cast<ModelItem*>(item->child(row, column(Transfer::Tc_TotalSize)))->emitDataChanged();
                if (changesFlags & Transfer::Tc_Percent)
                    static_cast<ModelItem*>(item->child(row, column(Transfer::Tc_Percent)))->emitDataChanged();
                if (changesFlags & Transfer::Tc_DownloadSpeed)
                    static_cast<ModelItem*>(item->child(row, column(Transfer::Tc_DownloadSpeed)))->emitDataChanged();
                if (changesFlags & Transfer::Tc_RemainingTime)
                    static_cast<ModelItem*>(item->child(row, column(Transfer::Tc_RemainingTime)))->emitDataChanged();

                transfer->resetChangesFlags();
                updatedTransfers.insert(transfer,changesFlags);            
            }    
        }
    }

    if(!updatedTransfers.isEmpty())
        emit transfersChangedEvent(updatedTransfers);

    foreach(TransferGroupHandler * group, m_changedGroups)
    {
        if(!updatedGroups.contains(group))
        {
            TransferGroup::ChangesFlags changesFlags = group->changesFlags();

            emit group->groupChangedEvent(group, changesFlags);
            
            int row = itemFromHandler(group)->row();
            
            if (changesFlags & TransferGroup::Gc_GroupName)
                static_cast<ModelItem*>(item(row, column(TransferGroup::Gc_GroupName)))->emitDataChanged();
            if (changesFlags & TransferGroup::Gc_Status)
                static_cast<ModelItem*>(item(row, column(TransferGroup::Gc_Status)))->emitDataChanged();
            if (changesFlags & TransferGroup::Gc_TotalSize)
                static_cast<ModelItem*>(item(row, column(TransferGroup::Gc_TotalSize)))->emitDataChanged();
            if (changesFlags & TransferGroup::Gc_Percent)
                static_cast<ModelItem*>(item(row, column(TransferGroup::Gc_Percent)))->emitDataChanged();
            if (changesFlags & TransferGroup::Gc_DownloadSpeed)
                static_cast<ModelItem*>(item(row, column(TransferGroup::Gc_DownloadSpeed)))->emitDataChanged();

            /*for(int i=0; i<8; i++)
            {
                if(((changesFlags >> i) & 0x00000001))
                {
                    QStandardItem *groupItem = itemFromHandler(group);
                    dynamic_cast<ModelItem*>(invisibleRootItem()->child(groupItem->row(), i))->emitDataChanged();
                    //QModelIndex index = createIndex(m_transferGroups.indexOf(group->m_group), i, group);
                    //emit dataChanged(index,index);
                }
            }*/

            group->resetChangesFlags();
            updatedGroups.insert(group, changesFlags);
        }
    }

    if(!updatedGroups.isEmpty())
        emit groupsChangedEvent(updatedGroups);

    m_changedTransfers.clear();
    m_changedGroups.clear();

    killTimer(m_timerId);
    m_timerId = -1;
}

#include "transfertreemodel.moc"
