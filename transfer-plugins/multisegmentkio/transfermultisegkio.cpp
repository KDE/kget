/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfermultisegkio.h"
#ifdef HAVE_NEPOMUK
#include "core/nepomukhandler.h"
#endif //HAVE_NEPOMUK

#include "multisegkiosettings.h"
#include "core/kget.h"
#include "core/transferdatasource.h"
// #include "mirrors.h"

#include <kiconloader.h>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <klocale.h>
#include <kdebug.h>

#include <QDomElement>
#include <QFile>

TransferMultiSegKio::TransferMultiSegKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(0), m_isDownloading(false), stopped(true), m_movingFile(false)
{
}

void TransferMultiSegKio::start()
{
    if (!m_movingFile)
    {
        if(!m_copyjob)
            createJob();

        kDebug(5001);

        setStatus(Job::Running, i18nc("transfer state: connecting", "Connecting...."), SmallIcon("network-connect")); // should be "network-connecting", but that doesn't exist for KDE 4.0 yet
        setTransferChange(Tc_Status, true);
        stopped = false;
    }
}

void TransferMultiSegKio::stop()
{
    kDebug(5001);

    stopped = true;
    if(status() == Stopped)
        return;

    if(m_copyjob)
    {
        m_copyjob->stop();
    }

    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

bool TransferMultiSegKio::isResumable() const
{
    return true;
}

bool TransferMultiSegKio::setDirectory(const KUrl& newDirectory)
{
    KUrl newDest = newDirectory;
    newDest.addPath(m_dest.fileName());
    return setNewDestination(newDest);
}

bool TransferMultiSegKio::setNewDestination(const KUrl &newDestination)
{
if (isResumable() && newDestination.isValid() && (newDestination != dest()))
    {
        KUrl oldPath = KUrl(m_dest.path() + ".part");
        if (oldPath.isValid() && QFile::exists(oldPath.pathOrUrl()))
        {
            m_movingFile = true;
            stop();
            setStatus(Job::Stopped, i18nc("changing the destination of the file", "Changing destination"), SmallIcon("media-playback-pause"));
            setTransferChange(Tc_Status, true);

            m_dest = newDestination;
#ifdef HAVE_NEPOMUK
            nepomukHandler()->setNewDestination(m_dest);
#endif //HAVE_NEPOMUK

            KIO::Job *move = KIO::file_move(oldPath, KUrl(newDestination.path() + ".part"), -1, KIO::HideProgressInfo);
            connect(move, SIGNAL(result(KJob *)), this, SLOT(newDestResult(KJob *)));
            connect(move, SIGNAL(infoMessage(KJob *, const QString &)), this, SLOT(slotInfoMessage(KJob *, const QString &)));
            connect(move, SIGNAL(percent(KJob *, unsigned long)), this, SLOT(slotPercent(KJob *, unsigned long)));

            return true;
        }
    }
    return false;
}

void TransferMultiSegKio::newDestResult(KJob *result)
{
    Q_UNUSED(result);//TODO handle errors etc.!
    m_movingFile = false;
    start();
    setTransferChange(Tc_FileName);
}

void TransferMultiSegKio::postDeleteEvent()
{
    if (status() != Job::Finished)//if the transfer is not finished, we delete the *.part-file
    {
        KIO::Job *del = KIO::del(m_dest.path() + ".part", KIO::HideProgressInfo);
        KIO::NetAccess::synchronousRun(del, NULL);
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
}

void TransferMultiSegKio::load(const QDomElement *element)
{
    kDebug(5001);

    if (!element)
    {
        setStatus(status(), i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
        setStartStatus(status());
        return;
    }

    Transfer::load(element);//TODO is this nescessary for MultiSegKio?!

    const QDomElement e = *element;

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
        kDebug(5001) << "adding Segment " << i;
        SegmentsData << d;
    }
    QDomNodeList urls = e.elementsByTagName ("Urls");
    QDomElement url;
    for( uint i=0 ; i < urls.length () ; ++i )
    {
        node = urls.item(i);
        url = node.toElement ();
        kDebug(5001) << "adding Url " << i;
        m_Urls << KUrl( url.attribute("Url") );
    }
}

void TransferMultiSegKio::save(const QDomElement &element)
{
    kDebug(5001);

    Transfer::save(element);//TODO is this nescessary for MultiSegKio?!

    QDomElement e = element;

    Transfer::save(e);

    QDomDocument doc(e.ownerDocument());
    QDomElement segment;
    QList<SegData>::iterator it = SegmentsData.begin();
    QList<SegData>::iterator itEnd = SegmentsData.end();
    kDebug(5001) << "saving: " << SegmentsData.size() << " segments";
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
        kDebug(5001) << "saving: " << m_Urls.size() << " urls";
        for ( ; it!=itEnd ; ++it )
        {
            url = doc.createElement("Urls");
            e.appendChild(url);
            url.setAttribute("Url", (*it).url()); 
        }
    }
}


//NOTE: INTERNAL METHODS

void TransferMultiSegKio::createJob()
{
//     mirror* searchjob = 0;
    if(!m_copyjob)
    {
        if(m_Urls.empty())
        {
            if(MultiSegKioSettings::useSearchEngines())
            {
                KUrl searchUrl(m_source);
                searchUrl.setProtocol("search");
                TransferDataSource * mirrorSearch = KGet::createTransferDataSource(searchUrl);
                if (mirrorSearch)
                {
                    connect(mirrorSearch, SIGNAL(data(const QList<KUrl>&)),
                        SLOT(slotSearchUrls(const QList<KUrl>&)));
                    mirrorSearch->start();
                }
           }
            m_Urls << m_source;
        }
        if(SegmentsData.empty())
        {
            m_copyjob = MultiSegfile_copy( m_Urls, m_dest, -1,  MultiSegKioSettings::segments());
        }
        else
        {
            m_copyjob = MultiSegfile_copy( m_Urls, m_dest, -1, m_downloadedSize, m_totalSize, SegmentsData, MultiSegKioSettings::segments());
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
        connect(m_copyjob, SIGNAL(speed(KJob *, unsigned long)),
           SLOT(slotSpeed(KJob *, unsigned long)));
    }
}

void TransferMultiSegKio::slotUpdateSegmentsData()
{
    SegmentsData.clear();
    SegmentsData = m_copyjob->SegmentsData();
    KGet::save();
}

void TransferMultiSegKio::slotResult( KJob *kioJob )
{
    kDebug(5001) << "(" << kioJob->error() << ")";
    switch (kioJob->error())
    {
        case 0:                            //The download has finished
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
            setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
            // "ok" icon should probably be "dialog-success", but we don't have that icon in KDE 4.0
            m_percent = 100;
            m_downloadSpeed = 0;
            m_downloadedSize = m_totalSize;
            setTransferChange(Tc_Percent | Tc_DownloadSpeed);
            break;
        default:
            //There has been an error
            kDebug(5001) << "--  E R R O R  (" << kioJob->error() << ")--";
            if (!stopped)
                setStatus(Job::Aborted, i18n("Aborted"), SmallIcon("dialog-error"));
            break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob = 0;
    m_isDownloading = false;
    setTransferChange(Tc_Status, true);
}

void TransferMultiSegKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
    Q_UNUSED(kioJob);
    m_log.append(QString(msg));
}

void TransferMultiSegKio::slotPercent( KJob * kioJob, unsigned long percent )
{
//     kDebug(5001);
    Q_UNUSED(kioJob);
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void TransferMultiSegKio::slotTotalSize( KJob *kioJob, qulonglong size )
{
    Q_UNUSED(kioJob);

    kDebug(5001);

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading...."), SmallIcon("media-playback-start"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void TransferMultiSegKio::slotProcessedSize( KJob *kioJob, qulonglong size )
{
//     kDebug(5001) << "slotProcessedSize"; 

    Q_UNUSED(kioJob);

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading...."), SmallIcon("media-playback-start"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_downloadedSize = size;
    setTransferChange(Tc_DownloadedSize, true);
}

void TransferMultiSegKio::slotSpeed( KJob * kioJob, unsigned long bytes_per_second )
{
//     kDebug(5001) << "slotSpeed: " << bytes_per_second;

    Q_UNUSED(kioJob);

    if (!m_isDownloading)
    {
        setStatus(Job::Running, i18n("Downloading...."), SmallIcon("media-playback-start"));
        m_isDownloading = true;
        setTransferChange(Tc_Status , true);
    }

    m_downloadSpeed = bytes_per_second;
    setTransferChange(Tc_DownloadSpeed, true);
}

void TransferMultiSegKio::slotSearchUrls(const QList<KUrl> &Urls)
{
    kDebug(5001) << "got: " << Urls.size() << " Urls.";
    m_Urls = Urls;

    //add the source URL as the mirrorSearch plugin does not include it in its result
    if(!m_Urls.contains(m_source))
    {
        m_Urls << m_source;
    }
    if (m_copyjob)
    {
        m_copyjob->slotUrls(m_Urls);
    }
}

#include "transfermultisegkio.moc"
