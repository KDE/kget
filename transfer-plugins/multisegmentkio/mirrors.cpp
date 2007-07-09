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

void mirror::search(const KUrl &url, QObject *receiver, const char *member)
{
    kDebug(5001) << "mirror::search() " << endl;

    m_url = url;
    m_Urls << m_url;

    KUrl search(m_search_engine.replace("${filename}",m_url.fileName()));
    m_job = KIO::get(search,false,false);
    connect(m_job,SIGNAL(data(KIO::Job*,const QByteArray &)),
               SLOT(slotData(KIO::Job*, const QByteArray& )));
    connect(m_job,SIGNAL(result(KJob *)),
               SLOT(slotResult(KJob * )));
    connect(this,SIGNAL(urls(QList<KUrl>&)),receiver,member);

}

void mirror::slotData(KIO::Job *, const QByteArray& data)
{
    kDebug(5001) << "mirror::slotData() " << endl;
    if (data.size() == 0)
        return;
    m_data.append(data);
}

void mirror::slotResult( KJob *job )
{
    kDebug(5001) << "mirror::slotResult() " << endl;
    m_job = 0;
    if( job->error() )
    {
        deleteLater();
        return;
    }
    QString str(m_data);

    int start = 0, posOfTagA = 0, posOfTagHref = 0, hrefEnd = 0;

    while ((posOfTagA = str.indexOf("<a " , start, Qt::CaseInsensitive)) != -1 )
    {
        posOfTagHref = str.indexOf("href=\"", posOfTagA, Qt::CaseInsensitive);
        hrefEnd = str.indexOf("\"",posOfTagHref + 6,Qt::CaseInsensitive);
        QString u = str.mid(posOfTagHref + 6, (hrefEnd - posOfTagHref -6));

        start = hrefEnd + 1;
            if ( u.endsWith( m_url.fileName() ) )
            {
                m_Urls << KUrl(u);
                kDebug(5001) << "url: " << u << endl;
            }
    }

    if (m_Urls.size() > 1)
        emit urls(m_Urls);
    deleteLater();
}

void MirrorSearch ( const KUrl &url, QObject *receiver, const char *member )
{
    mirror *searcher = new mirror();
    searcher->search(url, receiver, member);
}

#include "mirrors.moc"
