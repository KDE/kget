/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "nepomukhandler.h"

#include "transfer.h"

#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <QtCore/QFile>

NepomukHandler::NepomukHandler(Transfer *transfer, QObject *parent)
  : QObject(parent),
    m_isValid(false),
    m_transfer(transfer),
    m_destination(transfer->dest()),
    m_resource(Nepomuk::Resource(m_destination, Soprano::Vocabulary::Xesam::File()))
{
    isValid();//NOTE the resource is checked whenever changing properties, as the underlying file could be removed etc.
    //TODO also check isValid() for the const methods? --> set m_resource to mutable?
}

NepomukHandler::~NepomukHandler()
{
}

void NepomukHandler::setRating(int rating)
{
    if (isValid())
        m_resource.setRating(rating);
}

int NepomukHandler::rating() const
{
    return m_resource.rating();
}

void NepomukHandler::addTag(const QString &newTag)
{
    if (isValid() && !newTag.isEmpty())
        m_resource.addTag(Nepomuk::Tag(newTag));
}

void NepomukHandler::addTags(const QStringList &newTags)
{
    if (isValid() && !newTags.isEmpty())
    {
        QStringList::const_iterator itEnd = newTags.end();
        for( QStringList::const_iterator it = newTags.begin(); it != itEnd; ++it)
        {
            if(!(*it).isEmpty())
            {
                m_resource.addTag(Nepomuk::Tag(*it));
            }
        }
    }
}

void NepomukHandler::removeTag(const QString &oldTag)
{
    if (isValid())
    {
        QList<Nepomuk::Tag> list = m_resource.tags();
        list.removeAll(Nepomuk::Tag(oldTag));
        m_resource.setTags(list);
    }
}

QStringList NepomukHandler::tags() const
{
    QStringList list;
    foreach (const Nepomuk::Tag &tag, m_resource.tags())
        list.append(tag.genericLabel());
    return list;
}

void NepomukHandler::saveFileProperties()
{
    if (!isValid())
        return;
    saveFileProperties(m_resource);
}

void NepomukHandler::saveFileProperties(const Nepomuk::Resource &res)
{
    Nepomuk::Resource m_res = res;
    m_res.setProperty(Soprano::Vocabulary::Xesam::originURL(), Nepomuk::Variant(m_transfer->source()));
    m_res.setProperty(Soprano::Vocabulary::Xesam::size(), Nepomuk::Variant(m_transfer->totalSize()));
}

bool NepomukHandler::isValid()
{
    bool valid = QFile::exists(m_destination.pathOrUrl());
//NOTE the assoicated properties to the .part file do not seem to be correctly moved by Nepomuk
//after finnishing the download by Nepomuk (the metadata is not transferred)
//TODO reenable when this is fixed
//     //a part file exists so use that as destination for the Nepomuk properties
//     if (!valid && (QFile::exists(m_destination.pathOrUrl() + ".part")))
//     {
//         valid = true;
//         m_destination = KUrl::fromPath(m_destination.pathOrUrl() + ".part");
//         m_resource = Nepomuk::Resource(m_destination, Soprano::Vocabulary::Xesam::File());
//     }
    valid = valid && m_resource.isValid();
    //check if the validity changed
    if (valid != m_isValid)
    {
        m_isValid = valid;
        emit validityChanged(m_isValid);
    }

    return valid;
}
