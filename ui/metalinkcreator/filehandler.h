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

#ifndef FILEHANDLER
#define FILEHANDLER

#include <QMutex>
#include <QThread>

#include <KIO/UDSEntry>

#include "metalinker.h"

struct FileData {
    QUrl url;
    KGetMetalink::File file;
};

/**
 * Calculates the checksum and partial checksums for files and adds KGetMetalink::Resources and
 * KGetMetalink::CommonData to the resulting KGetMetalink::File
 */
class FileHandlerThread : public QThread
{
    Q_OBJECT

public:
    FileHandlerThread(QObject *parent = nullptr);
    ~FileHandlerThread() override;

    void setData(const QList<FileData> &files,
                 const QStringList &types,
                 bool createPartial,
                 const KGetMetalink::Resources &tempResources,
                 const KGetMetalink::CommonData &tempCommonData);

Q_SIGNALS:
    void fileResult(const KGetMetalink::File file);

protected:
    void run() override;

private:
    QMutex mutex;
    bool abort;
    QList<QList<FileData>> m_files;
    QList<QStringList> m_types;
    QList<bool> m_createPartial;
    QList<KGetMetalink::Resources> m_tempResources;
    QList<KGetMetalink::CommonData> m_tempCommonDatas;
};

/**
 * Gets the name and the size of the files of a list of urls, also enters
 * directories recursively if needed
 */
class DirectoryHandler : public QObject
{
    Q_OBJECT

public:
    DirectoryHandler(QObject *parent);
    ~DirectoryHandler() override;

    /**
     * Returns the handled files and clears the member, call this after finished is emitted
     */
    QList<FileData> takeFiles();

public Q_SLOTS:
    /**
     * The files the FileHandler should handle, the urls can also be urls to directories
     * then the files of these directories will be got recursively
     */
    void slotFiles(const QList<QUrl> &files);

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void slotDirEntries(KIO::Job *job, const KIO::UDSEntryList &entries);

    /**
     * Finished one of the jobs to get all files of a directory recursively
     */
    void slotFinished(KJob *job);

private:
    /**
     * Checks if all urls have been parsed already and emits finished in that case
     */
    void evaluateFileProcess();

private:
    bool m_allJobsStarted;
    QHash<KJob *, QUrl> m_jobs;
    QList<FileData> m_files;
};

#endif
