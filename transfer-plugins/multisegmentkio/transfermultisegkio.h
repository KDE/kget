/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef TRANSFER_MULTISEGKIO_H
#define TRANSFER_MULTISEGKIO_H

#include <kio/job.h>

#include "core/transfer.h"
#include "multisegkio.h"
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
        bool isResumable() const;
        void postDeleteEvent();

        void save(const QDomElement &element);

    protected:
        void load(const QDomElement &e);

    private:
        void createJob();

        MultiSegmentCopyJob *m_copyjob;
        QList<SegData> SegmentsData;
        QList<KUrl> m_Urls;
        bool m_isDownloading;
        bool stopped;

    private slots:
        void slotUpdateSegmentsData();
        void slotResult( KJob *kioJob );
        void slotInfoMessage( KJob * kioJob, const QString & msg );
        void slotPercent( KJob *kioJob, unsigned long percent );
        void slotTotalSize( KJob *kioJob, qulonglong size );
        void slotProcessedSize( KJob *kioJob, qulonglong size );
        void slotSpeed( KJob * kioJob, unsigned long bytes_per_second );
        void slotSearchUrls(const QList<KUrl> &Urls);
};

#endif
