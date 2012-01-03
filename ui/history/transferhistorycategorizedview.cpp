/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "transferhistorycategorizedview.h"
#include "ui/history/transferhistoryitemdelegate.h"

#include <kcategorizedsortfilterproxymodel.h>
#include <kcategorizedview.h>
#include <kcategorydrawer.h>
#include <KDialog>
#include <KDebug>

#include <QGridLayout>
#include <QLineEdit>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVariant>

TransferHistoryCategorizedView::TransferHistoryCategorizedView(QWidget *parent)
    : QWidget(parent)
{
    // the widget layout
    QGridLayout *layout = new QGridLayout();

    // initialize the model
    m_model = new QStandardItemModel();

    // the kcategoryizedview list
    TransferHistoryItemDelegate *item_delegate = new TransferHistoryItemDelegate(this);
    m_drawer = new KCategoryDrawer();
    m_view = new KCategorizedView(this);
    m_view->setCategoryDrawer(m_drawer);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setSpacing(KDialog::spacingHint());
    m_view->setViewMode(QListView::IconMode);
    m_view->setMouseTracking(true);
    m_view->setItemDelegate(item_delegate);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_view, 0, 0);

    //  the proxy sort filter model and the categorized delegate
    m_delegate = new DateCategorizedDelegate();
    m_proxyModel = new KCategorizedSortFilterProxyModel();
    m_proxyModel->setCategorizedModel(true);
    m_proxyModel->sort(0);
    m_proxyModel->setSourceModel(m_model);
    m_view->setModel(m_proxyModel);

    connect(item_delegate, SIGNAL(deletedTransfer(QString,QModelIndex)),
                           SIGNAL(deletedTransfer(QString,QModelIndex)));
    connect(m_view, SIGNAL(doubleClicked(QModelIndex)), SIGNAL(doubleClicked(QModelIndex)));
    setLayout(layout);
}

TransferHistoryCategorizedView::~TransferHistoryCategorizedView()
{
}

void TransferHistoryCategorizedView::addData(const QDate &date, const QString &url, const QString &dest, int size)
{
    QStandardItem *item = new QStandardItem(url);
    item->setData(QVariant(size), TransferHistoryCategorizedDelegate::RoleSize);
    item->setData(QVariant(url), TransferHistoryCategorizedDelegate::RoleUrl);
    item->setData(QVariant(dest), TransferHistoryCategorizedDelegate::RoleDest);
    item->setData(QVariant(date), TransferHistoryCategorizedDelegate::RoleDate);

    m_delegate->categorizeItem(item);
    m_model->appendRow(item);
}

QVariant TransferHistoryCategorizedView::data(const QModelIndex &index, 
                                             TransferHistoryCategorizedDelegate::AlternativeRoles role) const
{
    return m_model->itemFromIndex(m_proxyModel->mapToSource(index))->data(role);
}

void TransferHistoryCategorizedView::clear()
{
    m_model->clear();
}

void TransferHistoryCategorizedView::setFilterRegExp(const QString &text)
{
    m_proxyModel->setFilterRegExp(text);
}

void TransferHistoryCategorizedView::setCategorizedDelegate(TransferHistoryCategorizedDelegate *delegate)
{
    delete m_delegate;
    m_delegate = delegate;

    update();
}

void TransferHistoryCategorizedView::removeRow(int row, const QModelIndex &parent)
{
    m_model->removeRow(row, parent);
}

void TransferHistoryCategorizedView::update()
{
    for(int i=0; i<m_model->rowCount(); i++) {
        QStandardItem *item = m_model->item(i, 0);

        m_delegate->categorizeItem(item);
    }

    m_proxyModel = new KCategorizedSortFilterProxyModel(this);
    m_proxyModel->setCategorizedModel(true);
    m_proxyModel->sort(0);
    m_proxyModel->setSourceModel(m_model);
    QAbstractItemModel *oldProxy = m_view->model();
    m_view->setModel(m_proxyModel);
    oldProxy->deleteLater();
}

#include "transferhistorycategorizedview.moc"
