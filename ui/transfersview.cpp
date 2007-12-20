/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfersview.h"
#include "settings.h"

#include <kdebug.h>

#include <QDropEvent>
#include <QHeaderView>

TransfersView::TransfersView(QWidget * parent)
    : QTreeView(parent)
{
    setRootIsDecorated(false);
    setAnimated(true);
    setAllColumnsShowFocus(true);
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setMinimumSectionSize(80);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

TransfersView::~TransfersView()
{
    QList<int>  list;
    for (int i = 0; i<5; i++)
    {
        list.append(columnWidth(i));
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

#include "transfersview.moc"
