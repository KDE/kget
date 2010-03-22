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

#include "filehandler.h"
#include "core/verifier.h"

#include <QtCore/QDir>

FileHandlerThread::FileHandlerThread(QObject *parent)
  : QThread(parent),
    abort(false)
{
}

FileHandlerThread::~FileHandlerThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}

void FileHandlerThread::setData(const QList<FileData> &files, const QStringList &types, bool createPartial, const KGetMetalink::Resources &tempResources, const KGetMetalink::CommonData &tempCommonData)
{
    QMutexLocker locker(&mutex);
    m_files.append(files);
    m_types.append(types);
    m_createPartial.append(createPartial);
    m_tempResources.append(tempResources);
    m_tempCommonDatas.append(tempCommonData);

    if (!isRunning()) {
        start();
    }
}

void FileHandlerThread::run()
{
    mutex.lock();
    bool run = m_files.count();
    mutex.unlock();

    while (run && !abort) {
        mutex.lock();
        QList<FileData> files = m_files.takeFirst();
        QStringList types = m_types.takeFirst();
        bool createPartial = m_createPartial.takeFirst();
        KGetMetalink::Resources tempResources = m_tempResources.takeFirst();
        KGetMetalink::CommonData commonData = m_tempCommonDatas.takeFirst();
        mutex.unlock();

        while (files.count() && !abort) {
            //take the first file and try to handle it
            FileData data = files.takeFirst();
            const KUrl url = data.url;
            KGetMetalink::File file = data.file;
            file.data = commonData;

            foreach (const KGetMetalink::Url &metalinkUrl, tempResources.urls) {
                KGetMetalink::Url mirror = metalinkUrl;
                mirror.url.addPath(file.name);

                //if the url has already been added, remove it and readd it
                for (int i = 0; i < file.resources.urls.count(); ++i) {
                    if (file.resources.urls[i].url == mirror.url) {
                        file.resources.urls.removeAt(i);
                        break;
                    }
                }
                file.resources.urls.append(mirror);
            }

            foreach (const QString &type, types) {
                if (abort) {
                    return;
                }

                const QString hash = Verifier::checksum(url, type, &abort);
                if (!hash.isEmpty()) {
                    file.verification.hashes[type] = hash;
                }
            }

            if (createPartial) {
                foreach (const QString &type, types) {
                    if (abort) {
                        return;
                    }

                    const PartialChecksums partialChecksums = Verifier::partialChecksums(url, type, 0, &abort);
                    if (partialChecksums.isValid()) {
                        KGetMetalink::Pieces pieces;
                        pieces.type = type;
                        pieces.length = partialChecksums.length();
                        pieces.hashes = partialChecksums.checksums();
                        file.verification.pieces.append(pieces);
                    }
                }
            }
            if (!abort) {
                emit fileResult(file);
            }
        }
        mutex.lock();
        run = m_files.count();
        mutex.unlock();
    }
}

DirectoryHandler::DirectoryHandler(QObject *parent)
  : QObject(parent),
    m_allJobsStarted(false)
{
}

DirectoryHandler::~DirectoryHandler()
{
}

QList<FileData> DirectoryHandler::takeFiles()
{
    QList<FileData> files = m_files;
    m_files.clear();

    return files;
}

void DirectoryHandler::slotFiles(const QList<KUrl> &files)
{
    if (files.isEmpty()) {
        return;
    }

    m_allJobsStarted = false;

    foreach (const KUrl &url, files) {
        QDir dir(url.path());
        if (dir.exists()) {
            KIO::ListJob *listJob = KIO::listRecursive(url);
            m_jobs[listJob] = url;

            connect(listJob, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)), this, SLOT(slotDirEntries(KIO::Job*,KIO::UDSEntryList)));
            connect(listJob, SIGNAL(result(KJob*)), this, SLOT(slotFinished(KJob*)));
        } else {
            FileData data;
            data.url = url;
            data.file.name = url.fileName();
            QFile localFile(url.path());
            data.file.size = localFile.size();

            m_files.append(data);
        }
    }

    m_allJobsStarted = true;
    evaluateFileProcess();
}

void DirectoryHandler::slotDirEntries(KIO::Job *j, const KIO::UDSEntryList &entries)
{
    KJob *job = j;
    if (!m_jobs.contains(job)) {
        return;
    }

    const KUrl baseUrl = m_jobs[job];
    const QString baseDir = baseUrl.fileName() + '/';

    foreach (const KIO::UDSEntry &entry, entries) {
        //skip all found dirs
        if (!entry.isDir()) {
            const QString name = entry.stringValue(KIO::UDSEntry::UDS_NAME);
            FileData data;
            data.url = baseUrl;
            data.url.addPath(name);
            data.file.name = baseDir + name;
            data.file.size = entry.numberValue(KIO::UDSEntry::UDS_SIZE, -1);

            m_files.append(data);
        }
    }
}

void DirectoryHandler::slotFinished(KJob *job)
{
    if (m_jobs.contains(job)) {
        m_jobs.remove(job);
        evaluateFileProcess();
    }
}

void DirectoryHandler::evaluateFileProcess()
{
    //all jobs finished
    if (m_jobs.isEmpty() && m_allJobsStarted && !m_files.isEmpty()) {
        emit finished();
    }
}

#include "filehandler.moc"
