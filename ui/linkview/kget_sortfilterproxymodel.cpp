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

#include "kget_sortfilterproxymodel.h"

#include <QtCore/QStringList>

static const QStringList MIME_TYPES = QString(";video/;audio/;archive/;image/").split(';');
static const QString ARCHIVES = QString("/x-7z-compressed,/x-ace,/x-archive,/x-arj,/x-bzip,/x-bzip-compressed-tar,/x-compressed-tar,/x-rar,/x-tar,/x-tarz,/zip");
static const QString WEB_CONTENT = QString("/html,/x-asp,/xhtml+xml,/x-php,");

KGetSortFilterProxyModel::KGetSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), m_filterType(KGetSortFilterProxyModel::NoFilter),
    m_filterMode(KGetSortFilterProxyModel::Contain), m_showWebContent(false)
{
}

KGetSortFilterProxyModel::~KGetSortFilterProxyModel()
{
}

KGetSortFilterProxyModel::DownloadFilterType KGetSortFilterProxyModel::filterType() const
{
    return m_filterType;
}

KGetSortFilterProxyModel::FilterMode KGetSortFilterProxyModel::filterMode() const
{
    return m_filterMode;
}

bool KGetSortFilterProxyModel::showWebContent() const
{
    return m_showWebContent;
}

void KGetSortFilterProxyModel::setFilterType(KGetSortFilterProxyModel::DownloadFilterType filterType)
{
    m_filterType = filterType;
    invalidateFilter();
}

void KGetSortFilterProxyModel::setFilterType(int filterType)
{
    switch (filterType)
    {
        case AudioFiles:
            m_filterType = AudioFiles;
            break;
        case KGetSortFilterProxyModel::VideoFiles:
            m_filterType = VideoFiles;
            break;
        case KGetSortFilterProxyModel::CompressedFiles:
            m_filterType = CompressedFiles;
            break;
        case KGetSortFilterProxyModel::ImageFiles:
            m_filterType = ImageFiles;
            break;
        case KGetSortFilterProxyModel::NoFilter:
        default:
            m_filterType = NoFilter;
    }

    invalidateFilter();
}

void KGetSortFilterProxyModel::setFilterMode(KGetSortFilterProxyModel::FilterMode filterMode)
{
    m_filterMode = filterMode;
    invalidateFilter();
}

void KGetSortFilterProxyModel::setFilterMode(int filterMode)
{
    switch (filterMode)
    {
        case DoesNotContain:
            m_filterMode = DoesNotContain;
            break;
        case Contain:
        default:
            m_filterMode = Contain;
    }

    invalidateFilter();
}

void KGetSortFilterProxyModel::setShowWebContent(bool show)
{
    m_showWebContent = show;
    invalidateFilter();
}

void KGetSortFilterProxyModel::setShowWebContent(int show)
{
    m_showWebContent = show;
    invalidateFilter();
}

bool KGetSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 1, sourceParent);
    if (!index.isValid() || index.data(Qt::UserRole).toString().isEmpty())
    {
        return false;
    }

    const QString meta = index.data(Qt::UserRole).toString();
    const QString name = index.data(Qt::DisplayRole).toString();
    bool show = false;

    //do not show empty files when not using NoFilter and m_showWebContent
    if (!name.isEmpty() && (m_filterType != NoFilter))
    {
        show = meta.startsWith(MIME_TYPES[m_filterType]);

        if (m_filterType == CompressedFiles)
        {
            show = ARCHIVES.contains(meta.mid(meta.indexOf('/')));
        }
    }
    else if (m_filterType == NoFilter)
    {
        if (m_showWebContent)
        {
            show = true;
        }
        else
        {
            show = !name.isEmpty() && !WEB_CONTENT.contains(meta.mid(meta.indexOf('/')));
        }
    }

    if (show)
    {
        //look if the text-filter matches
        const QRegExp re = filterRegExp();
        show = (re.indexIn(name) != -1) ? true : false;
        if ((m_filterMode == DoesNotContain) && !re.isEmpty())
        {
            show = !show;
        }
    }

    return show;
}
