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

#include <Nepomuk2/Variant>
#include <Nepomuk2/Tag>
#include <Nepomuk2/Vocabulary/NDO>
#include <Nepomuk2/Vocabulary/NFO>
#include <Nepomuk2/Vocabulary/NIE>

using namespace Nepomuk2::Vocabulary;

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

void NepomukHandler::setProperties(const QList<QPair<QUrl, Nepomuk2::Variant> > &properties, const QList<KUrl> &files)
{
    QList<KUrl> usedFiles = (files.isEmpty() ? m_transfer->files() : files);
    QList<QUrl> fileUrls;
    
    foreach(const KUrl &usedFile, usedFiles) {
        fileUrls.push_back(usedFile);
    }
    
    KGet::nepomukController()->setProperties(fileUrls, properties);
}

void NepomukHandler::saveFileProperties()
{
    QList<QUrl> destinations;

    foreach(const KUrl &url, m_transfer->files()) {
        destinations.push_back(url);
    }

    const QUrl src = m_transfer->source();
    const QUrl srcType = (src.isLocalFile() ? NFO::FileDataObject() : NFO::RemoteDataObject());
    QPair<QUrl, Nepomuk2::Variant> property = qMakePair(NIE::url(), Nepomuk2::Variant(src));
    KGet::nepomukController()->setProperty(QList<QUrl>() << src, property, srcType);
    Nepomuk2::Resource srcFileRes(src, srcType);

    foreach (const QUrl &destination, destinations) {
        //set all the properties
        QList<QPair<QUrl, Nepomuk2::Variant> > properties;
        properties.append(qMakePair(NIE::url(), Nepomuk2::Variant(destination)));
        properties.append(qMakePair(NDO::copiedFrom(), Nepomuk2::Variant(srcFileRes)));

        Verifier *verifier = m_transfer->verifier(destination);
        if (verifier) {
            const QList<Checksum> checksums = verifier->availableChecksums();
            QList<Nepomuk2::Variant> hashes;
            foreach (const Checksum &checksum, checksums) {
                QString hashType = Verifier::cleanChecksumType(checksum.first);
                const QString hash = checksum.second;
                if (!hashType.isEmpty() && !hash.isEmpty()) {
                    Nepomuk2::Resource hashRes(hash, NFO::FileHash());
                    hashRes.addProperty(NFO::hashAlgorithm(), hashType);
                    hashRes.addProperty(NFO::hashValue(), hash);
                    hashRes.setLabel(hashType);
                    hashes << hashRes;
                }
            }
            if (!hashes.isEmpty()) {
                properties.append(qMakePair(NFO::hasHash(), Nepomuk2::Variant(hashes)));
            }
        }

        KGet::nepomukController()->setProperties(QList<QUrl>() << destination, properties);
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
