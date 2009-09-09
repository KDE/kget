/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef KIODOWNLOAD_H
#define KIODOWNLOAD_H


#include <KIO/Job>

class QFile;

/**
 * This class is there to normally download a file without using a transfer
 * You should use this class to e.g. get the size of a file while not wasting
 * the download needed to reach that point
 */
 
class KioDownload : public QObject
{
    Q_OBJECT

    public:
        KioDownload(const KUrl &src, const KUrl &dest, QObject *parent);

        ~KioDownload();

//         virtual bool setNewDestination(const KUrl &newDestination);
        KIO::filesize_t processedSize() const {return m_processedSize;}

    public slots:
        // --- Job virtual functions ---
        void start();
        void stop();

        bool isResumable() const;

//         void postDeleteEvent();

    Q_SIGNALS:
        void processedSize(KIO::filesize_t size);
        void totalSize(KIO::filesize_t size);
        void speed(ulong speed);
        void finished();
        //void suggestedFileName(const QString &name);

    private slots:
        void slotResult(KJob *kioJob);
//         void slotInfoMessage( KJob * kioJob, const QString & msg );
//         void slotPercent( KJob * kioJob, unsigned long percent );
        void slotTotalSize( KJob * kioJob, qulonglong size );
        void slotSpeed( KJob * kioJob, unsigned long bytes_per_second );
//         void newDestResult(KJob *result);
        void slotData(KIO::Job *job, const QByteArray &data);

    private:
        void createJob();
        void killJob();

    private:
        KUrl m_source;
        KUrl m_dest;
        QFile *m_file;
        KIO::TransferJob *m_getJob;
        bool m_stopped;
        bool m_movingFile;
        KIO::filesize_t m_processedSize;
        KIO::fileoffset_t m_offset;
};

#endif
