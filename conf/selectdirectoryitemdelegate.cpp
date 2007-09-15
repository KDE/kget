/* This file is part of the KDE project

   Copyright (C) 2004 - 2007 KGet Developers <kget@kde.org>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "selectdirectoryitemdelegate.h"

#include <QHBoxLayout>
#include <QLabel>

#include <KUrlRequester>

SelectDirectoryItemDelegate::SelectDirectoryItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *SelectDirectoryItemDelegate::createEditor(QWidget *parent,
            const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    KUrlRequester *widget = new KUrlRequester(parent);
    widget->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    widget->setPath(index.model()->data(index, Qt::DisplayRole).toString());

    return widget;
}

void SelectDirectoryItemDelegate::updateEditorGeometry(QWidget *editor,
     const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

void SelectDirectoryItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                    const QModelIndex &index) const
{
    KUrlRequester *fileWidget = static_cast<KUrlRequester*>(editor);
    QString path = fileWidget->url().pathOrUrl();

    if (!path.isEmpty()) {
        model->setData(index, path);
    }
}
