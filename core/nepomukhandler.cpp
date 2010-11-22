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
#include "transfergroup.h"
#include "kget.h"
#include "nepomukcontroller.h"
#include "verifier.h"

#include "nfo.h"
#include "ndo.h"
#include "nie.h"
#include <Soprano/Vocabulary/Xesam>
#include <Nepomuk/Variant>
#include <Nepomuk/Tag>

NepomukHandler::NepomukHandler(Transfer *transfer)
  : QObject(transfer),
    m_transfer(transfer)
{
}

NepomukHandler::~NepomukHandler()
{
}

KFileItemList NepomukHandler::fileItems() const
{
    KFileItemList fileItems;
    foreach (const KUrl &destination, m_transfer->files()) {
        fileItems << KFileItem(KFileItem::Unknown, KFileItem::Unknown, destination, true);
    }
    return fileItems;
}

void NepomukHandler::setProperties(const QList<QPair<QUrl, Nepomuk::Variant> > &properties, const QList<KUrl> &files)
{
    QList<KUrl> usedFiles = (files.isEmpty() ? m_transfer->files() : files);
    KGet::nepomukController()->setProperties(usedFiles, properties);
}

void NepomukHandler::saveFileProperties()
{
    const QList<KUrl> destinations = m_transfer->files();

    QPair<QUrl, Nepomuk::Variant> property = qMakePair(Nepomuk::Vocabulary::NIE::url(), Nepomuk::Variant(m_transfer->source()));
    KGet::nepomukController()->setProperty(QList<KUrl>() << m_transfer->source(), property, Nepomuk::Vocabulary::NFO::RemoteDataObject());
    Nepomuk::Resource srcFileRes(m_transfer->source(), Nepomuk::Vocabulary::NFO::RemoteDataObject());

    foreach (const KUrl &destination, destinations) {
        //set all the properties
        QList<QPair<QUrl, Nepomuk::Variant> > properties;
        properties.append(qMakePair(Nepomuk::Vocabulary::NIE::url(), Nepomuk::Variant(destination)));
        properties.append(qMakePair(Nepomuk::Vocabulary::NDO::copiedFrom(), Nepomuk::Variant(srcFileRes)));
        properties.append(qMakePair(Soprano::Vocabulary::Xesam::originURL(), Nepomuk::Variant(m_transfer->source().url())));

        //just adds one Hash as otherwise it would not be clear in KFileMetaDataWidget which hash belongs
        //to which algorithm
        Verifier *verifier = m_transfer->verifier(destination);
        if (verifier) {
            const QPair<QString, QString> checksum = verifier->availableChecksum(Verifier::Strongest);
            QString hashType = checksum.first;
            const QString hash = checksum.second;
            if (!hashType.isEmpty() && !hash.isEmpty()) {
                //use the offical names, i.e. uppercase and in the case of SHA with a '-'
                hashType = hashType.toUpper();
                if (hashType.contains(QRegExp("^SHA\\d+"))) {
                    hashType.insert(3, '-');
                }
                properties.append(qMakePair(Nepomuk::Vocabulary::NFO::hashAlgorithm(), Nepomuk::Variant(hashType)));
                properties.append(qMakePair(Nepomuk::Vocabulary::NFO::hashValue(), Nepomuk::Variant(hash)));
            }
        }

        KGet::nepomukController()->setProperties(QList<KUrl>() << destination, properties);
    }

    //set the tags of the group
    KGet::nepomukController()->addTags(destinations, m_transfer->group()->tags());

    /*Nepomuk::Resource downloadEventRes(QUrl(), Nepomuk::Vocabulary::NDO::DownloadEvent());
    downloadEventRes.addProperty(Nepomuk::Vocabulary::NUAO::involves(), m_res);
    downloadEventRes.addProperty(Nepomuk::Vocabulary::NUAO::start(), m_downloadJobStartTime);*/
}

void NepomukHandler::deinit()
{
    KGet::nepomukController()->removeResource(m_transfer->files());
}

#include "nepomukhandler.moc"
