/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _TRANSFER_MultiThreaded_H
#define _TRANSFER_MultiThreaded_H

#include <kio/job.h>

#include "core/transfer.h"
#include "mthreaded.h"

class QDomNode;

class TransferMultiThreaded : public QObject, public Transfer
{
    Q_OBJECT

    public:
        TransferMultiThreaded(TransferGroup * parent, TransferFactory * factory,
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
        void startJob();

        Mtget * m_Mtjob;
        QList<struct connd> tdata;

    private slots:
        void slotUpdate();
        void slotResult();
//         void slotInfoMessage( KIO::Job * kioJob, const QString & msg );
        void slotTotalSize( KIO::filesize_t Size );
        void slotProcessedSize( KIO::filesize_t Size );
        void slotSpeed( unsigned long bytes_per_second );
};

#endif  // MultiThreaded
