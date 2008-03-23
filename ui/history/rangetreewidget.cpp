/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "rangetreewidget.h"
#include "settings.h"

#include <KIcon>
#include <KDebug>

#include <QDate>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QStyledItemDelegate>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVariant>
#include <QUrl>

class RangeTreeWidget::Range
{
public:
    Range()
    {
    };

    bool check(const QVariant data) const
    {
        if (data.type() == QVariant::String) {
            return (QString::compare(data.toString(), min.toString()) == 0);
        }
        else if (data.type() == QVariant::Int || data.type() == QVariant::Double) {
            if (data.toDouble() >= min.toDouble() && (data.toDouble() <= max.toDouble() || max.toDouble() < 0)) {
                // the last range ends with -1
                return true;
            }
        }

        return false;
    }

    int id;
    QVariant min;
    QVariant max;
    QString title;
};

RangeTreeWidget::RangeTreeWidget(QWidget *parent) : QTreeView(parent),
    m_data(),
    m_ranges(),
    m_rangeDelegate(0)
{
    setDragEnabled(false);
    setAlternatingRowColors(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    header()->setMovable(false);

    // initialize the standard item model of the tree
    m_model = new QStandardItemModel();
    setModel(m_model);

    // delegate for the range title
    RangeTreeWidgetItemDelegate *delegate = new RangeTreeWidgetItemDelegate(this);
    setItemDelegate(delegate);
}

RangeTreeWidget::~RangeTreeWidget()
{
    QList<int>  list;
    for (int i = 0; i < 5; i++)
    {
        list.append(columnWidth(i));
    }
    Settings::setHistoryColumnWidths(list);
    Settings::self()->writeConfig();
    clear();

    delete m_rangeDelegate;
}

int RangeTreeWidget::addRange(const QVariant &min, const QVariant &max, const QString &title)
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
    setFirstColumnSpanned(row, QModelIndex(), true);
    openPersistentEditor(model()->index(row, 0, QModelIndex()));

    // expand the first row
    if(row == 0) {
        setExpanded(model()->index(row, 0, QModelIndex()), true);
    }

    return row;
}

void RangeTreeWidget::clear()
{
    m_model->clear();
    m_data.clear();
    m_ranges.clear();
}

void RangeTreeWidget::add(const QVariant &data, const QString &column)
{
    QVariantList list;
    list << QVariant(column);

    add(data, column);
}

void RangeTreeWidget::add(const QVariant &data, const QVariantList &columns)
{
    QStandardItem *parent = getRange(data);

    QList <QStandardItem *> list;
    foreach(const QVariant &item, columns) {
        list << new QStandardItem(item.toString());
    }

    parent->appendRow(list);
    // TODO: need to find a better way to update rangetitlewidget count from the QStandardItem children count
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

void RangeTreeWidget::setRangeDelegate(RangeDelegate *delegate)
{
    if (m_rangeDelegate) {
        delete m_rangeDelegate;
    }
    m_rangeDelegate = delegate;
}

QList <QVariantList> RangeTreeWidget::data()
{
    QList <QVariantList> list;
    foreach(const Range &range, m_ranges) {
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

QStandardItem *RangeTreeWidget::getRange(const QVariant &data)
{
    QVariant rangeData = data;
    if (m_rangeDelegate) {
        rangeData = m_rangeDelegate->getRangeData(data);
    }

    foreach (const Range &range, m_ranges) {
        if(range.check(rangeData)) {
            return m_data [range.id];
        }
    }

    if (m_rangeDelegate) {
        int id = addRange(rangeData, rangeData, rangeData.toString());
        return m_data [id];
    }
    else {
        // if no range found, return the last one
        return m_data [m_data.size() - 1];
    }
}

RangeDelegate::RangeDelegate(QObject *parent) : QObject(parent)
{
}

RangeDelegate::~RangeDelegate()
{
}

HostRangeDelegate::HostRangeDelegate(QObject *parent) : RangeDelegate(parent)
{
}

HostRangeDelegate::~HostRangeDelegate()
{
}

QVariant HostRangeDelegate::getRangeData(const QVariant &data)
{
    return QUrl(data.toString()).host();
}

RangeTreeWidgetItemDelegate::RangeTreeWidgetItemDelegate(QAbstractItemView *parent) : QStyledItemDelegate(parent)
{
}

void RangeTreeWidgetItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
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

    setAutoFillBackground(true);
    setMinimumWidth(80);
    setObjectName("rangeTitle");
    setStyleSheet("#rangeTitle {background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 0.5, stop: 0 white, stop: 1 palette(Midlight));margin:2px;}");

    m_titleLabel = new QLabel(QString("%1 (%2)").arg(title).arg(QString::number(count)), this);

    m_layout->addWidget(m_titleLabel);
}

void RangeTitleWidget::setTitle(const QString &title, int count)
{
    m_titleLabel->setText(QString("%1 (%2)").arg(title).arg(QString::number(count)));
}

#include "rangetreewidget.moc"
