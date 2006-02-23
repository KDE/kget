/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtXml/QDomElement>
#include <QtNetwork/QHttpRequestHeader>
#include <QtNetwork/QHttpResponseHeader>
#include <QtNetwork/QHttp>
#include <QtNetwork/QFtp>
#include <QtNetwork/QUrlInfo>

#include <kdebug.h>
#include <kio/global.h>
#include <kurl.h>

#include "thread.h"

#ifndef MTHREADED_H
#define MTHREADED_H

/**
**/

class Mtget : public QThread
{
    Q_OBJECT
    public:
        Mtget(KUrl src, KUrl dst, int n);
        void run();
        void kill();
        void getRemoteFileInfo();
        void createThreads(KIO::filesize_t totalSize, KIO::filesize_t ProcessedSize, QList<struct data> tdata);
        QList<struct data> getThreadsData();

    signals:
        void totalSize(KIO::filesize_t);
        void processedSize(KIO::filesize_t);
        void speed(unsigned long);
        void update();

    private:
        void openFile();
        void createThreads();
        void createThread(struct data tdata);
        void relocateThread();

        KUrl m_src;
        KUrl m_dst;
        uint m_n;
        bool m_stoped;
        QTimer m_speed_timer;
        unsigned long m_speedbytes;
        QFile *m_file;
        QFtp *m_ftpInfo;
        QHttp *m_httpInfo;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_ProcessedSize;
        QList<Thread*> m_threads;

    private slots:
        void calcSpeed();
        void slotTotalSize(KIO::filesize_t size);
        void slotProcessedSize(KIO::filesize_t bytes);
        void threadsControl(status stat);
        void slotThreadFinished();
};
#endif  // MTHREADED_H
