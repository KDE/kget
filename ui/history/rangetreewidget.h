/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef RANGETREEWIDGET_H
#define RANGETREEWIDGET_H

#include <QMap>
#include <QItemDelegate>
#include <QTreeView>

class QDate;
class QLabel;
class QLayout;
class QStandardItem;
class QStandardItemModel;

class RangeTreeWidget : public QTreeView
{
Q_OBJECT
public:

    RangeTreeWidget(QWidget *parent = 0);
    ~RangeTreeWidget();

    void addRange(long min, long max, const QString &title);
    void clear();

    void add(long data, const QString &column);
    void add(long data, const QVariantList &columns);
    void addLabel(const QString &title);
    void setLabels(const QStringList &labels);

    QList <QVariantList> data();
    QStandardItem *currentItem(int column = -1);
    QStandardItem *item(const QModelIndex &index = QModelIndex(), int column = 0);

public slots:
    void removeRow(int row, const QModelIndex &parent = QModelIndex());

private:
    QStandardItem *getRange(long data);

private:
    class Range;

    QStandardItemModel *m_model;
    QMap <int,  QStandardItem *> m_data;
    QList <RangeTreeWidget::Range> m_ranges;
};

class RangeTreeWidgetItemDelegate : public QItemDelegate
{
public:
    RangeTreeWidgetItemDelegate(QAbstractItemView *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

    void setEditorData(QWidget * editor, const QModelIndex & index) const;
};

class RangeTitleWidget : public QWidget
{
public:
    RangeTitleWidget(const QString &title, int count, QWidget *parent=0);

    void setTitle(const QString &title, int count = 0);

private:
    QLayout *m_layout;
    QLabel *m_titleLabel;
};

#endif
