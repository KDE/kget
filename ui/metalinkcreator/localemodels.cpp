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

#include "localemodels.h"

#include <KLanguageName>
#include <KCountry>

#include <QLocale>
#include <QStandardPaths>


CountryModel::CountryModel(QObject *parent)
  : QAbstractListModel(parent)
{
}

QVariant CountryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return m_countryNames.value(index.row());
    }
    else if (role == Qt::UserRole)
    {
        return m_countryCodes.value(index.row());
    }

    return QVariant();
}

int CountryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_countryCodes.count();
}

void CountryModel::setupModelData()
{
    beginResetModel();
    for (int c = 1; c <= QLocale::LastCountry; ++c)
    {
        QString countryCode;
        const auto country = static_cast<QLocale::Country>(c);
        QLocale locale(QLocale::AnyLanguage, country);
        if (locale.country() == country)
        {
            const QString localeName = locale.name();
            const auto idx = localeName.indexOf(QLatin1Char('_'));
            if (idx != -1)
            {
                countryCode = localeName.mid(idx + 1);
            }
        }
        const QString countryName = KCountry::fromAlpha2(countryCode).name();

        if (!countryName.isEmpty())
        {
            m_countryCodes.append(countryCode);
            m_countryNames.append(countryName);
        }
    }
    endResetModel();
}

LanguageModel::LanguageModel(QObject *parent)
: QAbstractListModel(parent)
{
}

QVariant LanguageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    else if (role == Qt::DisplayRole)
    {
        return m_languageNames.value(index.row());
    }

    return QVariant();
}

int LanguageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_languageNames.count();
}

void LanguageModel::setupModelData()
{
    beginResetModel();
    for (int l = 1; l <= QLocale::LastLanguage; ++l)
    {
        const auto lang = static_cast<QLocale::Language>(l);
        QLocale locale(lang);
        if (locale.language() == lang)
        {
            const QString localeName = locale.name();
            const QString languageName = KLanguageName::nameForCode(localeName);
            if (!languageName.isEmpty())
            {
                m_languageNames.append(languageName);
            }
        }
    }
    endResetModel();
}


