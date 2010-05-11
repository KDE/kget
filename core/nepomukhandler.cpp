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

#include "nfo.h"
#include "ndo.h"
#include "nie.h"
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/NAO>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>
#include <QtCore/QFile>

NepomukHandler::NepomukHandler(Transfer *transfer)
  : QObject(transfer),
    m_transfer(transfer),
    m_destination(transfer->dest()),
    m_resource(Nepomuk::Resource(m_destination, Nepomuk::Vocabulary::NFO::FileDataObject()))
{
}

NepomukHandler::~NepomukHandler()
{
}

void NepomukHandler::setNewDestination(const KUrl &newDestination)
{
    //TODO look if set tags get copied over!
    m_destination = newDestination;
    m_resource = Nepomuk::Resource(m_destination, Nepomuk::Vocabulary::NFO::FileDataObject());
}

void NepomukHandler::setRating(int rating)
{
    m_resource.setRating(rating);
}

int NepomukHandler::rating() const
{
    return m_resource.rating();
}

void NepomukHandler::addTag(const QString &newTag)
{
    if(!newTag.isEmpty())
    {
        m_resource.addTag(Nepomuk::Tag(newTag));
    }
}

void NepomukHandler::addTags(const QStringList &newTags)
{
    foreach (const QString &newTag, newTags)
    {
        if (!newTag.isEmpty())
        {
            m_resource.addTag(Nepomuk::Tag(newTag));
        }
    }
}

void NepomukHandler::removeTag(const QString &oldTag)
{
    if (!oldTag.isEmpty())
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
    {
        list.append(tag.genericLabel());
    }
    return list;
}

void NepomukHandler::setProperty(const QUrl &uri, const Nepomuk::Variant &value)
{
    m_resource.setProperty(uri, value);
}

void NepomukHandler::saveFileProperties()
{
    Nepomuk::Resource srcFileRes(m_transfer->source(), Nepomuk::Vocabulary::NFO::RemoteDataObject());
    srcFileRes.setProperty(Nepomuk::Vocabulary::NIE::url(), m_transfer->source());
    saveFileProperties(m_resource);
}

void NepomukHandler::saveFileProperties(const Nepomuk::Resource &res)
{
    Nepomuk::Resource srcFileRes(m_transfer->source(), Nepomuk::Vocabulary::NFO::RemoteDataObject());
    Nepomuk::Resource m_res = res;
    m_res.setProperty(Soprano::Vocabulary::Xesam::originURL(), Nepomuk::Variant(m_transfer->source()));
    m_res.setProperty(Soprano::Vocabulary::Xesam::size(), Nepomuk::Variant(m_transfer->totalSize()));
    m_res.setProperty(Nepomuk::Vocabulary::NDO::copiedFrom(), srcFileRes);
    m_res.setProperty(Nepomuk::Vocabulary::NIE::url(), m_transfer->dest());

    /*Nepomuk::Resource downloadEventRes(QUrl(), Nepomuk::Vocabulary::NDO::DownloadEvent());
    downloadEventRes.addProperty(Nepomuk::Vocabulary::NUAO::involves(), m_res);
    downloadEventRes.addProperty(Nepomuk::Vocabulary::NUAO::start(), m_downloadJobStartTime);*/
}

bool NepomukHandler::isValid() const
{
    bool valid = QFile::exists(m_destination.pathOrUrl());
    valid = valid && m_resource.isValid();

    return valid;
}

void NepomukHandler::deinit()
{
    if (!isValid())
    {
        m_resource.remove();
    }
}

#include "nepomukhandler.moc"
