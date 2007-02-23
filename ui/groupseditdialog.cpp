/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QListView>
#include <QHBoxLayout>

#include <KInputDialog>
#include <KMessageBox>
#include <KPushButton>
#include <KLocale>

#include "core/kget.h"
#include "groupseditdialog.h"

GroupsEditDialog::GroupsEditDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Edit Groups"));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *buttonLayout = new QVBoxLayout;

    KPushButton *addGroupBt = new KPushButton(i18n("Add Group"));
    addGroupBt->setIcon(KIcon("add"));
    connect(addGroupBt, SIGNAL(clicked()), SLOT(slotAddGroup()));
    buttonLayout->addWidget(addGroupBt);

    KPushButton *deleteGroupBt = new KPushButton(i18n("Delete Group"));
    deleteGroupBt->setIcon(KIcon("remove"));
    connect(deleteGroupBt, SIGNAL(clicked()), SLOT(slotDeleteGroup()));
    buttonLayout->addWidget(deleteGroupBt);

    buttonLayout->addStretch();

    groupList = new QListView();
    mainLayout->addWidget(groupList);
    mainLayout->addLayout(buttonLayout);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setMainWidget(mainWidget);

    KGet::addTransferView(groupList);

    groupList->setSelectionBehavior(QAbstractItemView::SelectRows);
    groupList->setSelectionModel((QItemSelectionModel *) KGet::selectionModel());
}

void GroupsEditDialog::slotAddGroup()
{
    bool ok = true;
    QString groupName;

    while (ok)
    {
        groupName = KInputDialog::getText(i18n("Enter the group name"),
                                          i18n("Group name:"), QString(), &ok, this);

        if(ok)
        {
            if (KGet::addGroup(groupName))
                return;
            else
                KMessageBox::sorry(this,
                                   i18n("A group with that name already exists!\n"
                                        "Please change the group name."));
        }
    }
}

void GroupsEditDialog::slotDeleteGroup()
{
    QItemSelectionModel * selModel = groupList->selectionModel();
    QAbstractItemModel * dataModel = groupList->model();

    QModelIndexList indexList = selModel->selectedRows();

    foreach(QModelIndex index, indexList)
    {
        QString groupName = dataModel->data(index, Qt::DisplayRole).toString();

        if(groupName == i18n("Default Group"))
        {
            KMessageBox::sorry(this, i18n("You can't delete the default group!"));
            continue;
        }

        if(KMessageBox::questionYesNoCancel(this,
                                            i18n("Are you sure that you want to remove\n"
                                                 "the group named %1?", groupName))
            == KMessageBox::Yes)
            KGet::delGroup(groupName);
    }
}


#include "groupseditdialog.moc"
