/* This file is part of the KDE project

   Based in the kcategorizeditemsviewdelegate from kdebase/workspace/libs/plasma/appletbrowser by Ivan Cukic
   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef TRANSFERHISTORYITEMDELEGATE_H
#define TRANSFERHISTORYITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>

#define ICON_SIZE 40
#define PADDING 5

class QAction;
class QModelIndex;

class TransferHistoryItemDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
    TransferHistoryItemDelegate(QWidget *parent);
    ~TransferHistoryItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent * event, QAbstractItemModel * model,
                    const QStyleOptionViewItem & option, const QModelIndex & index);

signals:
    void deletedTransfer(const QString &url, const QModelIndex &index);

private slots:
    void slotOpenFile();
    void slotDownload();
    void slotDeleteTransfer();

private:
    QWidget *m_view;
    QAction *m_actionDelete_Selected;
    QAction *m_actionDownload;
    QAction *m_openFile;

    QModelIndex m_selectedIndex;
};
#endif
