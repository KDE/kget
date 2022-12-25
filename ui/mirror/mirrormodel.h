/***************************************************************************
 *   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef MIRRORMODEL_H
#define MIRRORMODEL_H

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

#include <QIcon>
#include <QUrl>

class QSortFilterProxyModel;

class MirrorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MirrorDelegate(QObject *parent = nullptr);
    explicit MirrorDelegate(QSortFilterProxyModel *countrySort, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QSortFilterProxyModel *m_countrySort = nullptr;
};

class MirrorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    MirrorProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

class MirrorItem
{
public:
    MirrorItem();

    enum DataType { Used = 0, Url, Connections, Priority, Country };

    QVariant data(int column, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(int column) const;
    bool setData(int column, const QVariant &value, int role = Qt::EditRole);

private:
    QUrl m_url;
    Qt::CheckState m_checked;
    int m_numConnections;
    int m_priority;
    QString m_countryCode;
    QString m_countryName;
};

class MirrorModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MirrorModel(QObject *parent);
    ~MirrorModel() override;

    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    int columnCount(const QModelIndex &index = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void addMirror(const QUrl &url, int numConnections = 0, int priority = 0, const QString &countryCode = QString());
    void setMirrors(const QHash<QUrl, QPair<bool, int>> &mirrors);
    QHash<QUrl, QPair<bool, int>> availableMirrors() const;

private:
    QList<MirrorItem *> m_data;
};

#endif
