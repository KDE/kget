/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "transfersgroupwidget.h"

#include "core/kget.h"

#include <KInputDialog>
#include <KMessageBox>
#include <KLocale>
#include <KDebug>

#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QHeaderView>
#include <QStandardItem>
#include <QLabel>
#include <QLineEdit>

TransfersGroupTree::TransfersGroupTree(QWidget *parent)
    : QTreeView(parent)
{
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
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);

    if(QString::compare(lineEdit->text(), QString("")) != 0) { 
        QModelIndex currentIndex = selectionModel()->currentIndex();

        KGet::renameGroup(model()->data(currentIndex).toString(), lineEdit->text());
    }
}

void TransfersGroupTree::addGroup()
{
    QString groupName;

    if (KGet::addGroup(groupName)) {
        QModelIndex index = model()->index(model()->rowCount() - 1, 0); // the last item added to the model
        edit(index);
        return;
    }
}

void TransfersGroupTree::openEditMode()
{
    QItemSelectionModel *selModel = selectionModel();

    QModelIndexList indexList = selModel->selectedRows();

    foreach(QModelIndex index, indexList)
    {
        edit(index);
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


TransfersGroupWidget::TransfersGroupWidget(QWidget *parent) 
    : QVBoxLayout() 
{
    m_view = new TransfersGroupTree(parent);

    addButton = new QPushButton(i18n("Add"));
    addButton->setIcon(KIcon("list-add"));
    deleteButton = new QPushButton(i18n("Delete"));
    deleteButton->setIcon(KIcon("list-remove"));
    deleteButton->setEnabled(false);
    renameButton = new QPushButton(i18n("Edit"));
    renameButton->setIcon(KIcon("editinput"));
    renameButton->setEnabled(false);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(renameButton);
    buttonsLayout->addWidget(deleteButton);

    addWidget(m_view);
    addLayout(buttonsLayout);

    connect(addButton, SIGNAL(clicked()), m_view, SLOT(addGroup()));
    connect(deleteButton, SIGNAL(clicked()), m_view, SLOT(deleteSelectedGroup()));
    connect(renameButton, SIGNAL(clicked()), m_view, SLOT(openEditMode()));
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
