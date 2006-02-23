/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#include"mthreaded.h"

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
        QList<Thread*>::const_iterator it = m_threads.begin();
        QList<Thread*>::const_iterator itEnd = m_threads.end();
        for ( ; it!=itEnd ; ++it )
        {
            (*it)->quit();
        }
}

void Mtget::getRemoteFileInfo()
{
    RemoteFileInfo *FileInfo = new RemoteFileInfo(m_src);
    connect(FileInfo,SIGNAL(size(KIO::filesize_t)),SLOT(slotTotalSize(KIO::filesize_t)));
}

void Mtget::createThreads(KIO::filesize_t totalSize, KIO::filesize_t ProcessedSize, QList<struct data> tdata)
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
    QList<struct data>::const_iterator it = tdata.begin();
    QList<struct data>::const_iterator itEnd = tdata.end();
    for ( ; it!=itEnd ; ++it )
    {
        createThread( (*it) );
    }
    m_stoped = false;
    m_speed_timer.start(2000);
}

QList<struct data> Mtget::getThreadsData()
{
    kDebug () << "Mtget::getThreadsData" << endl;
    QList<struct data> tdata;
    QList<Thread*>::const_iterator it = m_threads.begin();
    QList<Thread*>::const_iterator itEnd = m_threads.end();
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
    struct data tdata;

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

void Mtget::createThread(struct data tdata)
{
    Thread* thread = new Thread(m_file, tdata);
    int id = m_threads.size();
    kDebug() << "Mtget::createThread: " << id << endl;
    m_threads << thread;
    connect(thread, SIGNAL(stat(status)), this, SLOT(threadsControl(status)));
    connect(thread, SIGNAL(finished ()), this, SLOT(slotThreadFinished()));
    connect(thread, SIGNAL(processedSize(KIO::filesize_t)),  this, SLOT(slotProcessedSize(KIO::filesize_t)));
    thread->start();
}

void Mtget::relocateThread()
{
    struct data tdata;
    KIO::filesize_t bytes;
    QList<Thread*>::const_iterator it = m_threads.begin();
    QList<Thread*>::const_iterator itEnd = m_threads.end();
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
//     kDebug() << "Mtget::calcSpeed " << endl;
    m_speedbytes = 0;
}

void Mtget::slotTotalSize(KIO::filesize_t size)
{
    m_totalSize = size;
    emit totalSize(size);
    sender()->deleteLater();
    createThreads();
}

void Mtget::slotProcessedSize(KIO::filesize_t bytes)
{
    m_ProcessedSize += bytes;
    m_speedbytes += bytes;
//     kDebug() <<  m_ProcessedSize <<" bytes downloaded" << endl;
    emit processedSize(m_ProcessedSize);
}

void Mtget::threadsControl(status stat)
{
    Thread* thread = (Thread*) sender();
    struct data tdata;
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

void Mtget::slotThreadFinished()
{
    m_threads.removeAt( m_threads.indexOf( (Thread*)sender() ) );
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
