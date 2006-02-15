/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include"mthreaded.h"
#include"mthreaded.moc"

Connection::Connection(QFile *file, struct connd tdata)
    : m_file ( file ) ,
      m_data ( tdata )
{
    kDebug() << "Connection::Connection" << endl;
    isFtp = false;
    isHttp = false;
    connect (this, SIGNAL(started()), SLOT(slotStart()));
}

void Connection::run()
{
    kDebug() << "Connection::run" << endl;
    exec();
}

struct connd Connection::getThreadData()
{
    return m_data;
}

void Connection::setBytes(KIO::filesize_t bytes)
{
    m_data.bytes = bytes;
}

void Connection::ftpGet()
{
    kDebug() << "Connection::ftpGet()" <<  endl;
    m_ftp = new QFtp(this);
    m_ftp->connectToHost(m_data.src.host());
    m_ftp->login();
    #warning "!!!WARNING!!! we need a patched qt-copy for mulithreaded ftp with an offset. patch: ftp-offset.diff in this dir. if your qt is patched, enable the following line!"
    m_ftp->get(m_data.src.path(), m_data.offSet);
    //and disable the following line in order to use QFtp with an offset
//     m_ftp->get(m_data.src.path());
    m_ftp->close();
    connect(m_ftp, SIGNAL(readyRead()), this, SLOT(ftpWriteBuffer()));
}

void Connection::httpGet()
{
    kDebug() << "Connection::httpGet()" <<  endl;
    QHttpRequestHeader header("GET", m_data.src.path());
    header.setValue("Host", m_data.src.host());
    header.setValue("Range","bytes=" + QString::number(m_data.offSet) + "-");
    header.setValue("Connection","close");
    m_http = new QHttp(this);
    m_http->setHost(m_data.src.host());
    m_http->request(header);
    connect(m_http,SIGNAL(readyRead(const QHttpResponseHeader & )),this,SLOT(httpWriteBuffer(const QHttpResponseHeader &)));
}

void Connection::getDst()
{
    kDebug() << "Connection::getDst: starting download" << endl;
    if (isFtp)
    {
        ftpGet();
        return;
    }
    if (isHttp)
    {
        httpGet();
        return;
    }
}

void Connection::ftpWriteBuffer()
{
    m_timer.start();
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
    bytesReaded=m_ftp->read(buff,len);
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
//     kDebug() << "Offset: " << m_data.offSet << " Bytes: " << m_data.bytes <<  endl;
    if ( bytes > BUFFER_SIZE )
    {
        bytes = 0;
        emit stat(buffer_ready);
    }
    if ( m_data.bytes == 0 )
    {
        disconnect(m_ftp,SIGNAL(readyRead()),this,SLOT(ftpWriteBuffer()));
        m_ftp->abort();
        m_ftp->deleteLater();
        kDebug() << "Connection::ftpWriteBuffer: Clossing connection" <<  endl;
        emit stat(closed);
    }
}

void Connection::httpWriteBuffer(const QHttpResponseHeader & resp)
{
    Q_UNUSED(resp);
    m_timer.start();
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
    bytesReaded=m_http->read(buff,len);
    if( bytesReaded > 0 )
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
//     kDebug() << "Offset: " << m_data.offSet << " Bytes: " << m_data.bytes <<  endl;
    if(bytes > BUFFER_SIZE)
    {
        bytes = 0;
        emit stat(buffer_ready);
    }
    if( m_data.bytes == 0 )
    {
        disconnect(m_http,SIGNAL(readyRead(const QHttpResponseHeader &)),this,SLOT(httpWriteBuffer(const QHttpResponseHeader &)));
        m_http->abort();
        m_http->deleteLater();
        kDebug() << "Connection::httpWriteBuffer: Clossing connection" << endl;
        emit stat(closed);
    }
}

void Connection::slotStart()
{
    kDebug() << "Connection::slotStart()" << endl;

    if( m_data.bytes == 0 )
   {
    quit();
    return;
    }
    m_timer.setSingleShot(true);
    m_timer.setInterval(60000);
    connect(&m_timer, SIGNAL(timeout()), SLOT(slotTimeout()));
    isFtp = (m_data.src.protocol() == "ftp");
    isHttp = (m_data.src.protocol()=="http");
    getDst();
}

void Connection::slotTimeout()
{
    if(isFtp && m_ftp)
    {
        m_ftp->abort();
        m_ftp->deleteLater();
    }
    if(isHttp && m_http)
    {
        m_http->abort();
        m_http->deleteLater();
    }
    emit stat(closed);
}

void Connection::slotRestart()
{
    kDebug() << "Restarting..." << endl;
    getDst();
}

/**
*
*
**/

Mtget::Mtget(KUrl src, KUrl dst, int n)
    :m_src(src),
    m_dst(dst),
    m_n(n),
    m_stoped(false),
    m_speedbytes(0),
    m_file(0),
    m_totalSize(0),
    m_ProcessedSize(0)
{
    kDebug() << "Mtget::Mtget" << endl;
    connect(&m_speed_timer, SIGNAL(timeout()), SLOT(calcSpeed()));
}

void Mtget::run()
{
    kDebug () << "Mtget::run()" << endl;
    exec();
}

void Mtget::kill()
{
        m_stoped = true;
        QList<Connection*>::const_iterator it = m_threads.begin();
        QList<Connection*>::const_iterator itEnd = m_threads.end();
        for ( ; it!=itEnd ; ++it )
        {
            (*it)->quit();
        }
}

void Mtget::getRemoteFileInfo()
{
    kDebug () << "Mtget::getRemoteFileInfo: " << m_src.url() << endl;
    if(m_src.protocol() == "ftp")
    {
        m_ftpInfo=new QFtp(this);
        m_ftpInfo->connectToHost(m_src.host());
        m_ftpInfo->login();
        connect(m_ftpInfo,SIGNAL(listInfo (const QUrlInfo &)),this,SLOT(ftpFileInfo(const QUrlInfo &)));
        m_ftpInfo->list(m_src.path());
        m_ftpInfo->close();
    }

    if(m_src.protocol() == "http")
    {
        m_httpInfo=new QHttp(this);
        m_httpInfo->setHost(m_src.host());
        connect(m_httpInfo,SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),this,SLOT(httpFileInfo(const QHttpResponseHeader &)));
        m_httpInfo->head(m_src.path());
        m_httpInfo->close();
    }
}

void Mtget::createThreads(KIO::filesize_t totalSize, KIO::filesize_t ProcessedSize, QList<struct connd> tdata)
{
    if(m_threads.size()!=0)
    {
        kDebug () << "Mtget::createThreads(,,,): there already threads working" << endl;
        return;
    }
    openFile();
    if(!m_file)
    {
        return;
    }
    m_totalSize = totalSize;
    m_ProcessedSize = ProcessedSize;
    QList<struct connd>::const_iterator it = tdata.begin();
    QList<struct connd>::const_iterator itEnd = tdata.end();
    for ( ; it!=itEnd ; ++it )
    {
        createThread( (*it) );
    }
    m_stoped = false;
    m_speed_timer.start(2000);
}

QList<struct connd> Mtget::getThreadsData()
{
    kDebug () << "Mtget::getThreadsData" << endl;
    QList<struct connd> tdata;
    QList<Connection*>::const_iterator it = m_threads.begin();
    QList<Connection*>::const_iterator itEnd = m_threads.end();
    for ( ; it!=itEnd ; ++it )
    {
        tdata << (*it)->getThreadData();
    }
    return tdata;
}

void Mtget::openFile()
{
    if(m_dst.protocol() != "file")
    {
    exit();
    }
    QString dst_part( m_dst.path() );
    dst_part += QLatin1String(".part");
    if(QFile::exists(dst_part))
    {
        kDebug() << "Warning file exist: " << m_dst.path() << endl;
    }
    m_file = new QFile(dst_part);
    if ( m_file->open( QIODevice::ReadWrite ) )
    {
        kDebug() << "local file: " << dst_part << endl;
    }
    else
    {
        kDebug() << "Error: " << m_file->error() << " Opening: " << dst_part << endl;
        delete m_file;
        m_file=0;
        exit();
    }
}

void Mtget::createThreads()
{
    struct connd tdata;

    openFile();
    if(!m_file)
    {
        return;
    }
    uint min = m_totalSize/MIN_SIZE;
    if(min < m_n)
    {
        m_n = min;
    }
    if(min == 0)
    {
        m_n = 1;
    }
    KIO::filesize_t conn_size = m_totalSize/m_n;
    KIO::fileoffset_t rest_size = conn_size + (m_totalSize%m_n);
    kDebug () << "Mtget::createThreads: threads:" << m_n << endl;
    for(uint i = 0; i < m_n; i++)
    {
        if(i == m_n - 1)

        {
            tdata.src = m_src;
            tdata.offSet = i*conn_size;
            tdata.bytes = rest_size;
            createThread(tdata);
    kDebug () << "threads data: " << i*conn_size << " "<< rest_size<< endl;
            continue;
        }
    tdata.src = m_src;
    tdata.offSet = i*conn_size;
    tdata.bytes = conn_size;
    createThread(tdata);
    kDebug () << "threads data: " << i*conn_size << " "<< conn_size << endl;
    }
    emit update();
    m_speed_timer.start(2000);
}

void Mtget::createThread(struct connd tdata)
{
    Connection* thread = new Connection(m_file, tdata);
    int id = m_threads.size();
    kDebug() << "Mtget::createThread: " << id << endl;
    m_threads << thread;
    connect(thread, SIGNAL(stat(status)), this, SLOT(threadsControl(status)));
    connect(thread, SIGNAL(finished ()), this, SLOT(slotConnectionFinished()));
    connect(thread, SIGNAL(processedSize(KIO::filesize_t)),  this, SLOT(slotProcessedSize(KIO::filesize_t)));
    thread->start();
}

void Mtget::relocateThread()
{
    struct connd tdata;
    KIO::filesize_t bytes;
    QList<Connection*>::const_iterator it = m_threads.begin();
    QList<Connection*>::const_iterator itEnd = m_threads.end();
    for ( ; it!=itEnd ; ++it )
    {
        tdata = (*it)->getThreadData();
        kDebug() <<"Mtget::relocateThread() -- Bytes: " <<  tdata.bytes << endl;
        if ( tdata.bytes > MIN_SIZE )
        {
            bytes = tdata.bytes/2;
            (*it)->setBytes(bytes);
        kDebug() <<" Mtget::relocateThread() -- realocating tread: " << bytes << endl;
            tdata.offSet += bytes;
            tdata.bytes = bytes + tdata.bytes%2;
            createThread(tdata);
            break;
        }
    }
}

void Mtget::calcSpeed()
{
    emit speed( m_speedbytes/2 );
    kDebug() << "Mtget::calcSpeed " << endl;
    m_speedbytes = 0;
}

void Mtget::slotProcessedSize(KIO::filesize_t bytes)
{
    m_ProcessedSize += bytes;
    m_speedbytes += bytes;
//     kDebug() <<  m_ProcessedSize <<" bytes downloaded" << endl;
    emit processedSize(m_ProcessedSize);
}

void Mtget::httpFileInfo(const QHttpResponseHeader & resp)
{
    disconnect(m_httpInfo,SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),this,SLOT(httpFileInfo(const QHttpResponseHeader &)));
    m_totalSize = resp.contentLength();
    m_httpInfo->deleteLater();
    emit totalSize(m_totalSize);
    kDebug() << "httpFileInfo: " << m_totalSize << endl;
    createThreads();
}

void Mtget::ftpFileInfo(const QUrlInfo & i)
{
    disconnect(m_ftpInfo,SIGNAL(listInfo (const QUrlInfo &)),this,SLOT(ftpFileInfo(const QUrlInfo &)));
    m_ftpInfo->deleteLater();
    m_totalSize = i.size();
    emit totalSize(m_totalSize);
    kDebug() << "ftpFileInfo: " << m_totalSize << endl;
    createThreads();
}

void Mtget::threadsControl(status stat)
{
    Connection* thread = (Connection*) sender();
    struct connd tdata;
    switch(stat)
    {
    case closed:
        emit update();
        tdata = thread->getThreadData();
        if(tdata.bytes == 0)
        {
            thread->quit();
        }
        else
        {
            kDebug() << "Waiting 5 seg..." << endl;
            QTimer::singleShot(5000, thread, SLOT(slotRestart()));
        }
        break;
    case buffer_ready:
        emit update();
        break;
    }
}

void Mtget::slotConnectionFinished()
{
    m_threads.removeAt( m_threads.indexOf( (Connection*)sender() ) );
    delete sender();
    kDebug() << m_threads.size() << " threads left"<<endl;

    if(!m_stoped)
    {
        relocateThread();
    }

    if(m_threads.empty())
    {
        m_file->close();
        if ( m_ProcessedSize==m_totalSize)
       {
            emit update();
            QFile::rename ( m_file->fileName(), m_dst.path() );
            kDebug() << "Download completed" << endl;
       }
        quit();
    }
}
