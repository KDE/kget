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

#ifndef LOCALEMODELS_H
#define LOCALEMODELS_H

#include <QtCore/QAbstractListModel>

#include <KIcon>

/**
 * The following models are there to store the localized names and the codes of languages and countries
 * Only codes supported by KDE are supported by these models
 */

/**
* The CountryModel stores localized names as well as the codes and the corresponding icons of countries
*/
class CountryModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        CountryModel(QObject *parent = 0);

        int rowCount (const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        void setupModelData(const QStringList &countryCodes);

    private:
        QStringList m_countryCodes;
        QStringList m_countryNames;
        QList<KIcon> m_countryIcons;
};

/**
* The LanguageModel stores localized names as well as the codes of languages
*/
class LanguageModel : public QAbstractListModel
{
    Q_OBJECT

    public:
        LanguageModel(QObject *parent = 0);

        int rowCount (const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

        void setupModelData(const QStringList &languageCodes);

    private:
        QStringList m_languageCodes;
        QStringList m_languageNames;
};

#endif
