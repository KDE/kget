/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "mirrors.h"
#include "mirrorsearchsettings.h"

#include "kget_debug.h"
#include <QDebug>

mirror::mirror()
{
    if( !MirrorSearchSettings::searchEnginesUrlList().isEmpty())
       m_search_engine = MirrorSearchSettings::searchEnginesUrlList().takeFirst();
}

void mirror::search(const QUrl &url, QObject *receiver, const char *member)
{
    qCDebug(KGET_DEBUG);

    m_url = url;
    if (m_url.path() != m_url.fileName())
    {
        m_Urls << m_url;
    }

    search(m_url.fileName(),receiver,member);
}

void mirror::search(const QString &fileName, QObject *receiver, const char *member)
{
    qCDebug(KGET_DEBUG);

    QUrl search(m_search_engine.replace("${filename}",fileName));
    m_job = KIO::get(search, KIO::NoReload, KIO::HideProgressInfo);
    connect(m_job,&KIO::TransferJob::data,
               this, &mirror::slotData);
    connect(m_job,&KJob::result,
               this, &mirror::slotResult);
    connect(this,SIGNAL(urls(QList<QUrl>&)),receiver,member);
}

void mirror::slotData(KIO::Job *, const QByteArray& data)
{
    qCDebug(KGET_DEBUG);
    if (data.size() == 0)
        return;
    m_data.append(data);
}

void mirror::slotResult( KJob *job )
{
    qCDebug(KGET_DEBUG);
    m_job = nullptr;
    int minUrlsNeeded = static_cast<int>(!m_Urls.isEmpty());

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
            if ( u.endsWith( '/' + m_url.fileName() ) )
            {
                m_Urls << QUrl(u);
                qCDebug(KGET_DEBUG) << "url: " << u;
            }
    }

    if (m_Urls.size() > minUrlsNeeded)
        Q_EMIT urls(m_Urls);
    deleteLater();
}

void MirrorSearch ( const QUrl &url, QObject *receiver, const char *member )
{
    mirror *searcher = new mirror();
    searcher->search(url, receiver, member);
}

void MirrorSearch ( const QString &fileName, QObject *receiver, const char *member )
{
    mirror *searcher = new mirror();
    searcher->search(fileName, receiver, member);
}


