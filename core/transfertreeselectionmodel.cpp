/* This file is part of the KDE project

   Copyright (C) 2006 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
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

#include "transfertreeselectionmodel.moc"
