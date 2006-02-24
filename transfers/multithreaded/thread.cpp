/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include"thread.h"

#warning "!!!WARNING!!! we need a patched qt-copy for mulithreaded ftp with an offset. patch: ftp-offset.diff in this dir. if your qt is patched, enable the following line!"
//#define KGET_HAVE_PATCHED_QFTP

RemoteFileInfo::RemoteFileInfo(KUrl src)
{
   kDebug() << "RemoteFileInfo::RemoteFileInfo" << endl; 
   Iface * m_iface = 0;

    if(src.protocol() == "ftp")
    {
        m_iface = static_cast<Iface *> (new Ftpiface());
    }
    if(src.protocol()=="http")
    {
        m_iface = static_cast<Iface *> (new Httpiface());
    }
    if(m_iface)
    {
    connect (m_iface, SIGNAL(totalSize(KIO::filesize_t)), SIGNAL(size(KIO::filesize_t)));
    m_iface->getRemoteFileInfo(src);
    }
}

Iface *newTransferThread(QFile *file, struct data tdata)
{
    kDebug() << "newTransferThread()" << endl;

    if(tdata.src.protocol() == "ftp")
    {
        return static_cast<Iface *> (new Ftpiface(file, tdata));
    }
    if(tdata.src.protocol()=="http")
    {
        return static_cast<Iface *> (new Httpiface(file, tdata));
    }
}

/**
Astract Base interface
**/

Iface::Iface(QFile *file, struct data tdata)
    : m_file ( file ) ,
      m_data ( tdata )
{
    connect (this, SIGNAL(started()), SLOT(slotStart()));
}

void Iface::run()
{
    exec();
}

struct data Iface::getThreadData()
{
    return m_data;
}

void Iface::setBytes(KIO::filesize_t bytes)
{
    m_data.bytes = bytes;
}

void Iface::slotStart()
{
    kDebug() << "Iface::slotStart" << endl;

    if( m_data.bytes == 0 )
   {
    quit();
    return;
    }
    setup();
    startDownload();
}

/**
Ftp interface
**/

Ftpiface::Ftpiface(QFile *file, struct data tdata)
    :Iface(file, tdata),
     ftp(0)
{
    kDebug() << "Ftpiface::Ftpiface" << endl;
}

Ftpiface::Ftpiface()
    :ftp(0)
{
    kDebug() << "Ftpiface::Ftpiface" << endl;
    setup();
}

void Ftpiface::setup()
{
    kDebug() << "Ftpiface::setup" << endl;
    if(ftp)
    {
        ftp->abort();
        ftp->deleteLater();
    }
    ftp = new QFtp(this);
    connect(ftp, SIGNAL(readyRead()), this, SLOT(slotWriteBuffer()));
    connect(ftp, SIGNAL(commandFinished(int, bool)),
                this, SLOT(slotCommandFinished(int, bool)));
    connect(ftp, SIGNAL(listInfo(const QUrlInfo &)),
                this, SLOT(slotlistInfo(const QUrlInfo &)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)),
                this, SLOT(slotDataTransferProgress(qint64, qint64)));
}

void Ftpiface::startDownload()
{
    kDebug() << "Ftpiface::run" <<  endl;
    ftp->connectToHost(m_data.src.host());
    ftp->login();
#ifdef KGET_HAVE_PATCHED_QFTP
    ftp->get(m_data.src.path(), m_data.offSet);
#else
    ftp->get(m_data.src.path());
#endif
    ftp->close();
}

void Ftpiface::getRemoteFileInfo(KUrl src)
{
    kDebug() << "Ftpiface::getRemoteFileInfo" <<  endl;
    ftp->connectToHost(src.host());
    ftp->login();
    ftp->list(src.path());
    ftp->close();
}

void Ftpiface::slotWriteBuffer()
{
    KIO::fileoffset_t bytesReaded=0, len=0;
    char buff[BUFFER_SIZE];
    if(m_data.bytes>BUFFER_SIZE)
    {
        len=BUFFER_SIZE;
    }
    else
    {
        len=m_data.bytes;
    }
    bytesReaded = ftp->read(buff,len);
    if(bytesReaded>0)
    {
        m_mutex.lock();
        m_file->seek(m_data.offSet);
        m_file->write(buff,bytesReaded);
        m_data.offSet += bytesReaded;
        m_data.bytes -= bytesReaded;
        bytes += bytesReaded;
        m_mutex.unlock();
        emit processedSize(bytesReaded);
    }
    kDebug() << "Offset: " << m_data.offSet << " Bytes: " << m_data.bytes <<  endl;
    if ( bytes > BUFFER_SIZE )
    {
        bytes = 0;
//         emit stat(buffer_ready);
    }
    if ( m_data.bytes == 0 )
    {
        disconnect(ftp,SIGNAL(readyRead()),this,SLOT(slotWriteBuffer()));
        ftp->abort();
        ftp->deleteLater();
        kDebug() << "Ftpiface::slotWriteBuffer: Clossing connection" <<  endl;
        emit stat(closed);
    }
}

void Ftpiface::slotlistInfo(const QUrlInfo & i)
{
    emit totalSize(i.size());
}

/**
Http interface
**/

Httpiface::Httpiface(QFile *file, struct data tdata)
    :Iface(file, tdata),
     http(0)
{
    kDebug() << "Httpiface::Httpiface" <<  endl;
}

Httpiface::Httpiface()
    :http(0)
{
    kDebug() << "Httpiface::Httpiface" <<  endl;
    setup();
}

void Httpiface::setup()
{
    kDebug() << "Httpiface::setup" << endl;
    if(http)
    {
        http->abort();
        http->deleteLater();
    }
    http = new QHttp(this);
    connect(http,SIGNAL(readyRead(const QHttpResponseHeader & )),
                this,SLOT(slotWriteBuffer(const QHttpResponseHeader &)));
    connect(http, SIGNAL(requestFinished(int, bool)),
                this, SLOT(slotRequestFinished(int, bool)));
    connect(http, SIGNAL(dataReadProgress(int, int)),
                this, SLOT(slotDataReadProgress(int, int)));
    connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
                this, SLOT(slotResponseHeader(const QHttpResponseHeader &)));
}

void Httpiface::startDownload()
{
    kDebug() << "Httpiface::startDownload" <<  endl;
    QHttpRequestHeader header("GET", m_data.src.path());
    header.setValue("Host", m_data.src.host());
    header.setValue("Range","bytes=" + QString::number(m_data.offSet) + "-");
    header.setValue("Connection","close");
    http->setHost(m_data.src.host());
    http->request(header);
}

void Httpiface::getRemoteFileInfo(KUrl src)
{
    kDebug() << "Httpiface::getRemoteFileInfo" <<  endl;
    http->setHost(src.host());
    http->head(src.path());
    http->close();
}

void Httpiface::slotWriteBuffer(const QHttpResponseHeader &)
{
    KIO::fileoffset_t bytesReaded=0, len=0;
    char buff[BUFFER_SIZE];
    if( m_data.bytes > BUFFER_SIZE )
    {
        len=BUFFER_SIZE;
    }
    else
    {
        len=m_data.bytes;
    }
    bytesReaded=http->read(buff,len);
    if( bytesReaded > 0 )
    {
        m_mutex.lock();
        m_file->seek(m_data.offSet);
        m_file->write(buff,bytesReaded);
        m_data.offSet += bytesReaded;
        m_data.bytes -= bytesReaded;
//         bytes += bytesReaded;
        m_mutex.unlock();
        emit processedSize(bytesReaded);
    }
//     kDebug() << "Offset: " << m_data.offSet << " Bytes: " << m_data.bytes <<  endl;
/*    if(bytes > BUFFER_SIZE)
    {
        bytes = 0;
        emit stat(buffer_ready);
    }*/
    if( m_data.bytes == 0 )
    {
        disconnect(http,SIGNAL(readyRead(const QHttpResponseHeader &)),this,SLOT(slotWriteBuffer(const QHttpResponseHeader &)));
        http->abort();
//         http->deleteLater();
        kDebug() << "Httpiface::slotWriteBuffer: Clossing connection" << endl;
        emit stat(closed);
    }
}

void Httpiface::slotResponseHeader(const QHttpResponseHeader & resp)
{
    emit totalSize(resp.contentLength());
}

void Httpiface::slotRequestFinished(int, bool)
{

}
