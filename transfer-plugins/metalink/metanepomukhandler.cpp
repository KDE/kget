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

#include "metanepomukhandler.h"

#include "core/transfer.h"

#include <Soprano/Vocabulary/Xesam>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <QtCore/QFile>

MetaNepomukHandler::MetaNepomukHandler(Transfer *transfer)
  : NepomukHandler(transfer),
    m_tempResource(Nepomuk::Resource("KGet::Transfer::" + m_transfer->source().url()))
{
}

MetaNepomukHandler::~MetaNepomukHandler()
{
}

void MetaNepomukHandler::setNewDestination(const KUrl &newDestination)
{
    Q_UNUSED(newDestination)
    //NOTE we use setDestinations for BtNepomukHandler
}

void MetaNepomukHandler::setDestinations(const QList<KUrl> &destinations)
{
    //remove all Urls that should not be in m_destinations anymore
    QHash<KUrl, Nepomuk::Resource>::iterator it = m_resources.begin();
    QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
    while (it != itEnd)
    {
        if (!destinations.contains(it.key()))
        {
            it = m_resources.erase(it);//TODO delete clever?
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
            m_resources[destinations[i]] = Nepomuk::Resource(destinations[i], Soprano::Vocabulary::Xesam::File());
        }
    }
}

QStringList MetaNepomukHandler::tags() const
{
    QStringList list;
    foreach (const Nepomuk::Tag &tag, m_tempResource.tags())
    {
        list.append(tag.genericLabel());
    }
    return list;
}

int MetaNepomukHandler::rating() const
{
    return m_tempResource.rating();
}

void MetaNepomukHandler::setRating(int rating)
{
    QHash<KUrl, Nepomuk::Resource>::iterator it;
    QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
    for (it = m_resources.begin(); it != itEnd; ++it)
    {
        (*it).setRating(rating);
    }
    m_tempResource.setRating(rating);
}

void MetaNepomukHandler::addTag(const QString &newTag)
{
    if (!newTag.isEmpty())
    {
        QHash<KUrl, Nepomuk::Resource>::iterator it;
        QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
        for (it = m_resources.begin(); it != itEnd; ++it)
        {
            (*it).addTag(Nepomuk::Tag(newTag));
        }
        m_tempResource.addTag(newTag);
    }
}

void MetaNepomukHandler::addTags(const QStringList &newTags)
{
    if (!newTags.isEmpty())
    {
        QStringList tags = newTags;
        tags.removeAll(QString());

        QHash<KUrl, Nepomuk::Resource>::iterator it;
        QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
        for (it = m_resources.begin(); it != itEnd; ++it)
        {
            foreach (const QString &tag, tags)
            {
                (*it).addTag(Nepomuk::Tag(tag));
            }
        }
        foreach (const QString &tag, tags)
        {
            m_tempResource.addTag(Nepomuk::Tag(tag));
        }
    }
}

void MetaNepomukHandler::removeTag(const QString &oldTag)
{
    if (!oldTag.isEmpty())
    {
        QHash<KUrl, Nepomuk::Resource>::iterator it;
        QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
        for (it = m_resources.begin(); it != itEnd; ++it)
        {
            QList<Nepomuk::Tag> list = it.value().tags();
            list.removeAll(Nepomuk::Tag(oldTag));
            it.value().setTags(list);
        }
        QList<Nepomuk::Tag> list = m_tempResource.tags();
        list.removeAll(Nepomuk::Tag(oldTag));
        m_tempResource.setTags(list);
    }
}

void MetaNepomukHandler::setProperty(const QUrl &uri, const Nepomuk::Variant &value)
{
    QHash<KUrl, Nepomuk::Resource>::iterator it;
    QHash<KUrl, Nepomuk::Resource>::iterator itEnd = m_resources.end();
    for (it = m_resources.begin(); it != itEnd; ++it)
    {
        if (uri.isValid() && value.isValid())
        {
            (*it).setProperty(uri, value);
        }
    }

    m_tempResource.setProperty(uri, value);
}

void MetaNepomukHandler::setFileMetaData(const KUrl &dest, const KGetMetalink::File &file)
{
    if (!m_resources.contains(dest))
    {
        m_resources[dest] = Nepomuk::Resource(dest, Soprano::Vocabulary::Xesam::File());
    }

    QHash<QUrl, Nepomuk::Variant> fileData = file.properties();
    QHash<QUrl, Nepomuk::Variant>::const_iterator it;
    QHash<QUrl, Nepomuk::Variant>::const_iterator itEnd = fileData.constEnd();
    for (it = fileData.constBegin(); it != itEnd; ++it)
    {
        m_resources[dest].setProperty(it.key(), it.value());
    }
}

void MetaNepomukHandler::saveFileProperties()
{
    //store the already set data for the new destinations
    //     if (!m_newDestinations.isEmpty())
    //     {
        //
        //     }
        //NOTE do something here?
}


void MetaNepomukHandler::postDeleteEvent()
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
