/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef MMSTRANSFER_H
#define MMSTRANSFER_H

#include <kio/job.h>

#include "core/transfer.h"

class QTimer;

class MMSClientThread;

class MmsTransfer : public Transfer
{
    Q_OBJECT

    public:
        MmsTransfer(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                    const QDomElement * e = 0);

    public slots:
        // --- Job virtual functions ---
        void start();
        void stop();

        bool isResumable() const;

    private slots:
        void read();
        /**void save(const QDomElement &e);

    protected:
        void load(const QDomElement &e);**/

    private:
        MMSClientThread * m_mmsClientThread;
        
    /**private:
        void createJob();

        KIO::FileCopyJob * m_copyjob;
        bool m_stopped;

    private slots:
        void slotResult( KJob * kioJob );
        void slotInfoMessage( KJob * kioJob, const QString & msg );
        void slotPercent( KJob * kioJob, unsigned long percent );
        void slotTotalSize( KJob * kioJob, qulonglong size );
        void slotProcessedSize( KJob * kioJob, qulonglong size );
        void slotSpeed( KJob * kioJob, unsigned long bytes_per_second );**/

};

#endif
