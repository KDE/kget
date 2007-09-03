/* This file is part of the KDE project

   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgdirectories.h"
#include "selectdirectoryitemdelegate.h"

#include "settings.h"

#include <QHeaderView>

#include <KMessageBox>

DlgDirectories::DlgDirectories(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    addButton->setIcon(KIcon("list-add"));
    removeButton->setIcon(KIcon("list-remove"));
    changeButton->setIcon(KIcon("edit-redo"));

    defaultFolderRequester->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    folderForExtensionList->verticalHeader()->setVisible(false);
    folderForExtensionList->horizontalHeader()->setClickable(false);
    folderForExtensionList->horizontalHeader()->setMovable(false);
    folderForExtensionList->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    folderForExtensionList->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    folderForExtensionList->setItemDelegateForColumn(1, new SelectDirectoryItemDelegate(parent));


    connect(addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(removeButtonClicked()));
    connect(changeButton, SIGNAL(clicked()), SLOT(changeButtonClicked()));
    connect(kcfg_EnableExceptions, SIGNAL(toggled(bool)), defaultFolderGroupBox, SLOT(setEnabled(bool)));
    connect(kcfg_UseDefaultDirectory, SIGNAL(toggled(bool)), defaultFolderRequester, SLOT(setEnabled(bool)));
    connect(folderForExtensionList->selectionModel(),
                        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection&)),
                        SLOT(listItemClicked(const QItemSelection &, const QItemSelection&)));
    connect(folderForExtensionList, SIGNAL(cellChanged(int, int)), SLOT(slotExtensionDataChanged(int, int)));

    readConfig();
}

DlgDirectories::~DlgDirectories()
{
}

void DlgDirectories::readConfig()
{
    QStringList list = Settings::extensionsFolderList();
    QStringList::Iterator it = list.begin();
    QStringList::Iterator end = list.end();
    while (it != end) {
        // odd list items are regular expressions for extensions
        QString rexp = *it;
        ++it;
        QString path = *it;
        ++it;
        addFolderForExtensionItem(rexp, path);
    }

    if (Settings::useDefaultDirectory())
        kcfg_UseDefaultDirectory->click();
    else
        askRadioButton->click();

    defaultFolderRequester->setUrl(Settings::defaultDirectory());
    connect(defaultFolderRequester, SIGNAL(textChanged(QString)), SLOT(saveSettings()));
}

void DlgDirectories::saveSettings()
{
    Settings::setDefaultDirectory(defaultFolderRequester->url().url());

    QStringList list;

    for(int row=0;row<folderForExtensionList->rowCount();row++) {
        QString extension = folderForExtensionList->item(row, 0)->text();
        QString path = folderForExtensionList->item(row, 1)->text();

        if(QString::compare(QString(""), extension) != 0 &&
                    QString::compare(QString(""), path) != 0) {

            list.append(extension);
            list.append(path);
        }
    }

    Settings::setExtensionsFolderList(list);
    Settings::self()->writeConfig();
}

void DlgDirectories::addButtonClicked()
{
    int newRow = folderForExtensionList->rowCount();
    folderForExtensionList->setRowCount(newRow + 1);

    folderForExtensionList->setItem(newRow, 0, new QTableWidgetItem("*.*"));
    folderForExtensionList->setItem(newRow, 1, new QTableWidgetItem(""));

    folderForExtensionList->edit(folderForExtensionList->model()->index(newRow, 0));
    folderForExtensionList->selectRow(newRow);
}

void DlgDirectories::removeButtonClicked()
{
    int row = folderForExtensionList->currentRow();
    folderForExtensionList->removeRow(row);
    saveSettings();

    folderForExtensionList->clearSelection();
    changeButton->setEnabled(false);
    removeButton->setEnabled(false);
}

void DlgDirectories::changeButtonClicked()
{
    int row = folderForExtensionList->currentRow();
    int column = folderForExtensionList->currentColumn();
    folderForExtensionList->edit(folderForExtensionList->model()->index(row, column));
}

void DlgDirectories::listItemClicked(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    changeButton->setEnabled(true);
    removeButton->setEnabled(true);
}

void DlgDirectories::addFolderForExtensionItem(const QString &extension, const QString &folder)
{
    if (extension.isEmpty() || folder.isEmpty()) {
        KMessageBox::error(this, i18n("Folder and extension can not be empty."), i18n("Error"));
        return;
    }

    int newRow = folderForExtensionList->rowCount();
    folderForExtensionList->setRowCount(newRow + 1);

    folderForExtensionList->setItem(newRow, 0, new QTableWidgetItem(extension));
    folderForExtensionList->setItem(newRow, 1, new QTableWidgetItem(folder));

    folderForExtensionList->clearSelection();
    folderForExtensionList->sortItems(0, Qt::AscendingOrder);

    changeButton->setEnabled(false);
}

void DlgDirectories::slotExtensionDataChanged(int row, int column)
{
    Q_UNUSED(column)

    if(folderForExtensionList->item(row, 0) && folderForExtensionList->item(row, 1)) {
        // we check if the extension isn't empty and the path is correct before call saveSettings
        QString extension = folderForExtensionList->item(row, 0)->text();
        QString path = folderForExtensionList->item(row, 1)->text();

        if(QString::compare(QString(""), extension) != 0 && QString::compare(QString(""), path) != 0) {
            saveSettings();
        }
    }
}

#include "dlgdirectories.moc"
