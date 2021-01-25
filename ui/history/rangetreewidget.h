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
#include <QSortFilterProxyModel>
#include <QTreeView>

class QStandardItem;
class QStandardItemModel;
class QSortFilterProxyModel;
class QVariant;
class RangeDelegate;

/**
* We need to override the qsortfilterproxymodel behavior
* to avoid include the range rows in the search filter
* the range rows are always showed
**/
class RangeSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    RangeSortFilterProxyModel(QObject *parent = nullptr);
    ~RangeSortFilterProxyModel() override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

class RangeTreeWidget : public QTreeView
{
Q_OBJECT
public:

    RangeTreeWidget(QWidget *parent = nullptr);
    ~RangeTreeWidget() override;

    /**
    * Creates a range with a title between two values
    */
    int addRange(const QVariant &min, const QVariant &max, const QString &title);
    void clear();

    void add(const QVariant &data, const QString &column);
    void add(const QVariant &data, const QVariantList &columns);
    void addLabel(const QString &title);
    void setLabels(const QStringList &labels);

    /**
    * Set a delegate in case you want to create the ranges dynamically
    * Ej, the host ranges delegate
    */
    void setRangeDelegate(RangeDelegate *delegate);

    QList <QVariantList> data();
    QStandardItem *currentItem(int column = -1);
    QStandardItem *item(const QModelIndex &index = QModelIndex(), int column = 0);

public Q_SLOTS:
    void removeRow(int row, const QModelIndex &parent = QModelIndex());
    /**
    * Filters the data throws the qsortfilterproxymodel
    */
    void setFilterRegExp(const QString &text);

private:
    /**
    * Returns the selected range for a data.
    * If a rangedelegate is set, then gets the range from the delegate
    */
    QStandardItem *getRange(const QVariant &data);

private:
    class Range;

    QStandardItemModel *m_model;
    RangeSortFilterProxyModel *m_proxyModel;
    QMap <int,  QStandardItem *> m_data;
    QList <RangeTreeWidget::Range> m_ranges;

    RangeDelegate *m_rangeDelegate;
};

/**
* Creates ranges dynamically, based on the item data
*/
class RangeDelegate : public QObject
{
Q_OBJECT
    public:
        RangeDelegate(QObject *parent = nullptr);
        ~RangeDelegate() override;

        /**
        * Returns the current range of the incoming data
        */
        virtual QVariant getRangeData(const QVariant &data) = 0;
};

/**
* Creates a ragen based on the host of the transfer
*/
class HostRangeDelegate : public RangeDelegate
{
Q_OBJECT
    public:
        HostRangeDelegate(QObject *parent = nullptr);
        ~HostRangeDelegate() override;

        QVariant getRangeData(const QVariant &data) override;
};

class RangeTreeWidgetItemDelegate : public QStyledItemDelegate
{
public:
    RangeTreeWidgetItemDelegate(QAbstractItemView *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint (const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
#endif
