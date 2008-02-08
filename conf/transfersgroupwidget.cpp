/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfersgroupwidget.h"

#include "core/kget.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreeselectionmodel.h"

#include <KMessageBox>
#include <KIconButton>

#include <QTreeView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QTimer>

TransfersGroupDelegate::TransfersGroupDelegate(QObject * parent)
    : QItemDelegate(parent)
{

}

void TransfersGroupDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QItemDelegate::updateEditorGeometry(editor, option, index);
    editor->setGeometry(editor->geometry().adjusted(14,0,0,0));
}

QWidget * TransfersGroupDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                                                const QModelIndex & index) const
{
    Q_UNUSED(option);
    return new GroupEditor(index, index.model()->data(index, Qt::DisplayRole).toString(), parent);
}

TransfersGroupTree::TransfersGroupTree(QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegate(new TransfersGroupDelegate());

    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);

    KGet::addTransferView(this);

    // hide the size, speed, percent and status columns
    header()->hideSection(1);
    header()->hideSection(2);
    header()->hideSection(3);
    header()->hideSection(4);

    setItemsExpandable(false);
    setSelectionModel((QItemSelectionModel *) KGet::selectionModel());
}

void TransfersGroupTree::commitData(QWidget *editor)
{
    GroupEditor * groupEditor = static_cast<GroupEditor *>(editor);

    if(groupEditor->groupIndex() != currentIndex())
        return;

    if (groupEditor->text().isEmpty())
    {
        KMessageBox::error( this, i18n("The group name is empty"), i18n("A group can not have an empty name\nPlease select a new one") );
        QTimer::singleShot( 0, this, SLOT(editCurrent()) );
        return;
    }
    else 
    {
        foreach(QString groupName, KGet::transferGroupNames())
        {
            if(groupName == groupEditor->text() && 
               groupName != ((TransferGroupHandler *) currentIndex().internalPointer())->name() )
            {
                KMessageBox::error( this, i18n("Group name already in use"), i18n("Another group with this name already exists\nPlease select a different name") );
                QTimer::singleShot( 0, this, SLOT(editCurrent()) );
                return;
            }
        }

        KGet::renameGroup(model()->data(currentIndex()).toString(), groupEditor->text());
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

void TransfersGroupTree::openEditMode()
{
    QItemSelectionModel *selModel = selectionModel();

    QModelIndexList indexList = selModel->selectedRows();

    foreach(QModelIndex index, indexList)
    {
        editCurrent();
    }
}

void TransfersGroupTree::deleteSelectedGroup()
{
    QItemSelectionModel *selModel = selectionModel();
    QAbstractItemModel *dataModel = model();

    QModelIndexList indexList = selModel->selectedRows();

    foreach(QModelIndex index, indexList)
    {
        QString groupName = dataModel->data(index, Qt::DisplayRole).toString();

        if(groupName == i18n("My Downloads"))
        {
            KMessageBox::sorry(this, i18n("You can not delete this group!"));
            continue;
        }

        if(KMessageBox::questionYesNoCancel(this,
                                            i18n("Are you sure that you want to remove\n"
                                                 "the group named %1?", groupName)) == KMessageBox::Yes)
            KGet::delGroup(groupName);
    }
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


TransfersGroupWidget::TransfersGroupWidget(QWidget *parent) 
    : QVBoxLayout()
{
    m_view = new TransfersGroupTree(parent);

    addButton = new QPushButton(i18n("Add"));
    addButton->setIcon(KIcon("list-add"));
    deleteButton = new QPushButton(i18n("Delete"));
    deleteButton->setIcon(KIcon("list-remove"));
    deleteButton->setEnabled(false);
    renameButton = new QPushButton(i18n("Rename"));
    renameButton->setIcon(KIcon("edit-rename"));
    renameButton->setEnabled(false);
    iconButton = new KIconButton(dynamic_cast<QWidget*>(this));
    iconButton->setIconSize(32);
    iconButton->setButtonIconSize(16);
    iconButton->setText(i18n("Select Icon"));
    iconButton->setIcon(KIcon("preferences-desktop-icons"));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(renameButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addWidget(iconButton);

    addWidget(m_view);
    addLayout(buttonsLayout);

    connect(addButton, SIGNAL(clicked()), m_view, SLOT(addGroup()));
    connect(deleteButton, SIGNAL(clicked()), m_view, SLOT(deleteSelectedGroup()));
    connect(renameButton, SIGNAL(clicked()), m_view, SLOT(openEditMode()));
    connect(iconButton, SIGNAL(iconChanged(const QString &)), m_view, SLOT(changeIcon(const QString &)));
    connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));
}

void TransfersGroupWidget::slotSelectionChanged(const QItemSelection &newSelection, 
                    const QItemSelection &oldSelection)
{
    Q_UNUSED(oldSelection)

    bool canDelete = true;

    foreach(QModelIndex index, newSelection.indexes()) {
        if(index.row() == 0) {
            canDelete = false;
        }
    }

    renameButton->setEnabled(canDelete);
    deleteButton->setEnabled(canDelete);
}
