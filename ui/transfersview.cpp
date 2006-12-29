/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "transfersview.h"

TransfersView::TransfersView(QWidget * parent)
    : QTreeView(parent)
{
//     setRootIsDecorated(false);
//     setAnimated(true);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
}

TransfersView::~TransfersView()
{

}

void TransfersView::setModel(QAbstractItemModel * model)
{
    QTreeView::setModel(model);

    setColumnWidth(0, 250);
}

#include "transfersview.moc"
