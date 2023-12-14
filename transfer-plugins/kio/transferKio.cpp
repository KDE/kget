/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferKio.h"
#include "core/signature.h"
#include "core/verifier.h"
#include "settings.h"

#ifdef Q_OS_WIN
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#include "kget_debug.h"
#include <QDebug>

#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KLocalizedString>
#include <KMessageBox>

#include <QDomElement>
#include <QFile>

TransferKio::TransferKio(TransferGroup *parent, TransferFactory *factory, Scheduler *scheduler, const QUrl &source, const QUrl &dest, const QDomElement *e)
    : Transfer(parent, factory, scheduler, source, dest, e)
    , m_copyjob(nullptr)
    , m_movingFile(false)
    , m_verifier(nullptr)
    , m_signature(nullptr)
{
    setCapabilities(Transfer::Cap_Moving | Transfer::Cap_Renaming | Transfer::Cap_Resuming); // TODO check if it really can resume
}

bool TransferKio::setDirectory(const QUrl &newDirectory)
{
    QUrl newDest = newDirectory;
    newDest.setPath(newDest.adjusted(QUrl::RemoveFilename).toString() + m_dest.fileName());
    return setNewDestination(newDest);
}

bool TransferKio::setNewDestination(const QUrl &newDestination)
{
    if (newDestination.isValid() && (newDestination != dest())) {
        QString oldPath = m_dest.toLocalFile() + ".part";
        if (QFile::exists(oldPath)) {
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

            KIO::Job *move =
                KIO::file_move(QUrl::fromLocalFile(oldPath), QUrl::fromLocalFile(newDestination.toLocalFile() + ".part"), -1, KIO::HideProgressInfo);
            connect(move, &KJob::result, this, &TransferKio::newDestResult);
            connect(move, &KJob::infoMessage, this, &TransferKio::slotInfoMessage);
            connect(move, SIGNAL(percent(KJob *, ulong)), this, SLOT(slotPercent(KJob *, ulong)));

            return true;
        }
    }
    return false;
}

void TransferKio::newDestResult(KJob *result)
{
    Q_UNUSED(result) // TODO handle errors etc.!
    m_movingFile = false;
    start();
    setTransferChange(Tc_FileName);
}

void TransferKio::start()
{
    if (!m_movingFile && (status() != Finished)) {
        m_stopped = false;
        if (!m_copyjob)
            createJob();

        qCDebug(KGET_DEBUG) << "TransferKio::start";
        setStatus(Job::Running,
                  i18nc("transfer state: connecting", "Connecting...."),
                  "network-connect"); // should be "network-connecting", but that doesn't exist for KDE 4.0 yet
        setTransferChange(Tc_Status, true);
    }
}

void TransferKio::stop()
{
    if ((status() == Stopped) || (status() == Finished)) {
        return;
    }

    m_stopped = true;

    if (m_copyjob) {
        m_copyjob->kill(KJob::EmitResult);
        m_copyjob = nullptr;
    }

    qCDebug(KGET_DEBUG) << "Stop";
    setStatus(Job::Stopped);
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

void TransferKio::deinit(Transfer::DeleteOptions options)
{
    if (options & DeleteFiles) // if the transfer is not finished, we delete the *.part-file
    {
        KIO::Job *del = KIO::del(QUrl::fromLocalFile(m_dest.path() + ".part"), KIO::HideProgressInfo);
        if (!del->exec()) {
            qCDebug(KGET_DEBUG) << "Could not delete part " << QString(m_dest.path() + ".part");
        }
    } // TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
}

// NOTE: INTERNAL METHODS

void TransferKio::createJob()
{
    if (!m_copyjob) {
        m_copyjob = KIO::file_copy(m_source, m_dest, -1, KIO::HideProgressInfo);

        connect(m_copyjob, &KJob::result, this, &TransferKio::slotResult);
        connect(m_copyjob, &KJob::infoMessage, this, &TransferKio::slotInfoMessage);
        connect(m_copyjob, SIGNAL(percent(KJob *, ulong)), this, SLOT(slotPercent(KJob *, ulong)));
        connect(m_copyjob, &KJob::totalSize, this, &TransferKio::slotTotalSize);
        connect(m_copyjob, &KJob::processedSize, this, &TransferKio::slotProcessedSize);
        connect(m_copyjob, &KJob::speed, this, &TransferKio::slotSpeed);
    }
}

void TransferKio::slotResult(KJob *kioJob)
{
    qCDebug(KGET_DEBUG) << "slotResult  (" << kioJob->error() << ")";
    switch (kioJob->error()) {
    case 0: // The download has finished
    case KIO::ERR_FILE_ALREADY_EXIST: // The file has already been downloaded.
        setStatus(Job::Finished);
        // "ok" icon should probably be "dialog-success", but we don't have that icon in KDE 4.0
        m_percent = 100;
        m_downloadSpeed = 0;
        m_downloadedSize = m_totalSize;
        setTransferChange(Tc_Percent | Tc_DownloadSpeed);
        break;
    default:
        // There has been an error
        qCDebug(KGET_DEBUG) << "--  E R R O R  (" << kioJob->error() << ")--";
        if (!m_stopped)
            setStatus(Job::Aborted);
        break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob = nullptr;

    // If it is an ftp file, there's still work to do
    Transfer::ChangesFlags flags = (m_source.scheme() != "ftp") ? Tc_Status : Tc_None;
    if (status() == Job::Finished) {
        if (!m_totalSize) {
            // downloaded elsewhere already, e.g. Konqueror
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

    if (m_source.scheme() == "ftp") {
        KIO::StatJob *statJob = KIO::stat(m_source);
        connect(statJob, &KJob::result, this, &TransferKio::slotStatResult);
        statJob->start();
    }

    setTransferChange(flags, true);
}

void TransferKio::slotInfoMessage(KJob *kioJob, const QString &msg)
{
    Q_UNUSED(kioJob)
    m_log.append(QString(msg));
}

void TransferKio::slotPercent(KJob *kioJob, unsigned long percent)
{
    qCDebug(KGET_DEBUG) << "slotPercent";
    Q_UNUSED(kioJob)
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void TransferKio::slotTotalSize(KJob *kioJob, qulonglong size)
{
    Q_UNUSED(kioJob)

    qCDebug(KGET_DEBUG) << "slotTotalSize";

    setStatus(Job::Running);

    m_totalSize = size;
    setTransferChange(Tc_Status | Tc_TotalSize, true);
}

void TransferKio::slotProcessedSize(KJob *kioJob, qulonglong size)
{
    Q_UNUSED(kioJob)

    //     qCDebug(KGET_DEBUG) << "slotProcessedSize";

    if (status() != Job::Running) {
        setStatus(Job::Running);
        setTransferChange(Tc_Status);
    }
    m_downloadedSize = size;
    setTransferChange(Tc_DownloadedSize, true);
}

void TransferKio::slotSpeed(KJob *kioJob, unsigned long bytes_per_second)
{
    Q_UNUSED(kioJob)

    //     qCDebug(KGET_DEBUG) << "slotSpeed";

    if (status() != Job::Running) {
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
        QString text;
        KGuiItem action;
        if (verifier()->partialChunkLength()) {
            text = i18n("The download (%1) could not be verified. Do you want to repair it?", m_dest.fileName());
            action = KGuiItem(i18nc("@action:button", "Repair"));
        } else {
            text = i18n("The download (%1) could not be verified. Do you want to redownload it?", m_dest.fileName());
            action = KGuiItem(i18nc("@action:button", "Download Again"), QStringLiteral("document-save"));
        }
        if (KMessageBox::warningTwoActions(nullptr, text, i18n("Verification failed."), action, KGuiItem(i18n("Ignore"), QStringLiteral("dialog-cancel")))
            == KMessageBox::PrimaryAction) {
            repair();
        }
    }
}

void TransferKio::slotStatResult(KJob *kioJob)
{
    auto *statJob = qobject_cast<KIO::StatJob *>(kioJob);

    if (!statJob->error()) {
        const KIO::UDSEntry entryResult = statJob->statResult();
        struct utimbuf time;

        time.modtime = entryResult.numberValue(KIO::UDSEntry::UDS_MODIFICATION_TIME);
        time.actime = QDateTime::currentDateTime().toSecsSinceEpoch();
        utime(m_dest.toLocalFile().toUtf8().constData(), &time);
    }

    setStatus(Job::Finished);
    setTransferChange(Tc_Status, true);
}

bool TransferKio::repair(const QUrl &file)
{
    Q_UNUSED(file)

    if (verifier()->status() == Verifier::NotVerified) {
        m_downloadedSize = 0;
        m_percent = 0;
        if (m_copyjob) {
            m_copyjob->kill(KJob::Quietly);
            m_copyjob = nullptr;
        }
        setTransferChange(Tc_DownloadedSize | Tc_Percent, true);

        start();

        return true;
    }

    return false;
}

Verifier *TransferKio::verifier(const QUrl &file)
{
    Q_UNUSED(file)

    if (!m_verifier) {
        m_verifier = new Verifier(m_dest, this);
        connect(m_verifier, &Verifier::verified, this, &TransferKio::slotVerified);
    }

    return m_verifier;
}

Signature *TransferKio::signature(const QUrl &file)
{
    Q_UNUSED(file)

    if (!m_signature) {
        m_signature = new Signature(m_dest, this);
    }

    return m_signature;
}

#include "moc_transferKio.cpp"
