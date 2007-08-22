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
    bool ok = true;
    QString groupName;

    while (ok)
    {
        groupName = KInputDialog::getText(i18n("Enter Group Name"),
                                          i18n("Group name:"), QString(), &ok, this);

        if(ok)
        {
            if (KGet::addGroup(groupName)) {
                return;
            }
            else {
                KMessageBox::sorry(this,
                                   i18n("A group with that name already exists!\n"
                                        "Please change the group name."));
            }
        }
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

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(deleteButton);

    addWidget(m_view);
    addLayout(buttonsLayout);

    connect(addButton, SIGNAL(clicked()), m_view, SLOT(addGroup()));
    connect(deleteButton, SIGNAL(clicked()), m_view, SLOT(deleteSelectedGroup()));
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

    deleteButton->setEnabled(canDelete);
}

