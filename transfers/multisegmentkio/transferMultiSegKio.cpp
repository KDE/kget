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
#include <kdebug.h>

#include "MultiSegKioSettings.h"
#include "transferMultiSegKio.h"
#include "mirrors.h"

transferMultiSegKio::transferMultiSegKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(0), m_isDownloading(false)
{
    kDebug(5001) << "transferMultiSegKio::transferMultiSegKio" << endl;
    if( e )
        load( *e );
}

void transferMultiSegKio::start()
{
    if(!m_copyjob)
        createJob();

    kDebug(5001) << "transferMultiSegKio::start" << endl;

    setStatus(Job::Running, i18n("Connecting.."), SmallIcon("connect_creating"));
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::stop()
{
    kDebug(5001) << "transferMultiSegKio::Stop()" << endl;

    if(status() == Stopped)
        return;

    if(m_copyjob)
    {
        m_copyjob->stop();
    }

    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("stop"));
    m_speed = 0;
    m_isDownloading = false;
    setTransferChange(Tc_Status | Tc_Speed, true);
}

int transferMultiSegKio::elapsedTime() const
{
    return -1; //TODO
}

int transferMultiSegKio::remainingTime() const
{
    return -1; //TODO
}

bool transferMultiSegKio::isResumable() const
{
    return true;
}

void transferMultiSegKio::load(QDomElement e)
{
    kDebug(5001) << "TransferMultiSegKio::load" << endl;

    SegData d;
    QDomNodeList segments = e.elementsByTagName ("Segment");
    QDomNode node;
    QDomElement segment;
    for( uint i=0 ; i < segments.length () ; ++i )
    {
        node = segments.item(i);
        segment = node.toElement ();
        d.bytes = segment.attribute("Bytes").toULongLong();
        d.offset = segment.attribute("OffSet").toULongLong();
        kDebug(5001) << "TransferMultiSegKio::load: adding Segment " << i << endl;
        SegmentsData << d;
    }
    QDomNodeList urls = e.elementsByTagName ("Urls");
    QDomElement url;
    for( uint i=0 ; i < urls.length () ; ++i )
    {
        node = urls.item(i);
        url = node.toElement ();
        kDebug(5001) << "TransferMultiSegKio::load: adding Url " << i << endl;
        m_Urls << KUrl( url.attribute("Url") );
    }
}

void transferMultiSegKio::save(QDomElement e)
{
    kDebug(5001) << "TransferMultiSegKio::save" << endl;

    Transfer::save(e);

    QDomDocument doc(e.ownerDocument());
    QDomElement segment;
    QList<SegData>::iterator it = SegmentsData.begin();
    QList<SegData>::iterator itEnd = SegmentsData.end();
    kDebug(5001) << "TransferMultiSegKio::saving: " << SegmentsData.size() << " segments" << endl;
    for ( ; it!=itEnd ; ++it )
    {
        segment = doc.createElement("Segment");
        e.appendChild(segment);
        segment.setAttribute("Bytes", (*it).bytes); 
        segment.setAttribute("OffSet", (*it).offset);
    }
    if( m_Urls.size() > 1 )
    {
        QDomElement url;
        QList<KUrl>::iterator it = m_Urls.begin();
        QList<KUrl>::iterator itEnd = m_Urls.end();
        kDebug(5001) << "TransferMultiSegKio::saving: " << m_Urls.size() << " urls" << endl;
        for ( ; it!=itEnd ; ++it )
        {
            url = doc.createElement("Urls");
            e.appendChild(url);
            url.setAttribute("Url", (*it).url()); 
        }
    }
}


//NOTE: INTERNAL METHODS

void transferMultiSegKio::createJob()
{
    if(!m_copyjob)
    {
        if(m_Urls.empty())
        {
            if(MultiSegKioSettings::useSearchEngines())
                m_Urls = MirrorSearch (m_source);
            else
                m_Urls << m_source;
        }
        if(SegmentsData.empty())
        {
            m_copyjob = MultiSegfile_copy( m_Urls, m_dest, -1, false,  MultiSegKioSettings::segments());
        }
        else
        {
            m_copyjob = MultiSegfile_copy( m_Urls, m_dest, -1, false, m_processedSize, m_totalSize, SegmentsData, MultiSegKioSettings::segments());
        }
        connect(m_copyjob, SIGNAL(updateSegmentsData()),
           SLOT(slotUpdateSegmentsData()));
        connect(m_copyjob, SIGNAL(result(KJob *)),
           SLOT(slotResult(KJob *)));
        connect(m_copyjob, SIGNAL(infoMessage(KJob *, const QString &)),
           SLOT(slotInfoMessage(KJob *, const QString &)));
        connect(m_copyjob, SIGNAL(percent(KJob *, unsigned long)),
           SLOT(slotPercent(KJob *, unsigned long)));
        connect(m_copyjob, SIGNAL(totalSize(KJob *, qulonglong)),
           SLOT(slotTotalSize(KJob *, qulonglong)));
        connect(m_copyjob, SIGNAL(processedSize(KJob *, qulonglong)),
           SLOT(slotProcessedSize(KJob *, qulonglong)));
        connect(m_copyjob, SIGNAL(speed(KIO::Job *, unsigned long)),
           SLOT(slotSpeed(KIO::Job *, unsigned long)));
    }
}

void transferMultiSegKio::slotUpdateSegmentsData()
{
    SegmentsData.clear();
    SegmentsData = m_copyjob->SegmentsData();
}

void transferMultiSegKio::slotResult( KJob *kioJob )
{
    kDebug(5001) << "transferMultiSegKio::slotResult  (" << kioJob->error() << ")" << endl;
    switch (kioJob->error())
    {
        case 0:                            //The download has finished
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
            setStatus(Job::Finished, i18n("Finished"), SmallIcon("ok"));
            m_percent = 100;
            m_speed = 0;
            m_processedSize = m_totalSize;
            setTransferChange(Tc_Percent | Tc_Speed);
            break;
        default:
            //There has been an error
            kDebug(5001) << "--  E R R O R  (" << kioJob->error() << ")--" << endl;
            setStatus(Job::Aborted, i18n("Aborted"), SmallIcon("stop"));
            break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob=0;
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
    Q_UNUSED(kioJob);
    m_log.append(QString(msg));
}

void transferMultiSegKio::slotPercent( KJob * kioJob, unsigned long percent )
{
    kDebug(5001) << "transferMultiSegKio::slotPercent" << endl;
    Q_UNUSED(kioJob);
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void transferMultiSegKio::slotTotalSize( KJob *kioJob, qulonglong size )
{
    kDebug(5001) << "transferMultiSegKio::slotTotalSize" << endl;

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("player_play"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void transferMultiSegKio::slotProcessedSize( KJob *kioJob, qulonglong size )
{
    kDebug(5001) << "slotProcessedSize" << endl; 

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("player_play"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_processedSize = size;
    setTransferChange(Tc_ProcessedSize, true);
}

void transferMultiSegKio::slotSpeed( KIO::Job * kioJob, unsigned long bytes_per_second )
{
    kDebug(5001) << "slotSpeed: " << bytes_per_second << endl;

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("player_play"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_speed = bytes_per_second;
    setTransferChange(Tc_Speed, true);
}

#include "transferMultiSegKio.moc"
