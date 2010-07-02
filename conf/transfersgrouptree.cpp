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

#include "transfersgrouptree.h"

#include "core/kget.h"
#include "core/transfertreemodel.h"
#include "core/transfertreeselectionmodel.h"

#include <QtGui/QHeaderView>

#include <KLineEdit>

TransfersGroupDelegate::TransfersGroupDelegate(QAbstractItemView *parent)
  : BasicTransfersViewDelegate(parent)
{
}

QWidget *TransfersGroupDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == TransferTreeModel::Name) {
        return new KLineEdit(parent);
    } else {
        return BasicTransfersViewDelegate::createEditor(parent, option, index);
    }
}

void TransfersGroupDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == TransferTreeModel::Name) {
        KLineEdit *groupEditor = static_cast<KLineEdit*>(editor);
        groupEditor->setText(index.data().toString());
    } else {
        BasicTransfersViewDelegate::setEditorData(editor, index);
    }
}

void TransfersGroupDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == TransferTreeModel::Name) {
        KLineEdit *groupEditor = static_cast<KLineEdit*>(editor);
        const QString newName = groupEditor->text();
        const QString oldName = index.data().toString();

        if (!newName.isEmpty()) {
            foreach (const QString &groupName, KGet::transferGroupNames()) {
                if (groupName == newName && groupName != oldName) {
                    groupEditor->setText(oldName);
                    return;
                }
            }

            KGet::renameGroup(oldName, newName);
        }
    } else {
        BasicTransfersViewDelegate::setModelData(editor, model, index);
    }
}


TransfersGroupTree::TransfersGroupTree(QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegate(new TransfersGroupDelegate(this));
}

void TransfersGroupTree::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);

    int nGroups = model->rowCount(QModelIndex());
    for (int i = 0; i < nGroups; i++) {
        kDebug(5001) << "openEditor for row " << i;
        openPersistentEditor(model->index(i, TransferTreeModel::Status, QModelIndex()));
    }

    setColumnWidth(0 , 250);
}

void TransfersGroupTree::rowsInserted(const QModelIndex &parent, int start, int end)
{
    if (!parent.isValid()) {
        for (int i = start; i <= end; ++i) {
            kDebug(5001) << "openEditor for row " << i;
            openPersistentEditor(model()->index(i, TransferTreeModel::Status, parent));
        }
    }

    QTreeView::rowsInserted(parent, start, end);
}

void TransfersGroupTree::editCurrent()
{
    QTreeView::edit(currentIndex());
}

void TransfersGroupTree::addGroup()
{
    QString groupName(i18n("New Group"));
    int i=0;

    while(KGet::transferGroupNames().contains(groupName))
    {
        groupName = i18n("New Group") + QString::number(++i);
    }

    if (KGet::addGroup(groupName)) {
        QModelIndex index = model()->index(model()->rowCount() - 1, 0);
        setCurrentIndex(index);
        editCurrent();
    }
}

void TransfersGroupTree::deleteSelectedGroup()
{
    KGet::delGroups(KGet::selectedTransferGroups());
}

void TransfersGroupTree::renameSelectedGroup()
{
    if(currentIndex().isValid())
        editCurrent();
}

void TransfersGroupTree::changeIcon(const QString &icon)
{
    kDebug(5001);
    TransferTreeSelectionModel *selModel = KGet::selectionModel();

    QModelIndexList indexList = selModel->selectedRows();

    if (!icon.isEmpty())
    {
        foreach (TransferGroupHandler *group, KGet::selectedTransferGroups())
        {
            group->setIconName(icon);
        }
    }
    emit dataChanged(indexList.first(),indexList.last());
}
