/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDomElement>

#include <kiconloader.h>
#include <klocale.h>
#include <kio/job.h>
#include <kdebug.h>

#include "transferMultiThreaded.h"
#include "transferMultiThreaded.moc"

TransferMultiThreaded::TransferMultiThreaded(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_Mtjob(0)
{
    kDebug() << "TransferMultiThreaded::TransferMultiThreaded" << endl;
    if( e )
    {
        load( *e );
    }
}

void TransferMultiThreaded::start()
{
    kDebug() << "TransferMultiThreaded::start" << endl;

    if(!m_Mtjob)
    {
        m_Mtjob = new Mtget(m_source, m_dest, 10);
        connect(m_Mtjob, SIGNAL(update()), this, SLOT(slotUpdate()));
        connect(m_Mtjob, SIGNAL(totalSize(KIO::filesize_t)), this, SLOT(slotTotalSize(KIO::filesize_t)));
        connect(m_Mtjob, SIGNAL(processedSize(KIO::filesize_t)), this, SLOT(slotProcessedSize(KIO::filesize_t)));
        connect(m_Mtjob, SIGNAL(speed( unsigned long )), this, SLOT(slotSpeed( unsigned long )));
        connect(m_Mtjob, SIGNAL(finished ()), this, SLOT(slotResult()));
    }
    startJob();

    setStatus(Job::Running, i18n("Connecting.."), SmallIcon("connect_creating"));
    setTransferChange(Tc_Status, true);
}

void TransferMultiThreaded::stop()
{
    kDebug() << "TransferMultiThreaded::stop" << endl;

    if(status() == Stopped)
        return;

    if(m_Mtjob)
    {
        m_Mtjob->kill(true);
    }

    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("stop"));
    m_speed = 0;
    setTransferChange(Tc_Status | Tc_Speed, true);
}

int TransferMultiThreaded::elapsedTime() const
{
    return -1; //TODO
}

int TransferMultiThreaded::remainingTime() const
{
    return -1; //TODO
}

bool TransferMultiThreaded::isResumable() const
{
    return true;
}

void TransferMultiThreaded::load(QDomElement e)
{

    kDebug() << "TransferMultiThreaded::load" << endl;
    struct connd data;
    QDomNodeList threads = e.elementsByTagName ("Thread");
    QDomNode node;
    QDomElement thread;
    for( uint i=0 ; i < threads.length () ; i++ )
    {
        node = threads.item(i);
        thread = node.toElement ();
        data.src = KUrl::fromPathOrURL(thread.attribute("Source"));
        data.bytes = thread.attribute("Bytes").toULongLong();
        data.offSet = thread.attribute("OffSet").toULongLong();
        kDebug() << "TransferMultiThreaded::load: adding thread " << i << endl;
        tdata << data;
    }

}

void TransferMultiThreaded::save(QDomElement e)
{
    Transfer::save(e);

    kDebug() << "TransferMultiThreaded::save" << endl;

    QDomDocument doc(e.ownerDocument());
    QDomElement thread;
    QList<struct connd>::iterator it = tdata.begin();
    QList<struct connd>::iterator itEnd = tdata.end();
    for ( ; it!=itEnd ; ++it )
    {
        thread = doc.createElement("Thread");
        e.appendChild(thread);
        thread.setAttribute("Source", (*it).src.url());
        thread.setAttribute("Bytes", (*it).bytes); 
        thread.setAttribute("OffSet", (*it).offSet);
     }
}


//NOTE: INTERNAL METHODS

void TransferMultiThreaded::startJob()
{
    if(m_Mtjob)
    {
        m_Mtjob->start();
        if( tdata.empty())
        {
            m_Mtjob->getRemoteFileInfo();
        }
        else
        {
            kDebug() << "TransferMultiThreaded::startJob: restarting saved transfer" << endl;
            m_Mtjob->createThreads(m_totalSize, m_processedSize, tdata);
        }
    }
}

void TransferMultiThreaded::slotUpdate()
{
    tdata.clear();
    tdata = m_Mtjob->getThreadsData();
}

void TransferMultiThreaded::slotResult()
{
    delete m_Mtjob;
    m_Mtjob = 0;
    if( m_processedSize == m_totalSize )
    {
        setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
        m_percent = 100;
        m_speed = 0;
        setTransferChange(Tc_Status | Tc_Percent | Tc_Speed, true);
    }
}

// void TransferKio::slotInfoMessage( KIO::Job * kioJob, const QString & msg )
// {
//   Q_UNUSED(kioJob);
//     m_log.append(QString(msg));
// }

void TransferMultiThreaded::slotTotalSize( KIO::filesize_t size )
{
    kDebug() << "slotTotalSize" << endl;

    setStatus(Job::Running, i18n("Downloading.."), SmallIcon("player_play"));
    setTransferChange(Tc_Status, true);

    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void TransferMultiThreaded::slotProcessedSize( KIO::filesize_t size )
{
    kDebug() << "slotProcessedSize" << endl; 

    if(status() != Job::Running)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("player_play"));
        setTransferChange(Tc_Status, true);
    }

    m_processedSize = size;
    slotUpdate();
    setTransferChange(Tc_ProcessedSize, true);

    if (m_totalSize > 0) 
    {
        m_percent = (int)((100.0 * m_processedSize) / m_totalSize);
        setTransferChange(Tc_Percent, true);
    }

}

void TransferMultiThreaded::slotSpeed( unsigned long bytes_per_second )
{
    m_speed = bytes_per_second;
    setTransferChange(Tc_Speed, true);
}

