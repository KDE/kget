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
#include <QStyledItemDelegate>
#include <QTreeView>

class QDate;
class QLabel;
class QLayout;
class QStandardItem;
class QStandardItemModel;
class QVariant;
class RangeDelegate;

class RangeTreeWidget : public QTreeView
{
Q_OBJECT
public:

    RangeTreeWidget(QWidget *parent = 0);
    ~RangeTreeWidget();

    int addRange(const QVariant &min, const QVariant &max, const QString &title);
    void clear();

    void add(const QVariant &data, const QString &column);
    void add(const QVariant &data, const QVariantList &columns);
    void addLabel(const QString &title);
    void setLabels(const QStringList &labels);

    /**
    */
    void setRangeDelegate(RangeDelegate *delegate);

    QList <QVariantList> data();
    QStandardItem *currentItem(int column = -1);
    QStandardItem *item(const QModelIndex &index = QModelIndex(), int column = 0);

public slots:
    void removeRow(int row, const QModelIndex &parent = QModelIndex());

private:
    QStandardItem *getRange(const QVariant &data);

private:
    class Range;

    QStandardItemModel *m_model;
    QMap <int,  QStandardItem *> m_data;
    QList <RangeTreeWidget::Range> m_ranges;

    RangeDelegate *m_rangeDelegate;
};

class RangeDelegate : public QObject
{
    public:
        RangeDelegate(QObject *parent = 0);
        ~RangeDelegate();

        virtual QVariant getRangeData(const QVariant &data) = 0;
};

class HostRangeDelegate : public RangeDelegate
{
    public:
        HostRangeDelegate(QObject *parent = 0);
        ~HostRangeDelegate();

        QVariant getRangeData(const QVariant &data);
};

class RangeTreeWidgetItemDelegate : public QStyledItemDelegate
{
public:
    RangeTreeWidgetItemDelegate(QAbstractItemView *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const;
};
#endif
