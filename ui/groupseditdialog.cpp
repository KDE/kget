/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QInputDialog>
#include <QMessageBox>

#include "core/kget.h"
#include "groupseditdialog.h"

GroupsEditDialog::GroupsEditDialog(QWidget *parent)
{
    ui.setupUi(this);

    setModal(true);

    setWindowTitle(i18n("Edit your groups"));

    connect(ui.addGroupBt, SIGNAL(clicked()), SLOT(slotAddGroup()));
    connect(ui.deleteGroupBt, SIGNAL(clicked()), SLOT(slotDeleteGroup()));

    KGet::addTransferView(ui.groupList);

    ui.groupList->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Mmmm.. Is sharing the selection model cool? If yes enable this
//     ui.groupList->setSelectionModel((QItemSelectionModel *) KGet::selectionModel());
}

GroupsEditDialog::~GroupsEditDialog()
{

}

void GroupsEditDialog::slotAddGroup()
{
    bool ok = true;
    QString groupName(" ");

    while (ok)
    {
        groupName = QInputDialog::getText(this, i18n("Enter the group name"),
                                          i18n("Group name:"), QLineEdit::Normal,
                                          "", &ok);

        if (groupName.isEmpty())
            QMessageBox::warning(this, i18n("Warning!"),
                                 i18n("Please enter a non empty name!\n"));
        else if(ok)
        {
            if (KGet::addGroup(groupName))
                return;
            else
                QMessageBox::warning(this, i18n("Warning!"),
                                     i18n("A group with that name already exists!\n") +
                                     i18n("Please change the group name"));
        }
    }
}

void GroupsEditDialog::slotDeleteGroup()
{
    QItemSelectionModel * selModel = ui.groupList->selectionModel();
    QAbstractItemModel * dataModel = ui.groupList->model();

    QModelIndexList indexList = selModel->selectedRows();

    foreach(QModelIndex index, indexList)
    {
        QString groupName = dataModel->data(index, Qt::DisplayRole).toString();

        if(groupName == i18n("Default Group"))
        {
            QMessageBox::warning(this, i18n("Warning!"),
                                    i18n("You can't delete the default group!"));
            continue;
        }

        QMessageBox::StandardButton bt;

        bt = QMessageBox::question(this, i18n("Are you sure?"),
                                   i18n("Are you sure that you want to remove\n") +
                                   i18n("the group named ") + groupName + "?",
                                   QMessageBox::Yes | QMessageBox::Yes,
                                   QMessageBox::No);

        if(bt == QMessageBox::Yes)
            KGet::delGroup(groupName);
    }
}


#include "groupseditdialog.moc"
