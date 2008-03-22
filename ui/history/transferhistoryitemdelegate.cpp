/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferhistoryitemdelegate.h"
#include "ui/history/transferhistoryitemwidget.h"
#include "ui/history/transferhistorycategorizeddelegate.h"
#include "ui/history/transferhistorycategorizedview.h"

#include <QAbstractItemModel>
#include <QDate>
#include <QModelIndex>

TransferHistoryItemDelegate::TransferHistoryItemDelegate()
{
}

TransferHistoryItemDelegate::~TransferHistoryItemDelegate()
{
}

QWidget *TransferHistoryItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    TransferHistoryItemWidget *widget = new TransferHistoryItemWidget(parent);

    connect(widget, SIGNAL(deletedTransfer(const QString &, const QModelIndex &)),
                    SIGNAL(deletedTransfer(const QString &, const QModelIndex &)));
    return widget;
}

QSize TransferHistoryItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QSize(200, 100);
}

void TransferHistoryItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const QAbstractItemModel *model = static_cast <const QAbstractItemModel *> (index.model());
    TransferHistoryItemWidget *widget = static_cast <TransferHistoryItemWidget *> (editor);

    widget->setSize(model->data(index, TransferHistoryCategorizedDelegate::RoleSize).toInt());
    widget->setDate(model->data(index, TransferHistoryCategorizedDelegate::RoleDate).toDate());
    widget->setUrl(model->data(index, TransferHistoryCategorizedDelegate::RoleUrl).toString());
    widget->setDest(model->data(index, TransferHistoryCategorizedDelegate::RoleDest).toString());
    widget->setModelIndex(index);
}

#include "transferhistoryitemdelegate.moc"
