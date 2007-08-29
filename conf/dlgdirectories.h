/* This file is part of the KDE project

   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DLGDIRECTORIES_H
#define DLGDIRECTORIES_H

#include <QWidget>

#include "ui_dlgdirectories.h"

class QItemSelection;

class DlgDirectories : public QWidget, public Ui::DlgDirectories
{
    Q_OBJECT

public:
    DlgDirectories(QWidget *parent = 0);
    ~DlgDirectories();

private Q_SLOTS:
    void readConfig();
    void saveSettings();
    void addButtonClicked();
    void removeButtonClicked();
    void changeButtonClicked();
    void listItemClicked(const QItemSelection &selected, const QItemSelection &deselected);
    void addFolderForExtensionItem(const QString &extension, const QString &folder);
    void slotExtensionDataChanged(int row, int column);
};

#endif
