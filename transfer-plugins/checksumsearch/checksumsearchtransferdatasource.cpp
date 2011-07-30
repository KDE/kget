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

#include <QFile>

#include <KDebug>
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

void ChecksumSearchController::registerSearch(ChecksumSearchTransferDataSource *search, const KUrl &baseUrl)
{
    if (m_finished.contains(baseUrl)) {
        kDebug(5001) << "Already downloaded" << baseUrl;
        const KUrl urlToFile = m_finished[baseUrl];
        if (!urlToFile.isEmpty()) {
            search->gotBaseUrl(m_finished[baseUrl]);
        }
    } else {
        const bool alreadySearchedFor = m_searches.contains(baseUrl);
        if (!m_searches.contains(baseUrl, search)) {
            m_searches.insert(baseUrl, search);

            if (alreadySearchedFor) {
                kDebug(5001) << "Search already started for" << baseUrl;
                return;
            }
            kDebug(5001) << "Creating download for" << baseUrl;
            static int files = 0;

            const KUrl dest = KUrl(KStandardDirs::locateLocal("appdata", "checksumsearch/") + QString::number(files++));
            if (QFile::exists(dest.toLocalFile())) {
                KIO::Job *del = KIO::del(dest, KIO::HideProgressInfo);
                KIO::NetAccess::synchronousRun(del, 0);
            }

            if (baseUrl.protocol() != "ftp" && baseUrl.protocol() != "sftp") {
                kDebug(5001) << "Downloading" << baseUrl;
                KIO::FileCopyJob *job = KIO::file_copy(baseUrl, dest, -1, KIO::HideProgressInfo);
                job->addMetaData("errorPage", "false");
                connect(job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
                m_jobs[job] = qMakePair(baseUrl, dest);
            } else {
                kDebug(5001) << "ftp, doing a listjob";
                KIO::ListJob *job = KIO::listDir(baseUrl, KIO::HideProgressInfo);
                connect(job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)), this, SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)));
                connect(job, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
                m_jobs[job] = qMakePair(baseUrl, dest);
            }
        }
    }
}

void ChecksumSearchController::unregisterSearch(ChecksumSearchTransferDataSource *search, const KUrl &baseUrl)
{
    if (baseUrl.isEmpty()) {
        const QList<KUrl> keys = m_searches.keys(search);
        foreach (const KUrl &key, keys) {
            m_searches.remove(key, search);
        }
    } else {
        m_searches.remove(baseUrl, search);
    }
}

void ChecksumSearchController::slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries)
{
    kDebug(5001);

    if (!m_jobs.contains(job)) {
        return;
    }

    const KUrl baseUrl = m_jobs[job].first;
    const KUrl urlToFile = m_jobs[job].second;
    QFile file(urlToFile.toLocalFile());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        kDebug(5001) << "Could not open file" << urlToFile;
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
    kDebug(5001);

    if (!m_jobs.contains(job)) {
        return;
    }

    const KUrl baseUrl = m_jobs[job].first;
    const KUrl urlToFile = m_jobs[job].second;
    m_jobs.remove(job);
    if (job->error()) {
        kDebug(5001) << "Error while getting baseurl:" << baseUrl << job->error() << job->errorString();
        m_finished[baseUrl] = KUrl();
        return;
    }

    m_finished[baseUrl] = urlToFile;

    const QList<ChecksumSearchTransferDataSource*> searches = m_searches.values(baseUrl);
    m_searches.remove(baseUrl);
    foreach (ChecksumSearchTransferDataSource *search, searches) {
        search->gotBaseUrl(urlToFile);
    }
}

ChecksumSearchTransferDataSource::ChecksumSearchTransferDataSource(const KUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent)
{
}

ChecksumSearchTransferDataSource::~ChecksumSearchTransferDataSource()
{
    s_controller.unregisterSearch(this, m_sourceUrl.upUrl());
}

void ChecksumSearchTransferDataSource::start()
{
    kDebug(5001);

    const KUrl baseUrl = m_sourceUrl.upUrl();
    s_controller.registerSearch(this, baseUrl);
}

void ChecksumSearchTransferDataSource::gotBaseUrl(const KUrl &urlToFile)
{
    QFile file(urlToFile.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        kDebug(5001) << "Could not open file" << urlToFile;
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    QStringList changes = ChecksumSearchSettings::self()->searchStrings();
    QList<int> modes = ChecksumSearchSettings::self()->urlChangeModeList();
    QStringList types = ChecksumSearchSettings::self()->checksumTypeList();

    QList<KUrl> urls;

    for (int i = 0, k = 0; i < changes.size(); ++i) {
        const ChecksumSearch::UrlChangeMode mode = static_cast<ChecksumSearch::UrlChangeMode>(modes.at(i));
        const KUrl source = ChecksumSearch::createUrl(m_sourceUrl, changes.at(i), mode);
        if (data.indexOf(source.fileName().toAscii()) != -1) {
            urls.append(source);
            ++k;
        } else {
            types.removeAt(k);
        }
    }

    kDebug(5001) << "Creating Checksumsearch for" << urls.count() << "urls.";

    if (urls.count() && types.count()) {
        ChecksumSearch *search = new ChecksumSearch(urls, m_sourceUrl.fileName(), types);

        connect(search, SIGNAL(data(QString,QString)), this, SIGNAL(data(QString,QString)));
    }
}

void ChecksumSearchTransferDataSource::stop()
{
    kDebug(5001);
}

void ChecksumSearchTransferDataSource::addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    Q_UNUSED(segmentSize)
    Q_UNUSED(segmentRange)
    kDebug(5001);
}

#include "checksumsearchtransferdatasource.moc"
