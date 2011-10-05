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

#ifndef KGET_SORTFILTERPROXYMODEL_H
#define KGET_SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class KGetSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    public:
        explicit KGetSortFilterProxyModel(int sortColumn, QObject *parent = 0);
        virtual ~KGetSortFilterProxyModel();

        enum DownloadFilterType {
            NoFilter,
            VideoFiles,
            AudioFiles,
            CompressedFiles,
            ImageFiles
        };

        enum FilterMode {
            Contain,
            DoesNotContain
        };

        void setFilterType(int filterType);
        void setFilterMode(int filterMode);
        void setFilterColumn(int column);
        bool showWebContent() const;

    public slots:
        void setShowWebContent(bool show);
        void setShowWebContent(int show);

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    private:
        /**
         * Returns true if text should be accepted by the filter
         */
        bool acceptText(const QString &text) const;

        /**
         * @param row to get the column from
         * @return string for the column that is being filtered
         */
        QString columnText(int row, const QModelIndex &sourceParent) const;

    private:
        int m_filterType;
        FilterMode m_filterMode;
        int m_column;
        bool m_showWebContent;
        QHash<int, QString> m_mimeTypes;
};

#endif // KGET_SORTFILTERPROXYMODEL_H
