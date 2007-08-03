/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "transferKio.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <QDomElement>

TransferKio::TransferKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(0)
{

}

void TransferKio::start()
{
    if(!m_copyjob)
        createJob();

    kDebug(5001) << "TransferKio::start";

    setStatus(Job::Running, i18n("Connecting.."), SmallIcon("connect-creating"));
    setTransferChange(Tc_Status, true);
}

void TransferKio::stop()
{
    if(status() == Stopped)
        return;

    if(m_copyjob)
    {
        m_copyjob->kill(KJob::EmitResult);
        m_copyjob=0;
    }

    kDebug(5001) << "Stop";
    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("process-stop"));
    m_speed = 0;
    setTransferChange(Tc_Status | Tc_Speed, true);
}

int TransferKio::elapsedTime() const
{
    return -1; //TODO
}

int TransferKio::remainingTime() const
{
    return -1; //TODO
}

bool TransferKio::isResumable() const
{
    return true;
}

void TransferKio::load(const QDomElement &e)
{
    Transfer::load(e);
}

void TransferKio::save(const QDomElement &e)
{
    Transfer::save(e);
}


//NOTE: INTERNAL METHODS

void TransferKio::createJob()
{
    if(!m_copyjob)
    {
        m_copyjob = KIO::file_copy(m_source, m_dest, -1, false, false, false);

        connect(m_copyjob, SIGNAL(result(KJob *)), 
                this, SLOT(slotResult(KJob *)));
        connect(m_copyjob, SIGNAL(infoMessage(KJob *, const QString &)), 
                this, SLOT(slotInfoMessage(KJob *, const QString &)));
        connect(m_copyjob, SIGNAL(percent(KJob *, unsigned long)), 
                this, SLOT(slotPercent(KJob *, unsigned long)));
        connect(m_copyjob, SIGNAL(totalSize(KJob *, KIO::filesize_t)), 
                this, SLOT(slotTotalSize(KJob *, KIO::filesize_t)));
        connect(m_copyjob, SIGNAL(processedSize(KJob *, KIO::filesize_t)), 
                this, SLOT(slotProcessedSize(KJob *, KIO::filesize_t)));
        connect(m_copyjob, SIGNAL(speed(KJob *, unsigned long)), 
                this, SLOT(slotSpeed(KJob *, unsigned long)));
    }
}

void TransferKio::slotResult( KJob * kioJob )
{
    kDebug(5001) << "slotResult  (" << kioJob->error() << ")";
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
            kDebug(5001) << "--  E R R O R  (" << kioJob->error() << ")--";
            setStatus(Job::Aborted, i18n("Aborted"), SmallIcon("process-stop"));
            break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob=0;
    setTransferChange(Tc_Status, true);
}

void TransferKio::slotInfoMessage( KJob * kioJob, const QString & msg )
{
  Q_UNUSED(kioJob);
    m_log.append(QString(msg));
}

void TransferKio::slotPercent( KJob * kioJob, unsigned long percent )
{
    kDebug(5001) << "slotPercent";
    Q_UNUSED(kioJob);
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void TransferKio::slotTotalSize( KJob * kioJob, KIO::filesize_t size )
{
    Q_UNUSED(kioJob);

    kDebug(5001) << "slotTotalSize";

    setStatus(Job::Running, i18n("Downloading.."), SmallIcon("media-playback-start"));

    m_totalSize = size;
    setTransferChange(Tc_Status | Tc_TotalSize, true);
}

void TransferKio::slotProcessedSize( KJob * kioJob, KIO::filesize_t size )
{
    Q_UNUSED(kioJob);

    kDebug(5001) << "slotProcessedSize"; 

    if(status() != Job::Running)
    {
        setStatus(Job::Running, i18n("Downloading.."),  SmallIcon("media-playback-start"));
        setTransferChange(Tc_Status, true);

    }
    m_processedSize = size;
    setTransferChange(Tc_ProcessedSize, true);
}

void TransferKio::slotSpeed( KJob * kioJob, unsigned long bytes_per_second )
{
    Q_UNUSED(kioJob);

//     kDebug(5001) << "slotSpeed";

    if(status() != Job::Running)
    {
        setStatus(Job::Running, i18n("Downloading.."), SmallIcon("media-playback-start"));
        setTransferChange(Tc_Status, true);

    }

    m_speed = bytes_per_second;
    setTransferChange(Tc_Speed, true);
}

#include "transferKio.moc"
