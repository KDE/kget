/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday @ gmail.com>
   First Url regular expression taken from urlview tool by Michael Elkins <me@cs.hmc.edu>.
   Regular expression improved by FiNex.
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "linkimporter.h"

#include <QFile>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QDir>
#include <QTextStream>
#include <QStringList>

#include <KDebug>
#include <KLocale>
#include <kio/copyjob.h>
#include <kio/netaccess.h>

//static QString REGULAR_EXPRESSION = "(((https?|ftp|gopher)://|(mailto|file|news):)[^’ <>\"]+|(www|web|w3).[-a-z0-9.]+)[^’ .,;<>\":]";
static QString REGULAR_EXPRESSION = "((http|https|ftp|ftps)+([\\:\\w\\d:#@%/;$()~_?\\+-=\\\\.&])*)";

LinkImporter::LinkImporter(const KUrl &url, QObject *parent) : QThread(parent),
    m_url(url),
    m_transfers(),
    m_tempFile()
{
}

LinkImporter::~LinkImporter()
{
}

void LinkImporter::run()
{
    if(!m_url.isLocalFile() && !m_tempFile.isEmpty()) {
        slotReadFile(KUrl(m_tempFile));
    }
    else {
        slotReadFile(m_url);
    }

    emit finished();
}

void LinkImporter::copyRemoteFile()
{
    m_tempFile = QString("%1/%2.tmp").arg(QDir::tempPath()).arg("importer_aux");

    KUrl aux(m_tempFile);
    KIO::CopyJob *job = KIO::copy(m_url, aux, KIO::HideProgressInfo);

    QMap<QString, QString> metaData;
    bool ok = KIO::NetAccess::synchronousRun(job, 0, 0, 0, &metaData);
    if(!ok) {
        emit error(ki18n("Error trying to get %1").subs(m_url.url()));
    }
}

void LinkImporter::slotReadFile(const QUrl &url)
{
    QRegExp rx(REGULAR_EXPRESSION);

    QFile file(url.path());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    quint64 size = file.size();
    quint64 readed = 0;
    while (!in.atEnd()) {
        QString line = in.readLine(200);
        readed += 200;

        rx.indexIn(line);

        foreach(const QString &link, rx.capturedTexts()) {
            QUrl auxUrl(link);

            if(!link.isEmpty() && auxUrl.isValid() && m_transfers.indexOf(link) < 0 && 
                !auxUrl.scheme().isEmpty() && !auxUrl.host().isEmpty()) {
                m_transfers << link;
            }
        }
        emit progress(readed * 100 / size);
    }
    if(!m_url.isLocalFile()) {
        file.remove();
    }
}

#include "linkimporter.moc"
