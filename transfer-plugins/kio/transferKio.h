/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFER_KIO_H
#define TRANSFER_KIO_H

#include <KIO/FileCopyJob>

#include "core/transfer.h"

/**
 * This transfer uses the KIO class to download files
 */

class Verifier;

class TransferKio : public Transfer
{
    Q_OBJECT

public:
    TransferKio(TransferGroup *parent, TransferFactory *factory, Scheduler *scheduler, const QUrl &src, const QUrl &dest, const QDomElement *e = nullptr);

    /**
     * Move the download to the new destination
     * @param newDirectory is a directory where the download should be stored
     * @returns true if newDestination can be used
     */
    bool setDirectory(const QUrl &newDirectory) override;

    bool repair(const QUrl &file = QUrl()) override;

    Verifier *verifier(const QUrl &file = QUrl()) override;
    Signature *signature(const QUrl &file = QUrl()) override;

public Q_SLOTS:
    bool setNewDestination(const QUrl &newDestination);

    // --- Job virtual functions ---
    void start() override;
    void stop() override;

    void deinit(Transfer::DeleteOptions options) override;

private:
    void createJob();

    KIO::FileCopyJob *m_copyjob;
    bool m_stopped;
    bool m_movingFile;

private Q_SLOTS:
    void slotResult(KJob *kioJob);
    void slotInfoMessage(KJob *kioJob, const QString &msg);
    void slotPercent(KJob *kioJob, unsigned long percent);
    void slotTotalSize(KJob *kioJob, qulonglong size);
    void slotProcessedSize(KJob *kioJob, qulonglong size);
    void slotSpeed(KJob *kioJob, unsigned long bytes_per_second);
    void newDestResult(KJob *result);
    void slotVerified(bool isVerified);
    void slotStatResult(KJob *kioJob);

private:
    Verifier *m_verifier;
    Signature *m_signature;
};

#endif
