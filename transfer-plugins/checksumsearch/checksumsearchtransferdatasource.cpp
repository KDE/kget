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

#include "checksumsearchtransferdatasource.h"
#include "checksumsearch.h"
#include "checksumsearchsettings.h"
#include "kget_debug.h"

#include <QFile>

#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <KStandardDirs>

ChecksumSearchController ChecksumSearchTransferDataSource::s_controller;

ChecksumSearchController::ChecksumSearchController(QObject *parent)
  : QObject(parent)
{
}

ChecksumSearchController::~ChecksumSearchController()
{
}

void ChecksumSearchController::registerSearch(ChecksumSearchTransferDataSource *search, const QUrl &baseUrl)
{
    if (m_finished.contains(baseUrl)) {
        qCDebug(KGET_DEBUG) << "Already downloaded" << baseUrl;
        const QUrl urlToFile = m_finished[baseUrl];
        if (!urlToFile.isEmpty()) {
            search->gotBaseUrl(m_finished[baseUrl]);
        }
    } else {
        const bool alreadySearchedFor = m_searches.contains(baseUrl);
        if (!m_searches.contains(baseUrl, search)) {
            m_searches.insert(baseUrl, search);

            if (alreadySearchedFor) {
                qCDebug(KGET_DEBUG) << "Search already started for" << baseUrl;
                return;
            }
            qCDebug(KGET_DEBUG) << "Creating download for" << baseUrl;
            static int files = 0;

            const QUrl dest = QUrl::fromLocalFile(KStandardDirs::locateLocal("appdata", "checksumsearch/") + QString::number(files++));
            if (QFile::exists(dest.toLocalFile())) {
                KIO::Job *del = KIO::del(dest, KIO::HideProgressInfo);
                KIO::NetAccess::synchronousRun(del, nullptr);
            }

            if (baseUrl.scheme() != "ftp" && baseUrl.scheme() != "sftp") {
                qCDebug(KGET_DEBUG) << "Downloading" << baseUrl;
                KIO::FileCopyJob *job = KIO::file_copy(baseUrl, dest, -1, KIO::HideProgressInfo);
                job->addMetaData("errorPage", "false");
                connect(job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
                m_jobs[job] = qMakePair(baseUrl, dest);
            } else {
                qCDebug(KGET_DEBUG) << "ftp, doing a listjob";
                KIO::ListJob *job = KIO::listDir(baseUrl, KIO::HideProgressInfo);
                connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)), this, SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
                connect(job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
                m_jobs[job] = qMakePair(baseUrl, dest);
            }
        }
    }
}

void ChecksumSearchController::unregisterSearch(ChecksumSearchTransferDataSource *search, const QUrl &baseUrl)
{
    if (baseUrl.isEmpty()) {
        const QList<QUrl> keys = m_searches.keys(search);
        foreach (const QUrl &key, keys) {
            m_searches.remove(key, search);
        }
    } else {
        m_searches.remove(baseUrl, search);
    }
}

void ChecksumSearchController::slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    qCDebug(KGET_DEBUG);

    if (!m_jobs.contains(job)) {
        return;
    }

    const QUrl baseUrl = m_jobs[job].first;
    const QUrl urlToFile = m_jobs[job].second;
    QFile file(urlToFile.toLocalFile());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCDebug(KGET_DEBUG) << "Could not open file" << urlToFile;
    }

    QTextStream out(&file);
    foreach (const KIO::UDSEntry &entry, entries) {
        if (!entry.isDir()) {
            out << entry.stringValue(KIO::UDSEntry::UDS_NAME) << '\n';
        }
    }
    file.close();
}

void ChecksumSearchController::slotResult(KJob *job)
{
    qCDebug(KGET_DEBUG);

    if (!m_jobs.contains(job)) {
        return;
    }

    const QUrl baseUrl = m_jobs[job].first;
    const QUrl urlToFile = m_jobs[job].second;
    m_jobs.remove(job);
    if (job->error()) {
        qCDebug(KGET_DEBUG) << "Error while getting baseurl:" << baseUrl << job->error() << job->errorString();
        m_finished[baseUrl] = QUrl();
        return;
    }

    m_finished[baseUrl] = urlToFile;

    const QList<ChecksumSearchTransferDataSource*> searches = m_searches.values(baseUrl);
    m_searches.remove(baseUrl);
    foreach (ChecksumSearchTransferDataSource *search, searches) {
        search->gotBaseUrl(urlToFile);
    }
}

ChecksumSearchTransferDataSource::ChecksumSearchTransferDataSource(const QUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent)
{
}

ChecksumSearchTransferDataSource::~ChecksumSearchTransferDataSource()
{
    s_controller.unregisterSearch(this, m_sourceUrl.adjusted(QUrl::RemoveFilename));
}

void ChecksumSearchTransferDataSource::start()
{
    qCDebug(KGET_DEBUG);

    const QUrl baseUrl = m_sourceUrl.adjusted(QUrl::RemoveFilename);
    s_controller.registerSearch(this, baseUrl);
}

void ChecksumSearchTransferDataSource::gotBaseUrl(const QUrl &urlToFile)
{
    QFile file(urlToFile.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KGET_DEBUG) << "Could not open file" << urlToFile;
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    QStringList changes = ChecksumSearchSettings::self()->searchStrings();
    QList<int> modes = ChecksumSearchSettings::self()->urlChangeModeList();
    QStringList types = ChecksumSearchSettings::self()->checksumTypeList();

    QList<QUrl> urls;

    for (int i = 0, k = 0; i < changes.size(); ++i) {
        const ChecksumSearch::UrlChangeMode mode = static_cast<ChecksumSearch::UrlChangeMode>(modes.at(i));
        const QUrl source = ChecksumSearch::createUrl(m_sourceUrl, changes.at(i), mode);
        if (data.indexOf(source.fileName().toAscii()) != -1) {
            urls.append(source);
            ++k;
        } else {
            types.removeAt(k);
        }
    }

    qCDebug(KGET_DEBUG) << "Creating Checksumsearch for" << urls.count() << "urls.";

    if (urls.count() && types.count()) {
        ChecksumSearch *search = new ChecksumSearch(urls, m_sourceUrl.fileName(), types);

        connect(search, SIGNAL(data(QString,QString)), this, SIGNAL(data(QString,QString)));
    }
}

void ChecksumSearchTransferDataSource::stop()
{
    qCDebug(KGET_DEBUG);
}

void ChecksumSearchTransferDataSource::addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    Q_UNUSED(segmentSize)
    Q_UNUSED(segmentRange)
    qCDebug(KGET_DEBUG);
}


