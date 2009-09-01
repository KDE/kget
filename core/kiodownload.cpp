/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "kiodownload.h"
// #ifdef HAVE_NEPOMUK
// #include "core/nepomukhandler.h"
// #endif //HAVE_NEPOMUK

// #include <kiconloader.h>
// #include <klocale.h>
#include <kdebug.h>

#include <QtCore/QFile>
// #include <QDomElement>

KioDownload::KioDownload(const KUrl &src, const KUrl &dest, QObject *parent)
    : QObject(parent),
      m_source(src),
      m_dest(dest),
      m_file(0),
      m_getJob(0),
      m_movingFile(false),
      m_processedSize(0),
      m_offset(0)
{
    kDebug() << "****";
}

KioDownload::~KioDownload()
{
    killJob();
}

// bool KioDownload::setNewDestination(const KUrl &newDestination)
// {
//     if (isResumable() && newDestination.isValid() && (newDestination != dest()))
//     {
//         KUrl oldPath = KUrl(m_dest.path() + ".part");
//         if (oldPath.isValid() && QFile::exists(oldPath.pathOrUrl()))
//         {
//             m_movingFile = true;
//             stop();
//             setStatus(Job::Stopped, i18nc("changing the destination of the file", "Changing destination"), SmallIcon("media-playback-pause"));
//             setTransferChange(Tc_Status, true);
// 
//             m_dest = newDestination;
// #ifdef HAVE_NEPOMUK
//             nepomukHandler()->setNewDestination(m_dest);
// #endif //HAVE_NEPOMUK
// 
//             KIO::Job *move = KIO::file_move(oldPath, KUrl(newDestination.path() + ".part"), -1, KIO::HideProgressInfo);
//             connect(move, SIGNAL(result(KJob *)), this, SLOT(newDestResult(KJob *)));
//             connect(move, SIGNAL(infoMessage(KJob *, const QString &)), this, SLOT(slotInfoMessage(KJob *, const QString &)));
//             connect(move, SIGNAL(percent(KJob *, unsigned long)), this, SLOT(slotPercent(KJob *, unsigned long)));
// 
//             return true;
//         }
//     }
//     return false;
// }

// void KioDownload::newDestResult(KJob *result)
// {
//     Q_UNUSED(result);//TODO handle errors etc.!
//     m_movingFile = false;
//     start();
//     setTransferChange(Tc_FileName);
// }

void KioDownload::start()
{
    kDebug() << "****";
    if (!m_movingFile)
    {
        if (!m_file)
        {
            m_file = new QFile(m_dest.pathOrUrl(), this);
            if (!m_file->open(QIODevice::ReadWrite))
            {
                delete m_file;
                m_file = 0;
                kDebug() << "***not open";
            }
        }
        m_stopped = false;
        if(!m_getJob)
        {
            createJob();
        }
    }
}

void KioDownload::stop()
{
    m_stopped = true;
    killJob();
    if (m_file)
    {
        m_file->close();
        m_file = 0;
    }
}

bool KioDownload::isResumable() const
{
    return true;
}

// void KioDownload::postDeleteEvent()
// {
//     if (status() != Job::Finished)//if the transfer is not finished, we delete the *.part-file
//     {
//         KIO::Job *del = KIO::del(m_dest.path() + ".part", KIO::HideProgressInfo);
//         KIO::NetAccess::synchronousRun(del, NULL);
//     }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
// }

//NOTE: INTERNAL METHODS

void KioDownload::createJob()
{
    if (!m_getJob)//TODO delete when not stopped, how? test
    {
        kDebug() << "****createJob";
        m_getJob = KIO::get(m_source, KIO::Reload, KIO::HideProgressInfo);
        m_getJob->suspend();
        m_getJob->addMetaData("errorPage", "false");
        m_getJob->addMetaData("AllowCompressedPage", "false");

        connect(m_getJob, SIGNAL(data(KIO::Job *, const QByteArray&)), this, SLOT(slotData(KIO::Job *, const QByteArray&)));
        connect(m_getJob, SIGNAL(result(KJob *)), this, SLOT(slotResult( KJob *)));
//         connect(m_getJob, SIGNAL(infoMessage(KJob *, const QString &)), this, SLOT(slotInfoMessage(KJob *, const QString &)));
//         connect(m_getJob, SIGNAL(percent(KJob *, unsigned long)), this, SLOT(slotPercent(KJob *, unsigned long)));
        connect(m_getJob, SIGNAL(totalSize(KJob *, qulonglong)), this, SLOT(slotTotalSize(KJob*, qulonglong)));
        connect(m_getJob, SIGNAL(speed(KJob *, unsigned long)), this, SLOT(slotSpeed(KJob*,ulong)));
    }
}

void KioDownload::killJob()
{
    if(m_getJob)
    {
        m_getJob->kill(KJob::EmitResult);
        m_getJob = 0;
    }
}

void KioDownload::slotResult(KJob *kioJob)
{
    kDebug(5001) << "slotResult  (" << kioJob->error() << ")";
    switch (kioJob->error())
    {
        case 0:                            //The download has finished
            emit finished();
            break;
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
//             setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
//         // "ok" icon should probably be "dialog-success", but we don't have that icon in KDE 4.0
//             m_percent = 100;
//             m_downloadSpeed = 0;
//             m_downloadedSize = m_totalSize;
//             setTransferChange(Tc_Percent | Tc_DownloadSpeed);
//             break;
        default:
            //There has been an error
            kDebug(5001) << "--  E R R O R  (" << kioJob->error() << ")--";
//             if (!m_stopped)
//                 setStatus(Job::Aborted, i18n("Aborted"), SmallIcon("dialog-error"));
            break;
    }
//     // when slotResult gets called, the m_copyjob has already been deleted!
    m_getJob = 0;
//     setTransferChange(Tc_Status, true);
}

// void KioDownload::slotInfoMessage( KJob * kioJob, const QString & msg )
// {
//   Q_UNUSED(kioJob);
//     m_log.append(QString(msg));
// }
// 
// void KioDownload::slotPercent( KJob * kioJob, unsigned long percent )
// {
//     kDebug(5001) << "slotPercent";
//     Q_UNUSED(kioJob);
//     m_percent = percent;
//     setTransferChange(Tc_Percent, true);
// }
// 
void KioDownload::slotTotalSize(KJob *kioJob, qulonglong size)
{
    Q_UNUSED(kioJob);

    emit totalSize(size);
}

void KioDownload::slotSpeed(KJob *kioJob, unsigned long bytes_per_second)
{
    Q_UNUSED(kioJob);

    emit speed(bytes_per_second);
}

void KioDownload::slotData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)

    if (m_file)
    {
        m_file->seek(m_offset);
        m_file->write(data);
        m_offset += data.size();

        m_processedSize += data.size();
        emit processedSize(m_processedSize);
    }
}

#include "kiodownload.moc"
