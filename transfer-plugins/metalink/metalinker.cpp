/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "metalinker.h"

#include <kdebug.h>
#include <kcodecs.h>
#include <kio/job.h>

#include <QDomElement>

bool MlinkFileData::isValidNameAttribute() const
{
    if (fileName.isEmpty()) {
        kError(5001) << "Name attribute of Metalink::File is empty.";
        return false;
    }

    if (fileName.contains(QRegExp("$(\\.\\.?)?/")) || fileName.contains("/../") || fileName.endsWith("/..")) {
        kError(5001) << "Name attribute of Metalink::File contains directory traversal directives:" << fileName;
        return false;
    }

    return true;
}

Metalinker::Metalinker()
{
}

QList<MlinkFileData> Metalinker::parseMetalinkFile(const QByteArray &data)
{
    QList<MlinkFileData> fileData;
    QDomDocument doc;

    if(!doc.setContent( data ))
    {
        kDebug(5001) << "Metalinker::parseMetalinkFile: unable to read the xml file";
        return fileData;
    }
    QDomNodeList files = doc.documentElement().
        elementsByTagName("files").
        item(0).toElement().elementsByTagName("file");

    kDebug(5001) << files.length() << " <file> tags found";

    QStringList fileNames;
    for( uint i=0 ; i < files.length() ; ++i )
    {
        QDomNode file = files.item(i);
        MlinkFileData data;
        data.fileName = QUrl::fromPercentEncoding(file.toElement().attribute("name").toAscii());
        kDebug(5001) << "filename: "<< data.fileName;
        if (!data.isValidNameAttribute()) {
            fileData.clear();
            return fileData;
        }

        if (fileNames.contains(data.fileName)) {
            kError(5001) << "Metalink::File name" << data.fileName << "exists multiple times.";
            fileData.clear();
            return fileData;
        }
        fileNames << data.fileName;

        QDomNodeList hashes = file.toElement().
            elementsByTagName("verification").
            item(0).toElement().elementsByTagName("hash");

        for( uint j=0 ; j < hashes.length() ; ++j )
        {
            QDomNode hash = hashes.item(j);
            if (hash.toElement().attribute("type") == "md5")
                data.md5 = hash.toElement().text();
            if (hash.toElement().attribute("type") == "sha256")
                data.sha256 = hash.toElement().text();
        kDebug(5001) << "md5 hash: "<< data.md5;
        kDebug(5001) << "sha256 hash: "<< data.sha256;

        }

        QDomNodeList urls = file.toElement().
            elementsByTagName("resources").
            item(0).toElement().elementsByTagName("url");

        for( uint k=0 ; k < urls.length() ; ++k )
        {
            QDomNode url = urls.item(k);
            data.urls << KUrl(url.toElement().text());
            kDebug(5001) << "url: "<< url.toElement().text();
        }

        fileData << data;
        kDebug(5001) << fileData.size() << " files Data";
    }

    return fileData;
}

bool Metalinker::verifyMD5(QIODevice& file, const QString& md5)
{
    KMD5 context;
    context.update(file);
    return context.verify( md5.toAscii() );
}
