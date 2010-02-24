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
#include "core/transfertreeselectionmodel.h"

#include <QtGui/QHeaderView>

TransfersGroupDelegate::TransfersGroupDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{

}

QWidget * TransfersGroupDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                                                const QModelIndex & index) const
{
    Q_UNUSED(option)
    return new GroupEditor(index, index.data(Qt::DisplayRole).toString(), parent);
}

TransfersGroupTree::TransfersGroupTree(QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegate(new TransfersGroupDelegate(this));
}

void TransfersGroupTree::commitData(QWidget *editor)
{
    GroupEditor * groupEditor = static_cast<GroupEditor *>(editor);

    if (groupEditor->groupIndex() != currentIndex()) {
        return;
    }

    const QString newName = groupEditor->text();
    if (!newName.isEmpty()){
        foreach (const QString &groupName, KGet::transferGroupNames()) {
            if (groupName == newName &&
                groupName != model()->data(currentIndex(), Qt::DisplayRole).toString()) {
                return;
            }
        }

        KGet::renameGroup(model()->data(currentIndex()).toString(), newName);
        setCurrentIndex(QModelIndex());
    }
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
