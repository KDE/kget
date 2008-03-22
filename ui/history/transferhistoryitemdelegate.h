/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef TRANSFERHISTORYITEMDELEGATE_H
#define TRANSFERHISTORYITEMDELEGATE_H

#include <QItemDelegate>
#include <QObject>

class QModelIndex;

class TransferHistoryItemDelegate : public QItemDelegate
{
Q_OBJECT
public:
    TransferHistoryItemDelegate();
    ~TransferHistoryItemDelegate();

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;

signals:
    void deletedTransfer(const QString &url, const QModelIndex &index);
};
#endif
