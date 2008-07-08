/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERHISTORYCATEGORIZEDVIEW_H
#define TRANSFERHISTORYCATEGORIZEDVIEW_H

#include "ui/history/transferhistorycategorizeddelegate.h"

#include <QWidget>
#include <QModelIndex>

class QDate;
class QStandardItemModel;
class KCategorizedView;
class KCategoryDrawer;
class KCategorizedSortFilterProxyModel;

class TransferHistoryCategorizedView : public QWidget
{
Q_OBJECT
public:

    TransferHistoryCategorizedView(QWidget *parent = 0);
    ~TransferHistoryCategorizedView();

    void addData(const QDate &date, const QString &url, const QString &dest, int size);
    QVariant data(const QModelIndex &index, TransferHistoryCategorizedDelegate::AlternativeRoles role) const;

signals:
    void deletedTransfer(const QString &url, const QModelIndex &index);
    void doubleClicked(const QModelIndex &);

public slots:
    void clear();
    void setFilterRegExp(const QString &text);
    void setCategorizedDelegate(TransferHistoryCategorizedDelegate *m_delegate);
    void removeRow(int row, const QModelIndex &parent = QModelIndex());

private slots:
    void update();

private:
    QStandardItemModel *m_model;
    KCategorizedView *m_view;
    KCategoryDrawer *m_drawer;
    KCategorizedSortFilterProxyModel *m_proxyModel;
    TransferHistoryCategorizedDelegate *m_delegate;
};
#endif
