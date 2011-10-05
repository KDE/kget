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

static const QString ARCHIVES = QString("/x-7z-compressed,/x-ace,/x-archive,/x-arj,/x-bzip,/x-bzip-compressed-tar,/x-compressed-tar,/x-deb/,/x-rar,/x-tar,/x-rpm,/x-tarz,/zip");
static const QString WEB_CONTENT = QString("/html,/x-asp,/xhtml+xml,/x-php,");

KGetSortFilterProxyModel::KGetSortFilterProxyModel(int column, QObject *parent)
  : QSortFilterProxyModel(parent),
    m_filterType(NoFilter),
    m_filterMode(Contain),
    m_column(column),
    m_showWebContent(false)
{
    m_mimeTypes.insert(NoFilter, "");
    m_mimeTypes.insert(VideoFiles, "video/");
    m_mimeTypes.insert(AudioFiles, "audio/");
    m_mimeTypes.insert(CompressedFiles, "archive/");
    m_mimeTypes.insert(ImageFiles, "image/");
}

KGetSortFilterProxyModel::~KGetSortFilterProxyModel()
{
}

bool KGetSortFilterProxyModel::showWebContent() const
{
    return m_showWebContent;
}

void KGetSortFilterProxyModel::setFilterType(int filterType)
{
    m_filterType = filterType;
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

void KGetSortFilterProxyModel::setFilterColumn(int column)
{
    m_column = column;
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
    const QString text = columnText(sourceRow, sourceParent);
    bool show = false;


    //do not show entries if their text is empty when not using NoFilter and m_showWebContent
    if (!text.isEmpty() && (m_filterType != NoFilter))
    {
        show = meta.startsWith(m_mimeTypes[m_filterType]);

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
            show = !text.isEmpty() && !WEB_CONTENT.contains(meta.mid(meta.indexOf('/')));
        }
    }

    if (show) {
        show = acceptText(text);
    }

    return show;
}

QString KGetSortFilterProxyModel::columnText(int row, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(row, m_column, sourceParent);
    return (index.isValid() ? index.data(Qt::DisplayRole).toString() : QString());
}

bool KGetSortFilterProxyModel::acceptText(const QString &text) const
{
    //look if the text-filter matches
    const QRegExp re = filterRegExp();
    bool accept = (re.indexIn(text) != -1) ? true : false;
    if ((m_filterMode == DoesNotContain) && !re.isEmpty()) {
        accept = !accept;
    }

    return accept;
}
