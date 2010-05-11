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

#include "btnepomukhandler.h"

#include "core/transfer.h"

#include "ndo.h"
#include "nie.h"
#include <Soprano/Vocabulary/Xesam>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <QtCore/QFile>
//TODO deinit
BtNepomukHandler::BtNepomukHandler(Transfer *transfer)
  : NepomukHandler(transfer),
    m_tempResource(Nepomuk::Resource("KGet::Transfer::" + m_transfer->source().url()))
{
}

BtNepomukHandler::~BtNepomukHandler()
{
}

void BtNepomukHandler::setNewDestination(const KUrl &newDestination)
{
    Q_UNUSED(newDestination)
    //NOTE we use setDestinations for BtNepomukHandler
}

void BtNepomukHandler::setDestinations(const QList<KUrl> &destinations)
{
    //remove all Urls that should not be in m_destinations anymore
    QHash<KUrl, Nepomuk::Resource>::iterator it = m_resources.begin();
    QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
    while (it != itEnd)
    {
        if (!destinations.contains(it.key()))
        {
            it = m_resources.erase(it);
        }
        else
        {
            ++it;
        }
    }

    //add all new Urls
    for (int i = 0; i < destinations.length(); ++i)
    {
        if (!m_resources.contains(destinations[i]) && !destinations[i].isEmpty())
        {
            m_resources[destinations[i]] = Nepomuk::Resource(destinations[i], Nepomuk::Vocabulary::NDO::TorrentedFile());
            m_resources[destinations[i]].setProperty(Nepomuk::Vocabulary::NIE::url(), destinations[i]);
        }
    }
}

QStringList BtNepomukHandler::tags() const
{
    QStringList list;
    foreach (const Nepomuk::Tag &tag, m_tempResource.tags())
    {
        list.append(tag.genericLabel());
    }
    return list;
}

int BtNepomukHandler::rating() const
{
    return m_tempResource.rating();
}

void BtNepomukHandler::setRating(int rating)
{
    QHash<KUrl, Nepomuk::Resource>::iterator it;
    QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
    for (it = m_resources.begin(); it != itEnd; ++it)
    {
        if (QFile::exists(it.key().pathOrUrl()) && it.value().isValid())
        {
            (*it).setRating(rating);
        }
    }
    m_tempResource.setRating(rating);
}

void BtNepomukHandler::addTag(const QString &newTag)
{
    if (!newTag.isEmpty())
    {
        QHash<KUrl, Nepomuk::Resource>::iterator it;
        QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
        for (it = m_resources.begin(); it != itEnd; ++it)
        {
            if (QFile::exists(it.key().pathOrUrl()) && it.value().isValid())
            {
                (*it).addTag(Nepomuk::Tag(newTag));
            }
        }
        m_tempResource.addTag(newTag);
    }
}

void BtNepomukHandler::addTags(const QStringList &newTags)
{
    if (!newTags.isEmpty())
    {
        QStringList tags = newTags;
        tags.removeAll(QString());

        QHash<KUrl, Nepomuk::Resource>::iterator it;
        QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
        for (it = m_resources.begin(); it != itEnd; ++it)
        {
            if (QFile::exists(it.key().pathOrUrl()) && it.value().isValid())
            {
                foreach (const QString &tag, tags)
                {
                    (*it).addTag(Nepomuk::Tag(tag));
                }
            }
        }
        foreach (const QString &tag, tags)
        {
            m_tempResource.addTag(Nepomuk::Tag(tag));
        }
    }
}

void BtNepomukHandler::removeTag(const QString &oldTag)
{
    if (!oldTag.isEmpty())
    {
        QHash<KUrl, Nepomuk::Resource>::iterator it;
        QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
        for (it = m_resources.begin(); it != itEnd; ++it)
        {
            if (QFile::exists(it.key().pathOrUrl()) && it.value().isValid())
            {
                QList<Nepomuk::Tag> list = it.value().tags();
                list.removeAll(Nepomuk::Tag(oldTag));
                it.value().setTags(list);
            }
        }
        QList<Nepomuk::Tag> list = m_tempResource.tags();
        list.removeAll(Nepomuk::Tag(oldTag));
        m_tempResource.setTags(list);
    }
}

void BtNepomukHandler::saveFileProperties()
{
    Nepomuk::Resource srcFileRes(m_transfer->source(), Nepomuk::Vocabulary::NDO::Torrent());
    srcFileRes.setProperty(Nepomuk::Vocabulary::NIE::url(), m_transfer->source());
    foreach (Nepomuk::Resource res, m_resources.values()) {
        NepomukHandler::saveFileProperties(res);
        srcFileRes.addProperty(Nepomuk::Vocabulary::NIE::hasLogicalPart(), res);
    }
}

void BtNepomukHandler::deinit()
{
    m_tempResource.remove();

    QHash<KUrl, Nepomuk::Resource>::iterator it;
    QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
    for (it = m_resources.begin(); it != itEnd; ++it)
    {
        if (!QFile::exists(it.key().pathOrUrl()))
        {
            it.value().remove();
        }
    }
}
