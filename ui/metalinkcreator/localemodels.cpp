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

#include <KGlobal>
#include <KLocale>
#include <KStandardDirs>

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
    else if (role == Qt::DecorationRole)
    {
        return m_countryIcons.value(index.row());
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

void CountryModel::setupModelData(const QStringList &countryCodes)
{
    foreach (const QString &countryCode, countryCodes)
    {
        const QString countryName = KGlobal::locale()->countryCodeToName(countryCode);
        if (!countryName.isEmpty())
        {
            m_countryCodes.append(countryCode);
            m_countryNames.append(countryName);

            QString path = KStandardDirs::locate("locale", QString::fromLatin1("l10n/%1/flag.png").arg(countryCode));
            if (path.isEmpty())
            {
                m_countryIcons.append(KIcon());
            }
            else
            {
                m_countryIcons.append(KIcon(path));
            }
        }
    }
    reset();
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

    if (role == Qt::DisplayRole)
    {
        return m_languageNames.value(index.row());
    }
    else if (role == Qt::UserRole)
    {
        return m_languageCodes.value(index.row());
    }

    return QVariant();
}

int LanguageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_languageCodes.count();
}

void LanguageModel::setupModelData(const QStringList &languageCodes)
{
    foreach (const QString &languageCode, languageCodes)
    {
        //no KDE-custom language-codes
        if (languageCode.contains('@'))
        {
            continue;
        }

        QString languageName = KGlobal::locale()->languageCodeToName(languageCode);
        if (!languageName.isEmpty())
        {
            m_languageCodes.append(languageCode);
            m_languageNames.append(languageName);
        }
    }
    reset();
}

#include "localemodels.moc"
