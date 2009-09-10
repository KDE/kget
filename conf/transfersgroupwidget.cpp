/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

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
#include <QAction>
#include <QCheckBox>

TransfersGroupDelegate::TransfersGroupDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{

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
    setItemDelegate(new TransfersGroupDelegate(this));

    setRootIsDecorated(false);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    KGet::addTransferView(this);

    // hide the size, speed, percent and status columns
    header()->hideSection(1);
    header()->hideSection(2);
    header()->hideSection(3);
    header()->hideSection(4);
    header()->hideSection(5);

    setItemsExpandable(false);
    setSelectionModel((QItemSelectionModel *) KGet::selectionModel());
}

void TransfersGroupTree::commitData(QWidget *editor)
{
    GroupEditor * groupEditor = static_cast<GroupEditor *>(editor);

    if(groupEditor->groupIndex() != currentIndex())
        return;

    const QString newName = groupEditor->text();
    if (!newName.isEmpty())
    {
        foreach(const QString &groupName, KGet::transferGroupNames())
        {
            if(groupName == newName &&
               groupName != model()->data(currentIndex(), Qt::DisplayRole).toString())
            {
                KMessageBox::error( this, i18n("Another group with this name already exists. Please select a different name."), i18n("Group Name Already in Use") );
                QTimer::singleShot( 0, this, SLOT(editCurrent()) );
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
    QList<TransferGroupHandler*> list = KGet::selectedTransferGroups();
    QStringList names;
    foreach (TransferGroupHandler * handler, list)
        names << handler->name();

    if (!list.isEmpty())
    {
        bool del = false;
        if (list.count() == 1) {
            del = KMessageBox::warningYesNo(this,
                  i18n("Are you sure that you want to remove the group named %1?", list.first()->name()),
                  i18n("Remove Group"),
                  KStandardGuiItem::remove(), KStandardGuiItem::cancel()) == KMessageBox::Yes;
        } else {
            del = KMessageBox::warningYesNoList(this,
                  i18n("Are you sure that you want to remove the following groups?"),
                  names,
                  i18n("Remove groups"),
                  KStandardGuiItem::remove(), KStandardGuiItem::cancel()) == KMessageBox::Yes;
        }
        if (del) {
            foreach (TransferGroupHandler * handler, list)
                KGet::delGroup(handler->name());
        }
    }
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


TransfersGroupWidget::TransfersGroupWidget(QWidget *parent) 
    : QVBoxLayout()
{
    QCheckBox * m_directoriesAsSuggestionCheck = new QCheckBox(i18n("Use default directories for groups as suggestion"), parent);
    m_directoriesAsSuggestionCheck->setObjectName("kcfg_DirectoriesAsSuggestion");

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
    iconButton->setText(i18n("Select Icon..."));
    iconButton->setIcon(KIcon("preferences-desktop-icons"));
    configureButton = new QPushButton(i18n("Configure..."));
    configureButton->setIcon(KGet::actionCollection()->action("transfer_group_settings")->icon());

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(addButton);
    buttonsLayout->addWidget(renameButton);
    buttonsLayout->addWidget(deleteButton);
    buttonsLayout->addWidget(iconButton);
    buttonsLayout->addWidget(configureButton);

    addWidget(m_directoriesAsSuggestionCheck);
    addWidget(m_view);
    addLayout(buttonsLayout);

    connect(addButton, SIGNAL(clicked()), m_view, SLOT(addGroup()));
    connect(deleteButton, SIGNAL(clicked()), m_view, SLOT(deleteSelectedGroup()));
    connect(renameButton, SIGNAL(clicked()), m_view, SLOT(renameSelectedGroup()));
    connect(iconButton, SIGNAL(iconChanged(const QString &)), m_view, SLOT(changeIcon(const QString &)));
    connect(configureButton, SIGNAL(clicked()), KGet::actionCollection()->action("transfer_group_settings"), SLOT(trigger()));
    connect(m_view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                this, SLOT(slotSelectionChanged()));
}

void TransfersGroupWidget::slotSelectionChanged()
{
    QModelIndexList selectedGroups = m_view->selectionModel()->selectedRows();
    bool somethingSelected = !selectedGroups.isEmpty();
    bool canDelete = somethingSelected;

    foreach(const QModelIndex &index, selectedGroups) {
        if(index.row() == 0) {
            canDelete = false;
            break;
        }
    }

    renameButton->setEnabled(canDelete);
    deleteButton->setEnabled(canDelete);
    configureButton->setEnabled(somethingSelected);
    iconButton->setEnabled(somethingSelected);
    if (somethingSelected && !KGet::selectedTransferGroups().isEmpty())
        iconButton->setIcon(KIcon(KGet::selectedTransferGroups().first()->iconName()));
    else
        iconButton->setIcon(KIcon("preferences-desktop-icons"));
}
