/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERS_GROUP_WIDGET_H
#define TRANSFERS_GROUP_WIDGET_H

#include <KLineEdit>

#include <QItemDelegate>
#include <QVBoxLayout>
#include <QTreeView>

class QPushButton;
class QItemSelection;

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

class TransfersGroupDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    TransfersGroupDelegate(QObject * parent=0);

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;

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
    void openEditMode();
    void commitData(QWidget *editor);
};

class TransfersGroupWidget : public QVBoxLayout
{
    Q_OBJECT
public:
    TransfersGroupWidget(QWidget *parent=0);

private slots:
    void slotSelectionChanged(const QItemSelection &current, const QItemSelection &old);

private:
    TransfersGroupTree *m_view;

    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *renameButton;
};

#endif
