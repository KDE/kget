/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _TRANSFER_KIO_H
#define _TRANSFER_KIO_H

#include <kio/job.h>

#include "core/transfer.h"

class QDomNode;

/**
 * This transfer uses the KIO class to download files
 */
 
class TransferKio : public QObject, public Transfer
{
    Q_OBJECT

    public:
        TransferKio(TransferGroup * parent, Scheduler * scheduler,
                    const KURL & source, const KURL & dest);
        TransferKio(TransferGroup * parent, Scheduler * scheduler,
                    QDomNode * n);

    public slots:
        // --- Job virtual functions ---
        void start();
        void stop();

        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

        // --- Transfer virtual functions ---
        unsigned long totalSize() const;
        unsigned long processedSize() const;

        int percent() const;
        int speed() const;

    protected:
        void read(QDomNode * n);
        void write(QDomNode * n);

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
