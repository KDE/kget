/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef TRANSFER_KIO_H
#define TRANSFER_KIO_H

#include <kio/job.h>

#include "core/transfer.h"

/**
 * This transfer uses the KIO class to download files
 */
 
class TransferKio : public QObject, public Transfer
{
    Q_OBJECT

    public:
        TransferKio(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                    const QDomElement * e = 0);

    public slots:
        // --- Job virtual functions ---
        void start();
        void stop();

        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

        void save(QDomElement e);

    protected:
        void load(QDomElement e);

    private:
        void createJob();

        KIO::FileCopyJob * m_copyjob;

    private slots:
        void slotResult( KIO::Job * kioJob );
        void slotInfoMessage( KIO::Job * kioJob, const QString & msg );
        void slotConnected( KIO::Job * kioJob );
        void slotPercent( KIO::Job * kioJob, unsigned long percent );
        void slotTotalSize( KIO::Job * kioJob, KIO::filesize_t size );
        void slotProcessedSize( KIO::Job * kioJob, KIO::filesize_t size );
        void slotSpeed( KIO::Job * kioJob, unsigned long bytes_per_second );
};

#endif
