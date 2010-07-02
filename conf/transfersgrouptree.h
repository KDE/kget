/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERSGROUPTREE_H
#define TRANSFERSGROUPTREE_H

#include "ui/transfersviewdelegate.h"

#include <QStyledItemDelegate>
#include <QTreeView>

class GroupStatusEditor;

class TransfersGroupDelegate : public BasicTransfersViewDelegate
{
    Q_OBJECT

    public:
        TransfersGroupDelegate(QAbstractItemView *parent);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

class TransfersGroupTree : public QTreeView
{
    Q_OBJECT
    public:
        TransfersGroupTree(QWidget *parent=0);
        void setModel(QAbstractItemModel *model);

    public slots:
        void editCurrent();
        void addGroup();
        void deleteSelectedGroup();
        void renameSelectedGroup();
        void changeIcon(const QString &icon);

    private:
        void rowsInserted(const QModelIndex &index, int start, int end);
};

#endif
