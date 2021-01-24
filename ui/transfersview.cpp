/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfersview.h"
#include "settings.h"
#include "transfersviewdelegate.h"
#include "transferdetails.h"
#include "core/transfertreemodel.h"
#include "core/kget.h"

#include "kget_debug.h"

#include <KLocalizedString>
#include <KRun>

#include <QAction>
#include <QDebug>
#include <QDropEvent>
#include <QHeaderView>
#include <QMenu>
#include <QSignalMapper>
#include <QGroupBox>

TransfersView::TransfersView(QWidget * parent)
    : QTreeView(parent)
{
//     setItemsExpandable(false);
    setRootIsDecorated(false);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setMinimumSectionSize(80);    
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSectionsClickable(true);
    m_headerMenu = new QMenu(header());

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(header(), &QWidget::customContextMenuRequested,
                      this, &TransfersView::slotShowHeaderMenu);
    connect(header(), &QHeaderView::sectionCountChanged, this, &TransfersView::populateHeaderActions);
    connect(header(), &QHeaderView::sectionMoved, this, &TransfersView::slotSectionMoved);
    connect(header(), &QHeaderView::sectionResized, this, &TransfersView::slotSaveHeader);
    connect(this, &TransfersView::doubleClicked, this, &TransfersView::slotItemActivated);
    connect(this, &TransfersView::collapsed, this, &TransfersView::slotItemCollapsed);
    connect(KGet::model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), 
            this,          SLOT(closeExpandableDetails(QModelIndex,int,int)));
}

TransfersView::~TransfersView()
{
}

void TransfersView::setModel(QAbstractItemModel * model)
{
    QTreeView::setModel(model);
    int nGroups = model->rowCount(QModelIndex());

    for(int i = 0; i < nGroups; i++)
    {
        qCDebug(KGET_DEBUG) << "openEditor for row " << i;
        openPersistentEditor(model->index(i, TransferTreeModel::Status, QModelIndex()));
    }

    QByteArray loadedState = QByteArray::fromBase64(Settings::headerState().toLatin1());
    if (loadedState.isEmpty()) {
        setColumnWidth(0 , 230);
    } else {
        header()->restoreState(loadedState);
    }

    //Workaround if the saved headerState is corrupted
    header()->setRootIndex(QModelIndex());

    populateHeaderActions();
    toggleMainGroup();
    connect(model, &QAbstractItemModel::rowsRemoved, this, &TransfersView::toggleMainGroup);
}

void TransfersView::dropEvent(QDropEvent * event)
{
    QModelIndex dropIndex = indexAt(event->pos());
    QTreeView::dropEvent(event);

    setExpanded(dropIndex, true);
}

void TransfersView::rowsInserted(const QModelIndex & parent, int start, int end)
{
    qCDebug(KGET_DEBUG) << "TransfersView::rowsInserted";

    if(!parent.isValid())
    {
        qCDebug(KGET_DEBUG) << "parent is not valid " << start << "  " << end;

        for(int i = start; i <= end; i++)
        {
            qCDebug(KGET_DEBUG) << "openEditor for row " << i;
            openPersistentEditor(model()->index(i, TransferTreeModel::Status, parent));
        }
    }

    QTreeView::rowsInserted(parent, start, end);

    setExpanded(parent, true);
    toggleMainGroup();
}

void TransfersView::populateHeaderActions()
{
    m_headerMenu->clear();
    m_headerMenu->addSection(i18n("Select columns"));

    auto *columnMapper = new QSignalMapper(this);
    connect(columnMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &TransfersView::slotHideSection);

    //Create for each column an action with the column-header as name
    QVector<QAction *> orderedMenuItems(header()->count());
    for (int i = 0; i < header()->count(); ++i) {
        auto *action = new QAction(this);
        action->setText(model()->headerData(i, Qt::Horizontal).toString());
        action->setCheckable(true);
        action->setChecked(!header()->isSectionHidden(i));
        orderedMenuItems[header()->visualIndex(i)] = action;

        connect(action, SIGNAL(toggled(bool)), columnMapper, SLOT(map()));
        columnMapper->setMapping(action, i);
    }

    //append the sorted actions
    for (int i = 0; i < orderedMenuItems.count(); ++i) {
        m_headerMenu->addAction(orderedMenuItems[i]);
    }
}

void TransfersView::slotHideSection(int logicalIndex)
{
    const bool hide = !header()->isSectionHidden(logicalIndex);
    header()->setSectionHidden(logicalIndex, hide);
    slotSaveHeader();
}

void TransfersView::slotSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    Q_UNUSED(logicalIndex)

    //first item is the title, so increase the indexes by one
    ++oldVisualIndex;
    ++newVisualIndex;
    QList<QAction*> actions = m_headerMenu->actions();

    QAction *before = actions.last();
    if (newVisualIndex + 1 < actions.count()) {
        if (newVisualIndex > oldVisualIndex) {
            before = actions[newVisualIndex + 1];
        } else {
            before = actions[newVisualIndex];
        }
    }

    QAction *action = actions[oldVisualIndex];
    m_headerMenu->removeAction(action);
    m_headerMenu->insertAction(before, action);
    slotSaveHeader();
}

void TransfersView::slotSaveHeader()
{
    Settings::setHeaderState(header()->saveState().toBase64());
    Settings::self()->save();
}

void TransfersView::dragMoveEvent ( QDragMoveEvent * event )
{
    Q_UNUSED(event)

    closeExpandableDetails();
    QTreeView::dragMoveEvent(event);
}

void TransfersView::slotItemActivated(const QModelIndex & index)
{
    if (!index.isValid())
        return;

    TransferTreeModel * transferTreeModel = KGet::model();
    ModelItem * item = transferTreeModel->itemFromIndex(index);
    auto *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

    if(!item)
        return;

    if(!item->isGroup() && index.column() == 0) {
        if(!view_delegate->isExtended(index)) {
            TransferHandler *handler = item->asTransfer()->transferHandler();
            QWidget *widget = getDetailsWidgetForTransfer(handler);

            m_editingIndexes.append(index);
            view_delegate->extendItem(widget, index);
        }
        else {
            m_editingIndexes.removeAll(index);
            view_delegate->contractItem(index);
        }
        KGet::actionCollection()->action("transfer_show_details")->setChecked(view_delegate->isExtended(index));
    } else if (!item->isGroup() && static_cast<TransferModelItem*>(item)->transferHandler()->status() == Job::Finished) {
        new KRun(static_cast<TransferModelItem*>(item)->transferHandler()->dest(), this);
    }
}

void TransfersView::slotItemCollapsed(const QModelIndex & index)
{
    if (!index.isValid())
        return;

    TransferTreeModel * transferTreeModel = KGet::model();
    ModelItem * item = transferTreeModel->itemFromIndex(index);
    auto *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

    if(!item)
        return;
    
    if(item->isGroup()) {
        TransferGroupHandler * groupHandler = item->asGroup()->groupHandler();
        QList<TransferHandler *> transfers = groupHandler->transfers();

        foreach(TransferHandler * transfer, transfers) {
            qCDebug(KGET_DEBUG) << "Transfer = " << transfer->source().toString(); 
            view_delegate->contractItem(KGet::model()->itemFromTransferHandler(transfer)->index());
        }
    }
}

void TransfersView::toggleMainGroup()
{
    // show or hide the first group header if there's only one download group
    int nGroups = model()->rowCount(QModelIndex());

    if(nGroups <= 1) {
        setRootIndex(model()->index(0, 0, QModelIndex()));
    }
    else {
        setRootIndex(QModelIndex());
    }
    header()->setRootIndex(QModelIndex());//HACK: else the header isn't visible with no visible items in the view
}

void TransfersView::rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    closeExpandableDetails(currentIndex());
}

void TransfersView::slotShowHeaderMenu(const QPoint &point)
{
    m_headerMenu->popup(header()->mapToGlobal(point));
}

void TransfersView::closeExpandableDetails(const QModelIndex &transferIndex)
{
    auto *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());
    
    if(transferIndex.isValid()) {
        view_delegate->contractItem(transferIndex);
        m_editingIndexes.removeAll(transferIndex);
    } else {
        view_delegate->contractAll();
        m_editingIndexes.clear();
    }
}

void TransfersView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(deselected)
    if (!selected.indexes().isEmpty()) {
        auto *view_delegate = static_cast<TransfersViewDelegate *>(itemDelegate());
        KGet::actionCollection()->action("transfer_show_details")->setChecked(view_delegate->isExtended(selected.indexes().first()));
    }

    QTreeView::selectionChanged(selected, deselected);
}

void TransfersView::closeExpandableDetails(const QModelIndex &parent, int rowStart, int rowEnd)
{
    Q_UNUSED(parent)
    Q_UNUSED(rowStart)
    Q_UNUSED(rowEnd)

    auto *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

    view_delegate->contractAll();
    m_editingIndexes.clear();
}

QWidget *TransfersView::getDetailsWidgetForTransfer(TransferHandler *handler)
{
    auto *groupBox = new QGroupBox(i18n("Transfer Details"));

    auto *layout = new QVBoxLayout(groupBox);
    QWidget *detailsWidget = TransferDetails::detailsWidget(handler);
    layout->addWidget(detailsWidget);

    return groupBox;
}


