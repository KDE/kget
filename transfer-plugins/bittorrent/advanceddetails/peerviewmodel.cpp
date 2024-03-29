/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include "peerviewmodel.h"

#include <KLocalizedString>

#include <QLocale>

#include <QDebug>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{
static QIcon yes, no;
static bool icons_loaded = false;

PeerViewModel::Item::Item(bt::PeerInterface *peer)
    : peer(peer)
{
    stats = peer->getStats();
    if (!icons_loaded) {
        yes = QIcon::fromTheme("dialog-ok");
        no = QIcon::fromTheme("dialog-cancel");
    }
}
/*
bool PeerViewModel::Item::changed() const
{
const PeerInterface::Stats & s = peer->getStats();


if (s.download_rate != stats.download_rate ||
    s.upload_rate != stats.upload_rate ||
    s.choked != stats.choked ||
    s.snubbed != stats.snubbed ||
    s.perc_of_file != stats.perc_of_file ||
    s.aca_score != stats.aca_score ||
    s.has_upload_slot != stats.has_upload_slot ||
    s.num_down_requests != stats.num_down_requests ||
    s.num_up_requests != stats.num_up_requests ||
    s.bytes_downloaded != stats.bytes_downloaded ||
    s.bytes_uploaded != stats.bytes_uploaded ||
    s.interested != stats.interested ||
    s.am_interested != stats.am_interested)
{
    stats = s;
    return true;
}
return false;
}
*/
bool PeerViewModel::Item::changed(int col, bool &modified) const
{
    const PeerInterface::Stats &s = peer->getStats();
    bool ret = false;

    switch (col) {
    case 3:
        ret = s.download_rate != stats.download_rate;
        break;
    case 4:
        ret = s.upload_rate != stats.upload_rate;
        break;
    case 5:
        ret = s.choked != stats.choked;
        break;
    case 6:
        ret = s.snubbed != stats.snubbed;
        break;
    case 7:
        ret = s.perc_of_file != stats.perc_of_file;
        break;
    case 9:
        ret = s.aca_score != stats.aca_score;
        break;
    case 10:
        ret = s.has_upload_slot != stats.has_upload_slot;
        break;
    case 11:
        ret = (s.num_down_requests != stats.num_down_requests || s.num_up_requests != stats.num_up_requests);
        break;
    case 12:
        ret = s.bytes_downloaded != stats.bytes_downloaded;
        break;
    case 13:
        ret = s.bytes_uploaded != stats.bytes_uploaded;
        break;
    case 14:
        ret = s.interested != stats.interested;
        break;
    case 15:
        ret = s.am_interested != stats.am_interested;
        break;
    default:
        ret = false;
        break;
    }

    modified = s.download_rate != stats.download_rate || s.upload_rate != stats.upload_rate || s.choked != stats.choked || s.snubbed != stats.snubbed
        || s.perc_of_file != stats.perc_of_file || s.aca_score != stats.aca_score || s.has_upload_slot != stats.has_upload_slot
        || s.num_down_requests != stats.num_down_requests || s.num_up_requests != stats.num_up_requests || s.bytes_downloaded != stats.bytes_downloaded
        || s.bytes_uploaded != stats.bytes_uploaded || s.interested != stats.interested || s.am_interested != stats.am_interested;
    stats = s;
    return ret;
}

QVariant PeerViewModel::Item::data(int col) const
{
    switch (col) {
    case 0:
        return stats.ip_address;
    case 1:
        return stats.client;
    case 2:
        if (stats.download_rate >= 103)
            return BytesPerSecToString(stats.download_rate);
        else
            return QVariant();
    case 3:
        if (stats.upload_rate >= 103)
            return BytesPerSecToString(stats.upload_rate);
        else
            return QVariant();
    case 4:
        return stats.choked ? i18nc("Choked", "Yes") : i18nc("Not choked", "No");
    case 5:
        return stats.snubbed ? i18nc("Snubbed", "Yes") : i18nc("Not snubbed", "No");
    case 6:
        return QString("%1 %").arg(QLocale().toString(stats.perc_of_file, 'g', 2));
    case 7:
        return QVariant();
    case 8:
        return QLocale().toString(stats.aca_score, 'g', 2);
    case 9:
        return QVariant();
    case 10:
        return QString("%1 / %2").arg(stats.num_down_requests).arg(stats.num_up_requests);
    case 11:
        return BytesToString(stats.bytes_downloaded);
    case 12:
        return BytesToString(stats.bytes_uploaded);
    case 13:
        return stats.interested ? i18nc("Interested", "Yes") : i18nc("Not Interested", "No");
    case 14:
        return stats.am_interested ? i18nc("Interesting", "Yes") : i18nc("Not Interesting", "No");
    default:
        return QVariant();
    }
    return QVariant();
}

bool PeerViewModel::Item::lessThan(int col, const Item *other) const
{
    switch (col) {
    case 0:
        return stats.ip_address < other->stats.ip_address;
    case 1:
        return QString::localeAwareCompare(stats.client, other->stats.client) < 0;
    case 2:
        return stats.download_rate < other->stats.download_rate;
    case 3:
        return stats.upload_rate < other->stats.upload_rate;
    case 4:
        return stats.choked < other->stats.choked;
    case 5:
        return stats.snubbed < other->stats.snubbed;
    case 6:
        return stats.perc_of_file < other->stats.perc_of_file;
    case 7:
        return stats.dht_support < other->stats.dht_support;
    case 8:
        return stats.aca_score < other->stats.aca_score;
    case 9:
        return stats.has_upload_slot < other->stats.has_upload_slot;
    case 10:
        return stats.num_down_requests + stats.num_up_requests < other->stats.num_down_requests + other->stats.num_up_requests;
    case 11:
        return stats.bytes_downloaded < other->stats.bytes_downloaded;
    case 12:
        return stats.bytes_uploaded < other->stats.bytes_uploaded;
    case 13:
        return stats.interested < other->stats.interested;
    case 14:
        return stats.am_interested < other->stats.am_interested;
    default:
        return false;
    }
    return false;
}

QVariant PeerViewModel::Item::decoration(int col) const
{
    switch (col) {
    case 0:
        if (stats.encrypted)
            return QIcon::fromTheme("kt-encrypted");
        break;
    case 1:
        return flag;
    case 8:
        return stats.dht_support ? yes : no;
    case 10:
        return stats.has_upload_slot ? yes : QIcon();
    }

    return QVariant();
}

/////////////////////////////////////////////////////////////

PeerViewModel::PeerViewModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    sort_column = 0;
    sort_order = Qt::AscendingOrder;
}

PeerViewModel::~PeerViewModel()
{
    qDeleteAll(items);
}

void PeerViewModel::peerAdded(bt::PeerInterface *peer)
{
    items.append(new Item(peer));
    insertRow(items.count() - 1);
    sort(sort_column, sort_order);
}

void PeerViewModel::peerRemoved(bt::PeerInterface *peer)
{
    int idx = 0;
    for (QList<Item *>::iterator i = items.begin(); i != items.end(); i++) {
        Item *item = *i;
        if (item->peer == peer) {
            items.erase(i);
            delete item;
            removeRow(idx);
            break;
        }
        idx++;
    }
}

void PeerViewModel::clear()
{
    beginResetModel();
    qDeleteAll(items);
    items.clear();
    endResetModel();
}

void PeerViewModel::update()
{
    bool resort = false;
    Uint32 idx = 0;
    foreach (Item *i, items) {
        bool modified = false;
        if (i->changed(sort_column, modified))
            resort = true;

        if (modified && !resort)
            Q_EMIT dataChanged(index(idx, 3), index(idx, 15));
        idx++;
    }

    if (resort)
        sort(sort_column, sort_order);
}

QModelIndex PeerViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent) || parent.isValid())
        return QModelIndex();
    else
        return createIndex(row, column, items[row]);
}

int PeerViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return items.count();
}

int PeerViewModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return 16;
}

QVariant PeerViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return i18n("IP Address");
        case 1:
            return i18n("Client");
        case 2:
            return i18n("Down Speed");
        case 3:
            return i18n("Up Speed");
        case 4:
            return i18n("Choked");
        case 5:
            return i18n("Snubbed");
        case 6:
            return i18n("Availability");
        case 7:
            return i18n("DHT");
        case 8:
            return i18n("Score");
        case 9:
            return i18n("Upload Slot");
        case 10:
            return i18n("Requests");
        case 11:
            return i18n("Downloaded");
        case 12:
            return i18n("Uploaded");
        case 13:
            return i18n("Interested");
        case 14:
            return i18n("Interesting");
        default:
            return QVariant();
        }
    } else if (role == Qt::ToolTipRole) {
        switch (section) {
        case 0:
            return i18n("IP address of the peer");
        case 1:
            return i18n("Which client the peer is using");
        case 2:
            return i18n("Download speed");
        case 3:
            return i18n("Upload speed");
        case 4:
            return i18n("Whether or not the peer has choked us. If we are choked, the peer will not send us any data.");
        case 5:
            return i18n("Snubbed means the peer has not sent us any data in the last 2 minutes");
        case 6:
            return i18n("How much of the torrent's data the peer has");
        case 7:
            return i18n("Whether or not the peer has DHT enabled");
        case 8:
            return i18n("The score of the peer. KTorrent uses this to determine who to upload to.");
        case 9:
            return i18n("Only peers which have an upload slot will get data from us");
        case 10:
            return i18n("The number of download and upload requests");
        case 11:
            return i18n("How much data we have downloaded from this peer");
        case 12:
            return i18n("How much data we have uploaded to this peer");
        case 13:
            return i18n("Whether the peer is interested in downloading data from us");
        case 14:
            return i18n("Whether we are interested in downloading from this peer");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant PeerViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= items.count() || index.row() < 0)
        return QVariant();

    Item *item = (Item *)index.internalPointer();
    if (role == Qt::DisplayRole)
        return item->data(index.column());
    else if (role == Qt::DecorationRole)
        return item->decoration(index.column());

    return QVariant();
}

bool PeerViewModel::removeRows(int row, int count, const QModelIndex & /*parent*/)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    endRemoveRows();
    return true;
}

bool PeerViewModel::insertRows(int row, int count, const QModelIndex & /*parent*/)
{
    beginInsertRows(QModelIndex(), row, row + count - 1);
    endInsertRows();
    return true;
}

bt::PeerInterface *PeerViewModel::indexToPeer(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= items.count() || index.row() < 0)
        return nullptr;
    else
        return ((Item *)index.internalPointer())->peer;
}

class PeerViewModelItemCmp
{
public:
    PeerViewModelItemCmp(int col, Qt::SortOrder order)
        : col(col)
        , order(order)
    {
    }

    bool operator()(PeerViewModel::Item *a, PeerViewModel::Item *b)
    {
        if (order == Qt::AscendingOrder)
            return a->lessThan(col, b);
        else
            return !a->lessThan(col, b);
    }

    int col;
    Qt::SortOrder order;
};

void PeerViewModel::sort(int col, Qt::SortOrder order)
{
    sort_column = col;
    sort_order = order;
    Q_EMIT layoutAboutToBeChanged();
    std::stable_sort(items.begin(), items.end(), PeerViewModelItemCmp(col, order));
    Q_EMIT layoutChanged();
}
}

#include "moc_peerviewmodel.cpp"
