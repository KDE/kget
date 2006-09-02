/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef TRANSFER_MULTISEGKIO_H
#define TRANSFER_MULTISEGKIO_H

#include <kio/job.h>

#include "core/transfer.h"
#include "MultiSegKio.h"
/**
 * This transfer uses the KIO class to download files
 */
 
class transferMultiSegKio : public QObject, public Transfer
{
    Q_OBJECT

    public:
        transferMultiSegKio(TransferGroup * parent, TransferFactory * factory,
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

        KIO::MultiSegmentCopyJob *m_copyjob;
        QList<struct KIO::MultiSegData> tdata;

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
