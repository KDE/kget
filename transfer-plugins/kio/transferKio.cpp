/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferKio.h"
#ifdef HAVE_NEPOMUK
#include "core/nepomukhandler.h"
#endif //HAVE_NEPOMUK
#include "core/verifier.h"

#include <kiconloader.h>
#include <kio/scheduler.h>
#include <KIO/DeleteJob>
#include <KIO/CopyJob>
#include <KIO/NetAccess>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>

#include <QtCore/QFile>
#include <QDomElement>

TransferKio::TransferKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(0),
      m_movingFile(false),
      m_verifier(0)
{

}

bool TransferKio::setDirectory(const KUrl& newDirectory)
{
    KUrl newDest = newDirectory;
    newDest.addPath(m_dest.fileName());
    return setNewDestination(newDest);
}

bool TransferKio::setNewDestination(const KUrl &newDestination)
{
    if (isResumable() && newDestination.isValid() && (newDestination != dest()))
    {
        KUrl oldPath = KUrl(m_dest.path() + ".part");
        if (oldPath.isValid() && QFile::exists(oldPath.pathOrUrl()))
        {
            m_movingFile = true;
            stop();
            setStatus(Job::Moving);
            setTransferChange(Tc_Status, true);

            m_dest = newDestination;
#ifdef HAVE_NEPOMUK
            nepomukHandler()->setNewDestination(m_dest);
#endif //HAVE_NEPOMUK

            if (m_verifier)
            {
                m_verifier->setDestination(newDestination);
            }

            KIO::Job *move = KIO::file_move(oldPath, KUrl(newDestination.path() + ".part"), -1, KIO::HideProgressInfo);
            connect(move, SIGNAL(result(KJob *)), this, SLOT(newDestResult(KJob *)));
            connect(move, SIGNAL(infoMessage(KJob *, const QString &)), this, SLOT(slotInfoMessage(KJob *, const QString &)));
            connect(move, SIGNAL(percent(KJob *, unsigned long)), this, SLOT(slotPercent(KJob *, unsigned long)));

            return true;
        }
    }
    return false;
}

void TransferKio::newDestResult(KJob *result)
{
    Q_UNUSED(result);//TODO handle errors etc.!
    m_movingFile = false;
    start();
    setTransferChange(Tc_FileName);
}

void TransferKio::start()
{
    if (!m_movingFile)
    {
        m_stopped = false;
        if(!m_copyjob)
            createJob();

        kDebug(5001) << "TransferKio::start";
        setStatus(Job::Running, i18nc("transfer state: connecting", "Connecting...."), SmallIcon("network-connect")); // should be "network-connecting", but that doesn't exist for KDE 4.0 yet
        setTransferChange(Tc_Status, true);
    }
}

void TransferKio::stop()
{
    if(status() == Stopped)
        return;

    m_stopped = true;

    if(m_copyjob)
    {
        m_copyjob->kill(KJob::EmitResult);
        m_copyjob=0;
    }

    kDebug(5001) << "Stop";
    setStatus(Job::Stopped);
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

bool TransferKio::isResumable() const
{
    return true;
}

void TransferKio::postDeleteEvent()
{
    if (status() != Job::Finished)//if the transfer is not finished, we delete the *.part-file
    {
        KIO::Job *del = KIO::del(m_dest.path() + ".part", KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
#ifdef HAVE_NEPOMUK
    nepomukHandler()->postDeleteEvent();
#endif //HAVE_NEPOMUK
}

//NOTE: INTERNAL METHODS

void TransferKio::createJob()
{
    if(!m_copyjob)
    {
        KIO::Scheduler::checkSlaveOnHold(true);
        m_copyjob = KIO::file_copy(m_source, m_dest, -1, KIO::HideProgressInfo);

        connect(m_copyjob, SIGNAL(result(KJob *)), 
                this, SLOT(slotResult(KJob *)));
        connect(m_copyjob, SIGNAL(infoMessage(KJob *, const QString &)), 
                this, SLOT(slotInfoMessage(KJob *, const QString &)));
        connect(m_copyjob, SIGNAL(percent(KJob *, unsigned long)), 
                this, SLOT(slotPercent(KJob *, unsigned long)));
        connect(m_copyjob, SIGNAL(totalSize(KJob *, qulonglong)), 
                this, SLOT(slotTotalSize(KJob *, qulonglong)));
        connect(m_copyjob, SIGNAL(processedSize(KJob *, qulonglong)), 
                this, SLOT(slotProcessedSize(KJob *, qulonglong)));
        connect(m_copyjob, SIGNAL(speed(KJob *, unsigned long)), 
                this, SLOT(slotSpeed(KJob *, unsigned long)));
    }
}

void TransferKio::slotResult( KJob * kioJob )
{
    kDebug(5001) << "slotResult  (" << kioJob->error() << ")";
    switch (kioJob->error())
    {
        case 0:                            //The download has finished
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
            setStatus(Job::Finished);
        // "ok" icon should probably be "dialog-success", but we don't have that icon in KDE 4.0
            m_percent = 100;
            m_downloadSpeed = 0;
            m_downloadedSize = m_totalSize;
            setTransferChange(Tc_Percent | Tc_DownloadSpeed);
            break;
        default:
            //There has been an error
            kDebug(5001) << "--  E R R O R  (" << kioJob->error() << ")--";
            if (!m_stopped)
                setStatus(Job::Aborted);
            break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob=0;

    if ((status() == Job::Finished) && m_verifier)
    {
        m_verifier->verify();
    }
    setTransferChange(Tc_Status, true);
}

void TransferKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
  Q_UNUSED(kioJob);
    m_log.append(QString(msg));
}

void TransferKio::slotPercent( KJob * kioJob, unsigned long percent )
{
    kDebug(5001) << "slotPercent";
    Q_UNUSED(kioJob);
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void TransferKio::slotTotalSize( KJob * kioJob, qulonglong size )
{
    Q_UNUSED(kioJob);

    kDebug(5001) << "slotTotalSize";

    setStatus(Job::Running);

    m_totalSize = size;
    setTransferChange(Tc_Status | Tc_TotalSize, true);
}

void TransferKio::slotProcessedSize( KJob * kioJob, qulonglong size )
{
    Q_UNUSED(kioJob);

//     kDebug(5001) << "slotProcessedSize";

    if(status() != Job::Running)
    {
        setStatus(Job::Running);
        setTransferChange(Tc_Status);
    }
    m_downloadedSize = size;
    setTransferChange(Tc_DownloadedSize, true);
}

void TransferKio::slotSpeed( KJob * kioJob, unsigned long bytes_per_second )
{
    Q_UNUSED(kioJob);

//     kDebug(5001) << "slotSpeed";

    if(status() != Job::Running)
    {
        if (m_movingFile)
            setStatus(Job::Moving);
        else
            setStatus(Job::Running);
        setTransferChange(Tc_Status);

    }

    m_downloadSpeed = bytes_per_second;
    setTransferChange(Tc_DownloadSpeed, true);
}

void TransferKio::slotVerified(bool isVerified)
{
    if (!isVerified && KMessageBox::warningYesNo(0,
                    i18n("The download (%1) could not be verfied. Do you want to repair it?", m_dest.fileName()),
                    i18n("Verification failed.")) == KMessageBox::Yes)
    {
        repair();
    }
}

bool TransferKio::repair(const KUrl &file)
{
    Q_UNUSED(file)

    if (verifier()->status() == Verifier::NotVerified)
    {
        m_downloadedSize = 0;
        m_percent = 0;
        if(m_copyjob)
        {
            m_copyjob->kill(KJob::Quietly);
            m_copyjob = 0;
        }
        setTransferChange(Tc_DownloadedSize | Tc_Percent, true);

        start();

        return true;
    }

    return false;
}

Verifier *TransferKio::verifier(const KUrl &file)
{
    Q_UNUSED(file)

    if (!m_verifier)
    {
        m_verifier = new Verifier(m_dest);
        connect(m_verifier, SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
    }

    return m_verifier;
}

#include "transferKio.moc"
