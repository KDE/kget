/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "checksumsearch.h"

#include "core/verifier.h"

#include <QFile>
#include <QFileInfo>

#include <KDebug>
#include <KLocale>

const QStringList ChecksumSearch::URLCHANGEMODES = (QStringList() << i18n("Append") << i18n("Replace file") << i18n("Replace file-ending"));

ChecksumSearch::ChecksumSearch(KUrl src, QString fileName, QString type)
  : m_copyJob(0),
    m_src(src),
    m_fileName(fileName),
    m_type(type),
    m_isEmpty(m_type.isEmpty())
{
    m_copyJob = KIO::get(m_src, KIO::Reload, KIO::HideProgressInfo);
    connect(m_copyJob, SIGNAL(data(KIO::Job *,const QByteArray &)), SLOT(slotData(KIO::Job *, const QByteArray &)));
    connect(m_copyJob, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)));
}

ChecksumSearch::~ChecksumSearch()
{
    if (m_copyJob)
    {
        m_copyJob->kill(KJob::Quietly);
    }
}

void ChecksumSearch::slotData(KIO::Job *job, const QByteArray& data)
{
    Q_UNUSED(job);
    kDebug(5001);

    if (m_dataBA.size() > 5 * 1024)
    {
        m_copyJob->kill(KJob::EmitResult);
    }
    else
    {
        m_dataBA.append(data);
    }
}

void ChecksumSearch::slotResult(KJob * job)
{
    kDebug(5001);

    if (m_copyJob->isErrorPage())
    {
        kDebug(5001) << "The requested url does not exist:" << m_src.pathOrUrl();
        m_copyJob = 0;
        emit finished(m_src);
        return;
    }

    switch (job->error())
    {
        case 0://The download has finished
        {
            kDebug(5001) << "Correctly downloaded" << m_src.pathOrUrl();
            break;
        }

        default:
            kDebug(5001) << "There was error" << job->error() << "while downloading" << m_src.pathOrUrl();
            break;
    }

    m_copyJob = 0;
    m_data = QString(m_dataBA);
    m_dataBA.clear();
    parseDownload();
}

void ChecksumSearch::parseDownload()
{
    //no type has been specified
    if (m_type.isEmpty())
    {
        parseDownloadEmpty();
        return;
    }

    const int length = Verifier::diggestLength(m_type);

    const QString patternChecksum = QString("\\w{%1}").arg(length);
    QRegExp rxChecksum(patternChecksum);
    QString hash;

    //find the correct line
    const QStringList lines = m_data.split('\n');
    foreach (const QString &line, lines)
    {
        if (line.contains(m_fileName, Qt::CaseInsensitive))
        {
            if (rxChecksum.indexIn(line) > -1)
            {
                hash = rxChecksum.cap(0);
                kDebug(5001) << "Found hash: " << hash;
                emit data(m_type, hash);
            }
        }
    }

    //nothing found yet, so simply search for a word in the whole data that has the correct length
    if (hash.isEmpty() && (rxChecksum.indexIn(m_data) > -1))
    {
        QString hash = rxChecksum.cap(0);
        kDebug(5001) << "Found hash:" << hash;
        emit data(m_type, hash);
    }

    //only emit finished if type was specified, otherwise parseDownloadEmpty has to handle this
    if (!m_isEmpty)
    {
        emit finished(m_src);
    }
}

void ChecksumSearch::parseDownloadEmpty()
{
    const QStringList lines = m_data.split('\n');
    const QStringList supportedTypes = Verifier::supportedVerficationTypes();
    foreach (const QString &type, supportedTypes)
    {
        if (m_data.contains(type, Qt::CaseInsensitive))
        {
            m_type = type;
            parseDownload();
        }
    }

    emit finished(m_src);
}

KUrl ChecksumSearch::createUrl(const KUrl &src, const QString &change, ChecksumSearch::UrlChangeMode mode)
{
    if (!src.isValid() || change.isEmpty())
    {
        return KUrl();
    }

    KUrl url;
    if (mode == Append)
    {
        url = KUrl(src.pathOrUrl() + change);
    }
    else if (mode == ReplaceFile)
    {
        KUrl temp = src.upUrl();
        temp.addPath(change);
        url = temp;
    }
    else if (mode == ReplaceEnding)
    {
        QString fileName = src.fileName();
        int index = fileName.lastIndexOf('.');
        if (index > -1)
        {
            fileName = fileName.left(index) + change;
            KUrl temp = src.upUrl();
            temp.addPath(fileName);
            url = temp;
        }
    }

    return url;
}

#include "checksumsearch.moc"
