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

#include <KDebug>
#include <KAction>
#include <KLocale>
#include <KMenu>
#include <KRun>

#include <QDropEvent>
#include <QHeaderView>
#include <QSignalMapper>
#include <QHBoxLayout>
#include <QGroupBox>

TransfersView::TransfersView(QWidget * parent)
    : QTreeView(parent),
        m_headerMenu(0)
{
//     setItemsExpandable(false);
    setRootIsDecorated(false);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setMinimumSectionSize(80);    
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setClickable(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(header(), SIGNAL(customContextMenuRequested(QPoint)),
                      SLOT(slotShowHeaderMenu(QPoint)));
    connect(header(), SIGNAL(sectionMoved(int,int,int)), this, SLOT(slotSectionMoved(int,int,int)));
    connect(header(), SIGNAL(sectionResized(int,int,int)), this, SLOT(slotSaveHeader()));
    connect(this,     SIGNAL(doubleClicked(QModelIndex)),
            this,     SLOT(slotItemActivated(QModelIndex)));
    connect(this,     SIGNAL(collapsed(QModelIndex)),
            this,     SLOT(slotItemCollapsed(QModelIndex)));
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
        kDebug(5001) << "openEditor for row " << i;
        openPersistentEditor(model->index(i, TransferTreeModel::Status, QModelIndex()));
    }

    QByteArray loadedState = QByteArray::fromBase64(Settings::headerState().toAscii());
    if (loadedState.isEmpty()) {
        setColumnWidth(0 , 230);
    } else {
        header()->restoreState(loadedState);
    }

    //Workaround if the saved headerState is corrupted
    header()->setRootIndex(QModelIndex());

    populateHeaderActions();
    toggleMainGroup();
    connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT (toggleMainGroup()));
}

void TransfersView::dropEvent(QDropEvent * event)
{
    QModelIndex dropIndex = indexAt(event->pos());
    QTreeView::dropEvent(event);

    setExpanded(dropIndex, true);
}

void TransfersView::rowsInserted(const QModelIndex & parent, int start, int end)
{
    kDebug(5001) << "TransfersView::rowsInserted";

    if(!parent.isValid())
    {
        kDebug(5001) << "parent is not valid " << start << "  " << end;

        for(int i = start; i <= end; i++)
        {
            kDebug(5001) << "openEditor for row " << i;
            openPersistentEditor(model()->index(i, TransferTreeModel::Status, parent));
        }
    }

    QTreeView::rowsInserted(parent, start, end);

    setExpanded(parent, true);
    toggleMainGroup();
}

void TransfersView::populateHeaderActions()
{
    m_headerMenu = new KMenu(header());
    m_headerMenu->addTitle(i18n("Select columns"));

    QSignalMapper *columnMapper = new QSignalMapper(this);
    connect(columnMapper, SIGNAL(mapped(int)), SLOT(slotHideSection(int)));

    //Create for each column an action with the column-header as name
    QVector<KAction*> orderedMenuItems(header()->count());
    for (int i = 0; i < header()->count(); ++i) {
        KAction *action = new KAction(this);
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
    Settings::self()->writeConfig();
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
    TransfersViewDelegate *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

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
    TransfersViewDelegate *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

    if(!item)
        return;
    
    if(item->isGroup()) {
        TransferGroupHandler * groupHandler = item->asGroup()->groupHandler();
        QList<TransferHandler *> transfers = groupHandler->transfers();

        foreach(TransferHandler * transfer, transfers) {
            kDebug(5001) << "Transfer = " << transfer->source().prettyUrl(); 
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
    TransfersViewDelegate *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());
    
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
        TransfersViewDelegate *view_delegate = static_cast<TransfersViewDelegate *>(itemDelegate());
        KGet::actionCollection()->action("transfer_show_details")->setChecked(view_delegate->isExtended(selected.indexes().first()));
    }

    QTreeView::selectionChanged(selected, deselected);
}

void TransfersView::closeExpandableDetails(const QModelIndex &parent, int rowStart, int rowEnd)
{
    Q_UNUSED(parent)
    Q_UNUSED(rowStart)
    Q_UNUSED(rowEnd)

    TransfersViewDelegate *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

    view_delegate->contractAll();
    m_editingIndexes.clear();
}

QWidget *TransfersView::getDetailsWidgetForTransfer(TransferHandler *handler)
{
    QGroupBox *groupBox = new QGroupBox(i18n("Transfer Details"));

    QVBoxLayout *layout = new QVBoxLayout(groupBox);
    QWidget *detailsWidget = TransferDetails::detailsWidget(handler);
    layout->addWidget(detailsWidget);

    return groupBox;
}


#include "transfersview.moc"
