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
#include "core/transfertreemodel.h"

#include <kdebug.h>
#include <KAction>

#include <QDropEvent>
#include <QHeaderView>
#include <QSignalMapper>

TransfersView::TransfersView(QWidget * parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setMinimumSectionSize(80);    
    header()->setContextMenuPolicy(Qt::ActionsContextMenu);
    header()->setClickable(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    populateHeaderActions();
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

QModelIndex TransfersView::indexFromTransferHandler(TransferHandler *handler)
{
    for(int groupRow = 0; groupRow < model()->rowCount(); groupRow ++) {
        QModelIndex groupIndex = model()->index(groupRow, 0, QModelIndex());
        for(int transferRow = 0; transferRow < model()->rowCount(groupIndex); transferRow ++) {
            QModelIndex index = model()->index(transferRow, 0, groupIndex);

            TransferHandler *indexHandler = static_cast <TransferHandler *> (index.internalPointer());
            if(indexHandler == handler) {
                return index;
            }
        }
    }
    return QModelIndex();
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
    QList <int> columns = Settings::columns();
    QSignalMapper *columnMapper = new QSignalMapper(this);
    connect(columnMapper, SIGNAL(mapped(int)),
                           SLOT(slotSetColumnVisible(int)));

    for(uint i=0; i<=TransferTreeModel::RemainingTime; i++) {
        KAction *action = new KAction(header());
        action->setText(TransferTreeModel::columnName(i));
        action->setCheckable(true);
        action->setChecked((columns.at(i) == 1) ? true : false);
        header()->addAction(action);

        connect(action, SIGNAL(toggled(bool)), columnMapper, SLOT(map()));
        columnMapper->setMapping(action, i);
    }
}

void TransfersView::dragMoveEvent ( QDragMoveEvent * event )
{
    Q_UNUSED(event);
    TransfersViewDelegate *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());

    view_delegate->closeExpandableDetails();
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
}

void TransfersView::rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    TransfersViewDelegate *view_delegate = static_cast <TransfersViewDelegate *> (itemDelegate());
    view_delegate->closeExpandableDetails(currentIndex());
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

#include "transfersview.moc"
