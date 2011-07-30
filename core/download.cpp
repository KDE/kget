/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "download.h"

#include <QFile>
#include <QFileInfo>

#include <KDebug>

Download::Download(const KUrl &srcUrl, const KUrl &destUrl)
  : m_srcUrl(srcUrl),
    m_destUrl(destUrl)
{
    kDebug(5001) << "DownloadFile: " << m_srcUrl.url() << " to dest: " << m_destUrl.url();
    m_copyJob = KIO::get(m_srcUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(m_copyJob, SIGNAL(data(KIO::Job*,QByteArray)), SLOT(slotData(KIO::Job*,QByteArray)));
    connect(m_copyJob, SIGNAL(result(KJob*)), SLOT(slotResult(KJob*)));
}

Download::~Download()
{
}

void Download::slotData(KIO::Job *job, const QByteArray& data)
{
    Q_UNUSED(job)
    kDebug(5001);
    /**if (data.size() == 0)
    {
        slotResult(job);
        return;
    }**/
    m_data.append(data);
}

void Download::slotResult(KJob * job)
{
    kDebug(5001);
    switch (job->error())
    {
        case 0://The download has finished
        {
            kDebug(5001) << "Downloading successfully finished" << m_destUrl.url();
            QFile torrentFile(m_destUrl.toLocalFile());
            if (!torrentFile.open(QIODevice::WriteOnly | QIODevice::Text)) {}
                //TODO: Do a Message box here
            torrentFile.write(m_data);
            torrentFile.close();
            emit finishedSuccessfully(m_destUrl, m_data);
            m_data = 0;
            break;
        }
        case KIO::ERR_FILE_ALREADY_EXIST:
        {
            kDebug(5001) << "ERROR - File already exists";
            QFile file(m_destUrl.toLocalFile());
            emit finishedSuccessfully(m_destUrl, file.readAll());
            m_data = 0;
            break;
        }
        default:
            kDebug(5001) << "We are sorry to say you, that there were errors while downloading :(";
            m_data = 0;
            emit finishedWithError();
            break;
    }
}
  
