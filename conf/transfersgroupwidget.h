/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2007 Javier Goday <jgoday @ gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef TRANSFER_GROUPS_WIDGET_H
#define TRANSFER_GROUPS_WIDGET_H

#include <QVBoxLayout>
#include <QTreeView>

class QAbstractItemModel;
class QStandardItemModel;
class QModelIndex;
class QStandardItem;
class QPushButton;
class QItemSelection;

class TransfersGroupTree : public QTreeView
{
    Q_OBJECT
public:
    TransfersGroupTree(QWidget *parent=0);

public slots:
    void addGroup();
    void deleteSelectedGroup();
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
};

#endif
