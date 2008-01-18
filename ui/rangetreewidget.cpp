/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "rangetreewidget.h"

#include <KIcon>
#include <KDebug>

#include <QDate>
#include <QLabel>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemDelegate>
#include <QStandardItem>
#include <QStandardItemModel>

class RangeTreeWidget::Range
{
public:
    Range()
    {
    };

    bool check(int data)
    {
        if (data >= min && (data <= max || max < 0)) { // the last range ends with -1
            return true;
        }
        return false;
    }

    int id;
    int min;
    int max;
    QString title;
};

RangeTreeWidget::RangeTreeWidget(QWidget *parent) : QTreeView(parent),
    m_data(), m_ranges()
{
    setDragEnabled(FALSE);
    setAlternatingRowColors(TRUE);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    header()->setMovable(FALSE);

    // initialize the standard item model of the tree
    m_model = new QStandardItemModel();
    setModel(m_model);

    // delegate for the range title
    RangeTreeWidgetItemDelegate *delegate = new RangeTreeWidgetItemDelegate(this);
    setItemDelegate(delegate);
}

RangeTreeWidget::~RangeTreeWidget()
{
    clear();
}

void RangeTreeWidget::addRange(int min, int max, const QString &title)
{
    int row = m_data.size();

    Range range;
    range.min = min;
    range.max = max;
    range.title = title;
    range.id = row;
    m_ranges << range;

    m_data [row] = new QStandardItem(title);
    m_model->insertRow(row, m_data [row]);
    setFirstColumnSpanned(row, QModelIndex(), TRUE);
    openPersistentEditor(model()->index(row, 0, QModelIndex()));

    // expand the first row
    if(row == 0) {
        setExpanded(model()->index(row, 0, QModelIndex()), TRUE);
    }
}

void RangeTreeWidget::clear()
{
    m_model->clear();
    m_data.clear();
    m_ranges.clear();
}

void RangeTreeWidget::add(int data, const QString &column)
{
    QVariantList list;
    list << QVariant(column);

    add(data, column);
}

void RangeTreeWidget::add(int data, const QVariantList &columns)
{
    QStandardItem *parent = getRange(data);

    QList <QStandardItem *> list;
    foreach(QVariant item, columns) {
        list << new QStandardItem(item.toString());
    }

    parent->appendRow(list);
    // TODO: need to find a better way to update rangetitlewidget count from the QStandardItem childs count
    closePersistentEditor(parent->index());
    openPersistentEditor(parent->index());
}

void RangeTreeWidget::addLabel(const QString &title)
{
    int index = header()->count();
    m_model->setColumnCount(index + 1);
    m_model->setHeaderData(index, Qt::Horizontal, title);
}

void RangeTreeWidget::setLabels(const QStringList &labels)
{
    m_model->setColumnCount(labels.size());

    for(int i=0; i<labels.size(); i++) {
        m_model->setHeaderData(i, Qt::Horizontal, labels.at(i));
    }
}

QList <QVariantList> RangeTreeWidget::data()
{
    QList <QVariantList> list;
    foreach(Range range, m_ranges) {
        QStandardItem *parent = m_model->itemFromIndex(model()->index(range.id, 0, QModelIndex()));

        for(int y=0; y<parent->rowCount(); y++) {
            QVariantList items;
            for(int x=0; x<header()->count(); x++) {
                QStandardItem *item = parent->child(y, x);
                items << item->data(Qt::DisplayRole);
            }
            list << items;
        }
    }

    return list;
}

QStandardItem *RangeTreeWidget::currentItem(int column)
{
    QStandardItem *item = 0;
    if (column >= 0) {
        item = m_model->itemFromIndex(model()->index(currentIndex().row(), column, currentIndex().parent()));
    }
    else {
        item = m_model->itemFromIndex(currentIndex());
    }
    return item;
}

QStandardItem *RangeTreeWidget::item(const QModelIndex &index, int column)
{
    return m_model->itemFromIndex(model()->index(index.row(), column, index.parent()));
}

void RangeTreeWidget::removeRow(int row, const QModelIndex &parent)
{
    m_model->removeRow(row, parent);
}

QStandardItem *RangeTreeWidget::getRange(int data)
{
    foreach (Range range, m_ranges) {
        if(range.check(data)) {
            return m_data [range.id];
        }
    }

    // if no range found, return the last one
    return m_data [m_data.size() - 1];
}

RangeTreeWidgetItemDelegate::RangeTreeWidgetItemDelegate(QAbstractItemView *parent) : QItemDelegate(parent)
{
}

void RangeTreeWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QItemDelegate::paint(painter, option, index);
}

QSize RangeTreeWidgetItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QSize(0, 30);
}

QWidget *RangeTreeWidgetItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    Q_UNUSED(option)

    const QStandardItemModel *model = static_cast<const QStandardItemModel *> (index.model());
    QStandardItem *item = model->itemFromIndex(index);
    return new RangeTitleWidget(item->data(Qt::DisplayRole).toString(), item->rowCount(), parent);
}

void RangeTreeWidgetItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    RangeTitleWidget *title = static_cast<RangeTitleWidget *> (editor);
    const QStandardItemModel *model = static_cast<const QStandardItemModel *> (index.model());
    QStandardItem *item = model->itemFromIndex(index);

    title->setTitle(item->data(Qt::DisplayRole).toString(), item->rowCount());
}

RangeTitleWidget::RangeTitleWidget(const QString &title, int count, QWidget *parent) : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    setLayout(m_layout);

    setAutoFillBackground(TRUE);
    setMinimumWidth(80);
    setObjectName("rangeTitle");
    setStyleSheet("#rangeTitle {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 0.5, stop: 0 white, stop: 1 palette(Midlight));margin:2px;}");

    m_titleLabel = new QLabel(title + " ("+QString::number(count)+")", this);

    m_layout->addWidget(m_titleLabel);
}

void RangeTitleWidget::setTitle(const QString &title, int count)
{
    m_titleLabel->setText(title + " ("+QString::number(count)+")");
}
