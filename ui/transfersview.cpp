/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

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

    populateHeaderActions();
    connect(header(), SIGNAL(customContextMenuRequested(const QPoint &)),
                      SLOT(slotShowHeaderMenu(const QPoint &)));
    connect(this,     SIGNAL(doubleClicked(const QModelIndex &)),
            this,     SLOT(slotItemActivated(const QModelIndex &)));
    connect(this,     SIGNAL(collapsed(const QModelIndex &)),
            this,     SLOT(slotItemCollapsed(const QModelIndex &)));
    connect(KGet::model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), 
            this,          SLOT(closeExpandableDetails(QModelIndex,int,int)));

}

TransfersView::~TransfersView()
{
    QList<int>  list;
    for (int i = 0; i<5; i++)
    {
        int width = columnWidth(i);

        if (Settings::columns().at(i) == 0) {
            width = 90;
        }

        list.append(width);
    }
    Settings::setColumnWidths( list );
    Settings::self()->writeConfig();
}

void TransfersView::setModel(QAbstractItemModel * model)
{
    QTreeView::setModel(model);

    int nGroups = model->rowCount(QModelIndex());

    for(int i = 0; i < nGroups; i++)
    {
        kDebug(5001) << "openEditor for row " << i;
        openPersistentEditor(model->index(i, 1, QModelIndex()));
    }

    QList<int> sizeList = Settings::columnWidths();

    if (!sizeList.isEmpty())
    {
        int j = 0;
        foreach(int i, sizeList)
        {
            setColumnWidth( j, i );
            j++;
        }
    }
    else
    {
        setColumnWidth(0 , 250);
    }

    QList <int> columns = Settings::columns();
    for (int i=0; i<columns.size(); i++) {
        setColumnHidden(i, (columns.at(i) == 1) ? false : true);
    }

    toggleMainGroup();
    connect(model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT (toggleMainGroup()));
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
            openPersistentEditor(model()->index(i, 1, parent));
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

    QList <int> columns = Settings::columns();
    QSignalMapper *columnMapper = new QSignalMapper(this);
    connect(columnMapper, SIGNAL(mapped(int)),
                           SLOT(slotSetColumnVisible(int)));

    for(uint i=0; i<=TransferTreeModel::RemainingTime; i++) {
        KAction *action = new KAction(this);
        action->setText(TransferTreeModel::columnName(i));
        action->setCheckable(true);
        action->setChecked((columns.at(i) == 1) ? true : false);
        m_headerMenu->addAction(action);

        connect(action, SIGNAL(toggled(bool)), columnMapper, SLOT(map()));
        columnMapper->setMapping(action, i);
    }
}

void TransfersView::dragMoveEvent ( QDragMoveEvent * event )
{
    Q_UNUSED(event);

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
    
    if(!item->isGroup() && Settings::showExpandableTransferDetails() && index.column() == 0) {
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
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    closeExpandableDetails(currentIndex());
}

void TransfersView::slotSetColumnVisible(int column)
{
    QList <int> columns = Settings::columns();

    if (columns.size() >= column) {
        columns.replace(column, (columns.at(column) == 1) ? 0 : 1);
    }
    else {
        columns.insert(column, 0);
    }

    setColumnHidden(column, (columns.at(column) == 1) ? false : true);

    Settings::setColumns(columns);
    Settings::self()->writeConfig();
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
    }
    else {
        view_delegate->contractAll();
        m_editingIndexes.clear();
    }
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
