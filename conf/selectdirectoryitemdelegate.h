/* This file is part of the KDE project

   Copyright (C) 2004 - 2007 KGet Developers <kget@kde.org>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef SELECT_DIRECTORY_ITEM_DELEGATE_H
#define SELECT_DIRECTORY_ITEM_DELEGATE_H

#include <QItemDelegate>

class SelectDirectoryItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SelectDirectoryItemDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, 
        const QStyleOptionViewItem &option, const QModelIndex & index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const;

};

#endif
