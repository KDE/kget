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

#ifndef MTHREADED_H
#define MTHREADED_H

#define MIN_SIZE      (100*1024)
#define BUFFER_SIZE   (10*1024)

struct connd
{
    KUrl src;
    KIO::fileoffset_t offSet;
    KIO::filesize_t bytes;
};

enum status
{
    closed = 0,
    buffer_ready
};

class Connection : public QThread
{
    Q_OBJECT
    public:
        Connection(QFile *file, struct connd tdata);
        void run();
        struct connd getThreadData();
        void setBytes(KIO::filesize_t bytes);
        void restart();

    signals:
        void stat(status);
        void processedSize(KIO::filesize_t);
        void totalSize(KIO::filesize_t);

    private:
        void ftpGet();
        void httpGet();
        void getDst();

        QTimer m_timer;
        QMutex m_mutex;
        QHttp *m_http;
        QFtp  *m_ftp;
        QFile *m_file;
        bool isFtp, isHttp;
        struct connd m_data;
        KIO::fileoffset_t bytes;

    private slots:
        void ftpWriteBuffer();
        void httpWriteBuffer(const QHttpResponseHeader & resp);
        void slotStart();
        void slotTimeout();
        void slotRestart();
};

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
        void createThreads(KIO::filesize_t totalSize, KIO::filesize_t ProcessedSize, QList<struct connd> tdata);
        QList<struct connd> getThreadsData();

    signals:
        void totalSize(KIO::filesize_t);
        void processedSize(KIO::filesize_t);
        void speed(unsigned long);
        void update();

    private:
        void openFile();
        void createThreads();
        void createThread(struct connd tdata);
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
        QList<Connection*> m_threads;

    private slots:
        void calcSpeed();
        void slotProcessedSize(KIO::filesize_t bytes);
        void httpFileInfo(const QHttpResponseHeader & resp);
        void ftpFileInfo(const QUrlInfo & i);
        void threadsControl(status stat);
        void slotConnectionFinished();
};
#endif  // MTHREADED_H
