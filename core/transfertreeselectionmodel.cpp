/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "core/transfertreeselectionmodel.h"

#include "core/transfertreemodel.h"

#include <kdebug.h>

TransferTreeSelectionModel::TransferTreeSelectionModel(QAbstractItemModel * model)
    : QItemSelectionModel(model)
{
}

TransferTreeSelectionModel::~TransferTreeSelectionModel()
{

}

void TransferTreeSelectionModel::select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command)
{
//     kDebug(5001) << "TransferTreeSelectionModel::select()" << endl;

//     const TransferTreeModel * transfersModel = static_cast<const TransferTreeModel *>(model());

//     QModelIndexList indexList = selection.indexes();
//     QItemSelection newSelection;

//     kDebug(5001) << "selection of items: " << indexList.size() << endl;

//     foreach(QModelIndex index, indexList)
//     {
//         kDebug(5001) << "iteration" << endl;

//         if(!transfersModel->isTransferGroup(index))
//         {

//             newSelection.select(index, index);
//         }
//     }

//     QItemSelectionModel::select(newSelection, command);

    QItemSelectionModel::select(selection, command);
}


#include "transfertreeselectionmodel.moc"
