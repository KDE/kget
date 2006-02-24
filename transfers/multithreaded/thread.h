/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include <QtCore>
#include <QtNetwork>

#include <kdebug.h>
#include <kio/global.h>
#include <kurl.h>


#ifndef THREAD_H
#define THREAD_H

#define MIN_SIZE      (100*1024)
#define BUFFER_SIZE   (10*1024)

class Ftpiface;
class Httpiface;
class Iface;

struct data
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

class RemoteFileInfo : public QObject
{
    Q_OBJECT

    public:
        RemoteFileInfo(KUrl src);
    signals:
        void size(KIO::filesize_t);
};

Iface *newTransferThread(QFile *file, struct data tdata);

class Iface : public QThread
{
    Q_OBJECT

    public:
        Iface(QFile *file, struct data tdata);
        Iface() {};
        struct data getThreadData();
        void setBytes(KIO::filesize_t bytes);
        void run();
        virtual void setup()=0;
        virtual void startDownload()=0;
        virtual void getRemoteFileInfo(KUrl src)=0;

    protected:
        struct data m_data;
        QFile *m_file;
        QMutex m_mutex;
        KIO::fileoffset_t bytes;

    signals:
        void totalSize(KIO::filesize_t);
        void speed(unsigned long);
        void processedSize(KIO::filesize_t);
        void stat(status);

    private slots:
        void slotStart();
};

class Ftpiface : public Iface
{
    Q_OBJECT

    public:
        Ftpiface(QFile *file, struct data tdata);
        Ftpiface();
        void setup();
        void startDownload();
        void getRemoteFileInfo(KUrl src);

    private:
        QFtp *ftp;

    private slots:
        void slotWriteBuffer();
        void slotlistInfo(const QUrlInfo & i);

};

class Httpiface: public Iface
{
    Q_OBJECT

    public:
        Httpiface(QFile *file, struct data tdata);
        Httpiface();
        void setup();
        void startDownload();
        void getRemoteFileInfo(KUrl src);

    private:
        QHttp *http;

    private slots:
        void slotWriteBuffer(const QHttpResponseHeader &);
        void slotResponseHeader(const QHttpResponseHeader & resp);
        void slotRequestFinished(int, bool);
};

#endif  // THREAD_H
