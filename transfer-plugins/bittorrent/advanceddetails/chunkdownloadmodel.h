/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef KTCHUNKDOWNLOADMODEL_H
#define KTCHUNKDOWNLOADMODEL_H

#include <QAbstractTableModel>
#include <interfaces/chunkdownloadinterface.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{

/**
    @author
*/
class ChunkDownloadModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ChunkDownloadModel(QObject *parent);
    ~ChunkDownloadModel() override;

    /// A peer has been added
    void downloadAdded(bt::ChunkDownloadInterface *cd);

    /// A download has been removed
    void downloadRemoved(bt::ChunkDownloadInterface *cd);

    /// change the current torrent
    void changeTC(bt::TorrentInterface *tc);

    /**
     * Update the model
     */
    void update();

    void clear();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

public Q_SLOTS:
    void sort(int col, Qt::SortOrder order) override;

public:
    struct Item {
        mutable bt::ChunkDownloadInterface::Stats stats;
        bt::ChunkDownloadInterface *cd;
        QString files;

        Item(bt::ChunkDownloadInterface *cd, const QString &files);

        bool changed(int col, bool &modified) const;
        QVariant data(int col) const;
        bool lessThan(int col, const Item *other) const;
    };

private:
    QList<Item *> items;
    bt::TorrentInterface *tc;
    int sort_column;
    Qt::SortOrder sort_order;
};
}

#endif
