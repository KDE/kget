/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDomElement>
#include <QFile>

#include <kdebug.h>
#include <kio/job.h>

#include "metalinker.h"


Metalinker::Metalinker()
{
}

QList<MlinkFileData> Metalinker::parseMetalinkFile(const KUrl& url)
{
    kDebug(5001) << "Metalinker::parseMetalinkFile " << url << endl;
    QList<MlinkFileData> fileData;
    QFile file( url.fileName() );
    QDomDocument doc;

    if(!doc.setContent(&file))
        return fileData;

    QDomNodeList metalink = doc.documentElement().
        elementsByTagName("metalink");
    kDebug(5001) << metalink.length() << " <metalink> tags found" << endl;

    QDomNodeList files = metalink.item(0).toElement().
        elementsByTagName("files");
    kDebug(5001) << files.length() << " <files> tags found" << endl;

    QDomNodeList _file = files.item(0).toElement().
        elementsByTagName("file");
    kDebug(5001) << _file.length() << " <file> tags found" << endl;

//     QDomNodeList files = doc.documentElement().
//         elementsByTagName("metalink").
//         item(0).toElement().elementsByTagName("files").
//         item(0).toElement().elementsByTagName("file");

//     kDebug(5001) << files.length() << " <file> tags found" << endl;

    for( uint i=0 ; i < files.length() ; ++i )
    {
        QDomNode file = files.item(i);
        MlinkFileData data;
        data.fileName = file.toElement().attribute("name");

        QDomNodeList hashes = file.toElement().
            elementsByTagName("verification").
            item(0).toElement().elementsByTagName("hash");

        for( uint j=0 ; i < hashes.length() ; ++j )
        {
            QDomNode hash = hashes.item(i);
            if (hash.toElement().attribute("type") == "md5")
                data.md5 = hash.toElement().text();
            if (hash.toElement().attribute("type") == "sha256")
                data.sha256 = hash.toElement().text();
        }

        QDomNodeList urls = file.toElement().
            elementsByTagName("resources").
            item(0).toElement().elementsByTagName("url");

        for( uint j=0 ; i < urls.length() ; ++j )
        {
            QDomNode url = urls.item(i);
            data.urls << KUrl(url.toElement().text());
        }

        fileData << data;
    }

    return fileData;
}
