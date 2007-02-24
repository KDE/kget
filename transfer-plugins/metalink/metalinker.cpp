/* This file is part of the KDE project

   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDomElement>
#include <QFile>

#include <kio/job.h>
#include "metalinker.h"

Metalinker::Metalinker()
{
}

void Metalinker::parseMetalinkFile(const KUrl& url)
{
    QDomNodeList nodelist = readMetalinkFile(url).elementsByTagName("metalink");
}

QDomElement Metalinker::readMetalinkFile(const KUrl& url)
{
    if(url.protocol() == "file")
    {
        QFile file( url.fileName() );
        QDomDocument doc;

        if(doc.setContent(&file))
        {
            return doc.documentElement();
        }
    }
    else
    {
// i need to check the way konqueror plugin works.
// maybe it already give me the remote .metalink file or
// maybe i have to download it my self
// for now i'll comment the following code

//         KIO::TransferJob *job = KIO::get(url, false, false);
//         connect(job,SIGNAL(data(KIO::Job*,const QByteArray &)),
//                SLOT(slotData(KIO::Job*, const QByteArray& )));

        return QDomElement();
    }
}
