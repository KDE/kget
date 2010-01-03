/* This file is part of the KDE project

   Copyright (C) 2008 Javier Goday <jgoday @ gmail.com>
   First Url regular expression taken from urlview tool by Michael Elkins <me@cs.hmc.edu>.
   Regular expression improved by FiNex.
   Improvements to regular expression and slotReadFile by Frantisek Ziacik

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
// static QString REGULAR_EXPRESSION = "((http|https|ftp|ftps)+([\\:\\w\\d:#@%/;$()~_?\\+-=\\\\.&])*)";
static QString REGULAR_EXPRESSION = "(\\w+[:]//)?(((([\\w-]+[.]){1,}(ac|ad|ae|af|ag|ai|al|am|an|ao|aq|ar|as|at|au|aw|az|ba|bb|bd|be|bf|bg|bh|bi|bj|bm|bn|bo|br|bs|bt|bv|bw|by|bz|ca|cc|cd|cf|cg|ch|ci|ck|cl|cm|cn|co|com|cr|cs|cu|cv|cx|cy|cz|de|dj|dk|dm|do|dz|ec|edu|ee|eg|eh|er|es|et|eu|fi|fj|fk|fm|fo|fr|ga|gd|ge|gf|gg|gh|gi|gl|gm|gn|gov|gp|gq|gr|gs|gt|gu|gw|gy|hk|hm|hn|hr|ht|hu|id|ie|il|im|in|int|io|iq|ir|is|it|je|jm|jo|jp|ke|kg|kh|ki|km|kn|kp|kr|kw|ky|kz|la|lb|lc|li|lk|lr|ls|lt|lu|lv|ly|ma|mc|md|mg|mh|mil|mk|ml|mm|mn|mo|mp|mq|mr|ms|mt|mu|mv|mw|mx|my|mz|na|nc|ne|net|nf|ng|ni|nl|no|np|nr|nt|nu|nz|om|org|pa|pe|pf|pg|ph|pk|pl|pm|pn|pr|ps|pt|pw|py|qa|re|ro|ru|rw|sa|sb|sc|sd|se|sg|sh|si|sj|sk|sl|sm|sn|so|sr|sv|st|sy|sz|tc|td|tf|tg|th|tj|tk|tm|tn|to|tp|tr|tt|tv|tw|tz|ua|ug|uk|um|us|uy|uz|va|vc|ve|vg|vi|vn|vu|wf|ws|ye|yt|yu|za|zm|zw|aero|biz|coop|info|museum|name|pro|travel))|([0-9]+[.][0-9]+[.][0-9]+[.][0-9]+)))([:][0-9]*)?([?/][\\w~#\\-;%?@&=/.+]*)?(?!\\w)";

LinkImporter::LinkImporter(const KUrl &url, QObject *parent) : QThread(parent),
    m_url(url),
    m_transfers(),
    m_tempFile()
{
}

LinkImporter::LinkImporter(QObject *parent) : QThread(parent),
    m_url(),
    m_transfers(),
    m_tempFile()
{
}

LinkImporter::~LinkImporter()
{
}

void LinkImporter::checkClipboard(const QString &clipboardContent)
{
    QRegExp rx(REGULAR_EXPRESSION);

    int regexPos = 0;

    while ((regexPos = rx.indexIn(clipboardContent, regexPos)) > -1) {
        QString link = rx.capturedTexts()[0];

        addTransfer(link);

        regexPos += rx.matchedLength();
    }

    emit finished();
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
    QFile file(url.toLocalFile());

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    quint64 size = file.size();
    quint64 position = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        int regexPos = 0;
        quint64 lastPosition = position;

        while ((regexPos = rx.indexIn(line, regexPos)) > -1) {
            QString link = rx.capturedTexts()[0];

            addTransfer(link);

            regexPos += rx.matchedLength();
            position = lastPosition + regexPos;

            emit progress(position * 100 / size);
        }

        position += line.size();

        emit progress(position * 100 / size);
    }

    if(!m_url.isLocalFile()) {
        file.remove();
    }
}

void LinkImporter::addTransfer(QString &link)
{
    KUrl auxUrl;
    
    if (link.contains("://")) {
        auxUrl = KUrl(link);
    } else {
        auxUrl = KUrl(QString("http://") + link);
    }

    if(!link.isEmpty() && auxUrl.isValid() && m_transfers.indexOf(link) < 0 &&
       !auxUrl.scheme().isEmpty() && !auxUrl.host().isEmpty()) {
        m_transfers << link;
    }
}

#include "linkimporter.moc"
