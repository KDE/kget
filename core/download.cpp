/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "download.h"

#include <QDebug>
#include <QFile>

#include <KIO/Global>

#include "kget_debug.h"

Download::Download(const QUrl &srcUrl, const QUrl &destUrl)
    : m_srcUrl(srcUrl)
    , m_destUrl(destUrl)
{
    qCDebug(KGET_DEBUG) << "DownloadFile: " << m_srcUrl.url() << " to dest: " << m_destUrl.url();
    m_copyJob = KIO::get(m_srcUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(m_copyJob, &KIO::TransferJob::data, this, &Download::slotData);
    connect(m_copyJob, &KJob::result, this, &Download::slotResult);
}

Download::~Download()
{
}

void Download::slotData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)
    qCDebug(KGET_DEBUG);
    /**if (data.size() == 0)
    {
        slotResult(job);
        return;
    }**/
    m_data.append(data);
}

void Download::slotResult(KJob *job)
{
    qCDebug(KGET_DEBUG);
    switch (job->error()) {
    case 0: // The download has finished
    {
        qCDebug(KGET_DEBUG) << "Downloading successfully finished" << m_destUrl.url();
        QFile torrentFile(m_destUrl.toLocalFile());
        if (!torrentFile.open(QIODevice::WriteOnly | QIODevice::Text)) { }
        // TODO: Do a Message box here
        torrentFile.write(m_data);
        torrentFile.close();
        Q_EMIT finishedSuccessfully(m_destUrl, m_data);
        m_data = nullptr;
        break;
    }
    case KIO::ERR_FILE_ALREADY_EXIST: {
        qCDebug(KGET_DEBUG) << "ERROR - File already exists";
        QFile file(m_destUrl.toLocalFile());
        Q_EMIT finishedSuccessfully(m_destUrl, file.readAll());
        m_data = nullptr;
        break;
    }
    default:
        qCDebug(KGET_DEBUG) << "We are sorry to say you, that there were errors while downloading :(";
        m_data = nullptr;
        Q_EMIT finishedWithError();
        break;
    }
}

#include "moc_download.cpp"
