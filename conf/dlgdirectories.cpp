/* This file is part of the KDE project

   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgdirectories.h"

#include "settings.h"

#include <KMessageBox>

DlgDirectories::DlgDirectories(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    addButton->setIcon(KIcon("list-add"));
    removeButton->setIcon(KIcon("list-remove"));
    changeButton->setIcon(KIcon("edit-redo"));

    defaultFolderRequester->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    folderForExtensionRequester->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

    connect(addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(removeButtonClicked()));
    connect(changeButton, SIGNAL(clicked()), SLOT(changeButtonClicked()));
    connect(askRadioButton, SIGNAL(clicked()), SLOT(radioButtonClicked()));
    connect(kcfg_UseDefaultDirectory, SIGNAL(clicked()), SLOT(radioButtonClicked()));
    connect(folderForExtensionList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(listItemClicked(QTreeWidgetItem*)));

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
    QTreeWidgetItemIterator it(folderForExtensionList);
    while (*it) {
        list.append((*it)->text(0));
        list.append((*it)->text(1));
        ++it;
    }
    Settings::setExtensionsFolderList(list);

    Settings::writeConfig();
}

void DlgDirectories::addButtonClicked()
{
    addFolderForExtensionItem(extensionLineEdit->text(), folderForExtensionRequester->url().url());
    extensionLineEdit->clear();
    folderForExtensionRequester->clear();
    extensionLineEdit->setFocus();

    saveSettings();
}

void DlgDirectories::removeButtonClicked()
{
    QList<QTreeWidgetItem *> selectedItems = folderForExtensionList->selectedItems();

    foreach(QTreeWidgetItem *selectedItem, selectedItems)
        delete(selectedItem);

    saveSettings();
}

void DlgDirectories::changeButtonClicked()
{
// it works this way, but it could be improved with really update item; not just remove and add the item
    removeButtonClicked();

    addButtonClicked();
}

void DlgDirectories::radioButtonClicked()
{
    defaultFolderGroupBox->setEnabled(kcfg_UseDefaultDirectory->isChecked());
}

void DlgDirectories::listItemClicked(QTreeWidgetItem *item)
{
    extensionLineEdit->setText(item->text(0));
    folderForExtensionRequester->setUrl(item->text(1));
    extensionLineEdit->setFocus();
    changeButton->setEnabled(true);
}

void DlgDirectories::addFolderForExtensionItem(const QString &extension, const QString &folder)
{
    if (extension.isEmpty() || folder.isEmpty()) {
        KMessageBox::error(this, i18n("Folder and extension can not be empty."), i18n("Error"));
        return;
    }

    folderForExtensionList->addTopLevelItem(new QTreeWidgetItem(QStringList() << extension << folder));
    folderForExtensionList->clearSelection();
    folderForExtensionList->sortItems(0, Qt::AscendingOrder);

    changeButton->setEnabled(false);
}

#include "dlgdirectories.moc"
