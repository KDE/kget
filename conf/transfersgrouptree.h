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

#include <QStyledItemDelegate>
#include <QTreeView>

#include <KLineEdit>

class GroupEditor : public KLineEdit
{
    Q_OBJECT
public:
    GroupEditor(QModelIndex group, const QString contents, QWidget * parent=0)
        : KLineEdit(contents, parent), m_groupIndex(group)
    {
    }

    QModelIndex groupIndex() const  {return m_groupIndex;}

private:
    QModelIndex m_groupIndex;
};

class TransfersGroupDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TransfersGroupDelegate(QObject * parent=0);

    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
};

class TransfersGroupTree : public QTreeView
{
    Q_OBJECT
    public:
        TransfersGroupTree(QWidget *parent=0);

    public slots:
        void editCurrent();
        void addGroup();
        void deleteSelectedGroup();
        void renameSelectedGroup();
        void changeIcon(const QString &icon);
        void commitData(QWidget *editor);
};

#endif
