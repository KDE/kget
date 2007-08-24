/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "transfersview.h"

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

    setColumnWidth(0, 250);
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
}

#include "transfersview.moc"
