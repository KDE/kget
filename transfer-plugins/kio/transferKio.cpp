/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferKio.h"
#include "core/verifier.h"
#include "core/signature.h"
#include "kget/settings.h"

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
      m_verifier(0),
      m_signature(0)
{
    setCapabilities(Transfer::Cap_Moving | Transfer::Cap_Renaming | Transfer::Cap_Resuming);//TODO check if it really can resume
}

bool TransferKio::setDirectory(const KUrl& newDirectory)
{
    KUrl newDest = newDirectory;
    newDest.addPath(m_dest.fileName());
    return setNewDestination(newDest);
}

bool TransferKio::setNewDestination(const KUrl &newDestination)
{
    if (newDestination.isValid() && (newDestination != dest())) {
        KUrl oldPath = KUrl(m_dest.path() + ".part");
        if (oldPath.isValid() && QFile::exists(oldPath.pathOrUrl())) {
            m_movingFile = true;
            stop();
            setStatus(Job::Moving);
            setTransferChange(Tc_Status, true);

            m_dest = newDestination;

            if (m_verifier) {
                m_verifier->setDestination(newDestination);
            }
            if (m_signature) {
                m_signature->setDestination(newDestination);
            }

            KIO::Job *move = KIO::file_move(oldPath, KUrl(newDestination.path() + ".part"), -1, KIO::HideProgressInfo);
            connect(move, SIGNAL(result(KJob*)), this, SLOT(newDestResult(KJob*)));
            connect(move, SIGNAL(infoMessage(KJob*,QString)), this, SLOT(slotInfoMessage(KJob*,QString)));
            connect(move, SIGNAL(percent(KJob*,ulong)), this, SLOT(slotPercent(KJob*,ulong)));

            return true;
        }
    }
    return false;
}

void TransferKio::newDestResult(KJob *result)
{
    Q_UNUSED(result)//TODO handle errors etc.!
    m_movingFile = false;
    start();
    setTransferChange(Tc_FileName);
}

void TransferKio::start()
{
    if (!m_movingFile && (status() != Finished)) {
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
    if ((status() == Stopped) || (status() == Finished)) {
        return;
    }

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

void TransferKio::deinit(Transfer::DeleteOptions options)
{
    if (options & DeleteFiles)//if the transfer is not finished, we delete the *.part-file
    {
        KIO::Job *del = KIO::del(QString(m_dest.path() + ".part"), KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, 0);
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
}

//NOTE: INTERNAL METHODS

void TransferKio::createJob()
{
    if(!m_copyjob)
    {
        KIO::Scheduler::checkSlaveOnHold(true);
        m_copyjob = KIO::file_copy(m_source, m_dest, -1, KIO::HideProgressInfo);

        connect(m_copyjob, SIGNAL(result(KJob*)), 
                this, SLOT(slotResult(KJob*)));
        connect(m_copyjob, SIGNAL(infoMessage(KJob*,QString)), 
                this, SLOT(slotInfoMessage(KJob*,QString)));
        connect(m_copyjob, SIGNAL(percent(KJob*,ulong)), 
                this, SLOT(slotPercent(KJob*,ulong)));
        connect(m_copyjob, SIGNAL(totalSize(KJob*,qulonglong)), 
                this, SLOT(slotTotalSize(KJob*,qulonglong)));
        connect(m_copyjob, SIGNAL(processedSize(KJob*,qulonglong)), 
                this, SLOT(slotProcessedSize(KJob*,qulonglong)));
        connect(m_copyjob, SIGNAL(speed(KJob*,ulong)), 
                this, SLOT(slotSpeed(KJob*,ulong)));
    }
}

void TransferKio::slotResult( KJob * kioJob )
{
    kDebug(5001) << "slotResult  (" << kioJob->error() << ")";
    switch (kioJob->error()) {
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

    Transfer::ChangesFlags flags = Tc_Status;
    if (status() == Job::Finished) {
        if (!m_totalSize) {
            //downloaded elsewhere already, e.g. Konqueror
            if (!m_downloadedSize) {
                QFile file(m_dest.toLocalFile() + ".part");
                m_downloadedSize = file.size();
                if (!m_downloadedSize) {
                    QFile file(m_dest.toLocalFile());
                    m_downloadedSize = file.size();
                }
            }
            m_totalSize = m_downloadedSize;
            flags |= Tc_DownloadedSize;
        }
        if (m_verifier && Settings::checksumAutomaticVerification()) {
            m_verifier->verify();
        }
        if (m_signature && Settings::signatureAutomaticVerification()) {
            m_signature->verify();
        }
    }

    setTransferChange(flags, true);
}

void TransferKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
  Q_UNUSED(kioJob)
    m_log.append(QString(msg));
}

void TransferKio::slotPercent( KJob * kioJob, unsigned long percent )
{
    kDebug(5001) << "slotPercent";
    Q_UNUSED(kioJob)
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void TransferKio::slotTotalSize( KJob * kioJob, qulonglong size )
{
    Q_UNUSED(kioJob)

    kDebug(5001) << "slotTotalSize";

    setStatus(Job::Running);

    m_totalSize = size;
    setTransferChange(Tc_Status | Tc_TotalSize, true);
}

void TransferKio::slotProcessedSize( KJob * kioJob, qulonglong size )
{
    Q_UNUSED(kioJob)

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
    Q_UNUSED(kioJob)

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
    if (!isVerified) {
        QString text = i18n("The download (%1) could not be verified. Do you want to repair it?", m_dest.fileName());

        if (!verifier()->partialChunkLength()) {
            text = i18n("The download (%1) could not be verified. Do you want to redownload it?", m_dest.fileName());
        }
        if (KMessageBox::warningYesNo(0,
                                      text,
                                      i18n("Verification failed.")) == KMessageBox::Yes) {
            repair();
        }
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
        m_verifier = new Verifier(m_dest, this);
        connect(m_verifier, SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
    }

    return m_verifier;
}

Signature *TransferKio::signature(const KUrl &file)
{
    Q_UNUSED(file)

    if (!m_signature) {
        m_signature = new Signature(m_dest, this);
    }
    
    return m_signature;
}

#include "transferKio.moc"
