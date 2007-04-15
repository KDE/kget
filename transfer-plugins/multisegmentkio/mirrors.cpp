/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "mirrors.h"
#include "multisegkiosettings.h"

#include <KDebug>

mirror::mirror()
{
    m_search_engine = MultiSegKioSettings::searchEnginesUrlList().takeFirst();
}

void mirror::slotData(KIO::Job *, const QByteArray& data)
{
    if (data.size() == 0)
        return;
    m_data.append(data);
}

void mirror::slotResult( KJob *job )
{
    kDebug(5001) << "mirror::slotResult() " << endl;
    if (job->error())
        return;
    m_job = 0;
    if ( EventLoop.isRunning() )
        EventLoop.quit();
}

void mirror::URLRequest(const KUrl &url)
{
    KUrl search(m_search_engine.replace("${filename}",url.fileName()));
    m_job = KIO::get(search,false,false);
    connect(m_job,SIGNAL(data(KIO::Job*,const QByteArray &)),
               SLOT(slotData(KIO::Job*, const QByteArray& )));
    connect(m_job,SIGNAL(result(KJob *)),
               SLOT(slotResult(KJob * )));
}


QList<KUrl>  mirror::search(const KUrl &url)
{
    kDebug(5001) << "mirror::search() " << endl;

    m_Urls << url;
    URLRequest (url);

    EventLoop.exec();

    QString str(m_data);

    int start = 0, posOfTagA = 0, posOfTagHref = 0, hrefEnd = 0;

    while ((posOfTagA = str.indexOf("<a" , start, Qt::CaseInsensitive)) != -1 )
    {
    posOfTagHref = str.indexOf("href=\"", posOfTagA, Qt::CaseInsensitive);
    hrefEnd = str.indexOf("\"",posOfTagHref + 6,Qt::CaseInsensitive);
    QString u = str.mid(posOfTagHref + 6, (hrefEnd - posOfTagHref -6));
    start = hrefEnd + 1;
        if ( u.endsWith( url.fileName() ) )
        {
            m_Urls << KUrl(u);
            kDebug(5001) << "url: " << u << endl;
        }
    }

    return m_Urls;
}

QList<KUrl> MirrorSearch ( const KUrl &url )
{
    mirror searcher;
    return searcher.search(url);
}

#include "mirrors.moc"
