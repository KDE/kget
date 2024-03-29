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
#include "iwfilelistmodel.h"

#include <cmath>

#include <KLocalizedString>

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>

using namespace bt;

namespace kt
{

IWFileListModel::IWFileListModel(bt::TorrentInterface *tc, QObject *parent)
    : TorrentFileListModel(tc, KEEP_FILES, parent)
{
    mmfile = IsMultimediaFile(tc->getStats().output_path);
    preview = false;
    percentage = 0;
}

IWFileListModel::~IWFileListModel()
{
}

int IWFileListModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 5;
    else
        return 0;
}

QVariant IWFileListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    if (section < 2)
        return TorrentFileListModel::headerData(section, orientation, role);

    switch (section) {
    case 2:
        return i18n("Priority");
    case 3:
        return i18n("Preview");
    // xgettext: no-c-format
    case 4:
        return i18nc("Percent of File Downloaded", "% Complete");
    default:
        return QVariant();
    }
}

static QString PriorityString(const bt::TorrentFileInterface *file)
{
    switch (file->getPriority()) {
    case FIRST_PREVIEW_PRIORITY:
    case FIRST_PRIORITY:
        return i18nc("Download first", "First");
    case LAST_PREVIEW_PRIORITY:
    case LAST_PRIORITY:
        return i18nc("Download last", "Last");
    case ONLY_SEED_PRIORITY:
    case EXCLUDED:
        return QString();
    case NORMAL_PREVIEW_PRIORITY:
    default:
        return i18nc("Download Normal (not as first or last)", "Normal");
    }
}

QVariant IWFileListModel::data(const QModelIndex &index, int role) const
{
    if (index.column() < 2 && role != Qt::ForegroundRole)
        return TorrentFileListModel::data(index, role);

    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount(QModelIndex()))
        return QVariant();

    if (role == Qt::ForegroundRole && index.column() == 2 && tc->getStats().multi_file_torrent)
        return QVariant();

    if (role == Qt::DisplayRole)
        return displayData(index);
    else if (role == Qt::UserRole)
        return sortData(index);

    return QVariant();
}

QVariant IWFileListModel::displayData(const QModelIndex &index) const
{
    if (tc->getStats().multi_file_torrent) {
        const bt::TorrentFileInterface *file = &tc->getTorrentFile(index.row());
        switch (index.column()) {
        case 2:
            return PriorityString(file);
        case 3:
            if (file->isMultimedia()) {
                if (file->isPreviewAvailable())
                    return i18nc("Preview available", "Available");
                else
                    return i18nc("Preview pending", "Pending");
            } else
                return i18nc("No preview available", "No");
        case 4: {
            float percent = file->getDownloadPercentage();
            return ki18n("%1 %").subs(percent, 0, 'f', 2).toString();
        }
        default:
            return QVariant();
        }
    } else {
        switch (index.column()) {
        case 2:
            return QVariant();
        case 3:
            if (mmfile) {
                if (tc->readyForPreview())
                    return i18nc("Preview available", "Available");
                else
                    return i18nc("Preview pending", "Pending");
            } else
                return i18nc("No preview available", "No");
        case 4: {
            double percent = bt::Percentage(tc->getStats());
            return ki18n("%1 %").subs(percent, 0, 'f', 2).toString();
        }
        default:
            return QVariant();
        }
    }
    return QVariant();
}

QVariant IWFileListModel::sortData(const QModelIndex &index) const
{
    if (tc->getStats().multi_file_torrent) {
        const bt::TorrentFileInterface *file = &tc->getTorrentFile(index.row());
        switch (index.column()) {
        case 2:
            return (int)file->getPriority();
        case 3:
            if (file->isMultimedia()) {
                if (file->isPreviewAvailable())
                    return 3;
                else
                    return 2;
            } else
                return 1;
        case 4:
            return file->getDownloadPercentage();
        }
    } else {
        switch (index.column()) {
        case 2:
            return QVariant();
        case 3:
            if (mmfile) {
                if (tc->readyForPreview())
                    return 3;
                else
                    return 2;
            } else
                return 1;
        case 4:
            return bt::Percentage(tc->getStats());
        }
    }
    return QVariant();
}

bool IWFileListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
        return TorrentFileListModel::setData(index, value, role);

    if (!index.isValid() || role != Qt::UserRole)
        return false;

    int r = index.row();
    if (r < 0 || r >= rowCount(QModelIndex()))
        return false;

    bt::TorrentFileInterface &file = tc->getTorrentFile(r);
    ;
    auto prio = (bt::Priority)value.toInt();
    Priority old = file.getPriority();

    if (prio != old) {
        file.setPriority(prio);
        dataChanged(createIndex(index.row(), 0), createIndex(index.row(), 4));
    }

    return true;
}

void IWFileListModel::filePercentageChanged(bt::TorrentFileInterface *file, float percentage)
{
    Q_UNUSED(percentage)
    QModelIndex idx = createIndex(file->getIndex(), 4, file);
    Q_EMIT dataChanged(idx, idx);
}

void IWFileListModel::filePreviewChanged(bt::TorrentFileInterface *file, bool preview)
{
    Q_UNUSED(preview)
    QModelIndex idx = createIndex(file->getIndex(), 3, file);
    Q_EMIT dataChanged(idx, idx);
}

void IWFileListModel::update()
{
    if (!tc->getStats().multi_file_torrent) {
        bool changed = false;
        bool np = mmfile && tc->readyForPreview();
        if (preview != np) {
            preview = np;
            changed = true;
        }

        double perc = bt::Percentage(tc->getStats());
        if (fabs(perc - percentage) > 0.01) {
            percentage = perc;
            changed = true;
        }

        if (changed)
            dataChanged(createIndex(0, 0), createIndex(0, 4));
    }
}
}

#include "moc_iwfilelistmodel.cpp"
