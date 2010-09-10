/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfersgroupwidget.h"
#include "transfersgrouptree.h"

#include "core/kget.h"
#include "core/transfertreemodel.h"
#include "core/transfertreeselectionmodel.h"


TransfersGroupWidget::TransfersGroupWidget(QWidget *parent) 
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.treeView->setModel(KGet::model());
    ui.treeView->setSelectionModel(KGet::selectionModel());

    ui.treeView->header()->hideSection(TransferTreeModel::Progress);
    ui.treeView->header()->hideSection(TransferTreeModel::RemainingTime);
    ui.treeView->header()->hideSection(TransferTreeModel::Size);
    ui.treeView->header()->hideSection(TransferTreeModel::Speed);

    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.configure->setGuiItem(KStandardGuiItem::Configure);
    ui.rename->setIcon(KIcon("edit-rename"));
    ui.selectIcon->setIcon(KIcon("preferences-desktop-icons"));

    connect(ui.add, SIGNAL(clicked()), ui.treeView, SLOT(addGroup()));
    connect(ui.remove, SIGNAL(clicked()), ui.treeView, SLOT(deleteSelectedGroup()));
    connect(ui.rename, SIGNAL(clicked()), ui.treeView, SLOT(renameSelectedGroup()));
    connect(ui.selectIcon, SIGNAL(iconChanged(QString)), ui.treeView, SLOT(changeIcon(QString)));
    connect(ui.configure, SIGNAL(clicked()), KGet::actionCollection()->action("transfer_group_settings"), SLOT(trigger()));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));

    slotSelectionChanged();
}

void TransfersGroupWidget::slotSelectionChanged()
{
    const QModelIndexList selectedGroups = ui.treeView->selectionModel()->selectedRows();
    const bool somethingSelected = !selectedGroups.isEmpty();
    bool canDelete = somethingSelected;

    foreach (const QModelIndex &index, selectedGroups) {
        if (index.row() == 0) {
            canDelete = false;
            break;
        }
    }

    ui.rename->setEnabled(canDelete);
    ui.remove->setEnabled(canDelete);
    ui.configure->setEnabled(somethingSelected);
    ui.selectIcon->setEnabled(somethingSelected);

    if (somethingSelected && !KGet::selectedTransferGroups().isEmpty()) {
        ui.selectIcon->setIcon(KIcon(KGet::selectedTransferGroups().first()->iconName()));
    } else {
        ui.selectIcon->setIcon(KIcon("preferences-desktop-icons"));
    }
}
