/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDomElement>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "transferMultiSegKio.h"

transferMultiSegKio::transferMultiSegKio(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_copyjob(0)
{

}

void transferMultiSegKio::start()
{
    if(!m_copyjob)
        createJob();

    kDebug() << "transferMultiSegKio::start" << endl;

    setStatus(Job::Running, i18n("Connecting.."), SmallIcon("connect_creating"));
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::stop()
{
    if(status() == Stopped)
        return;

    if(m_copyjob)
    {
        m_copyjob->doKill();
        m_copyjob=0;
    }

    kDebug() << "Stop" << endl;
    setStatus(Job::Stopped, i18n("Stopped"), SmallIcon("stop"));
    m_speed = 0;
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
    Transfer::load(e);
}

void transferMultiSegKio::save(QDomElement e)
{
    Transfer::save(e);
}


//NOTE: INTERNAL METHODS

void transferMultiSegKio::createJob()
{
    if(!m_copyjob)
    {
        m_copyjob = KIO::MultiSegfile_copy( m_source, m_dest, -1, false, 5);
        connect(m_copyjob, SIGNAL(result(KIO::Job *)), 
                this, SLOT(slotResult(KIO::Job *)));
        connect(m_copyjob, SIGNAL(infoMessage(KIO::Job *, const QString &)), 
                this, SLOT(slotInfoMessage(KIO::Job *, const QString &)));
        connect(m_copyjob, SIGNAL(connected(KIO::Job *)), 
                this, SLOT(slotConnected(KIO::Job *)));
        connect(m_copyjob, SIGNAL(percent(KIO::Job *, unsigned long)), 
                this, SLOT(slotPercent(KIO::Job *, unsigned long)));
        connect(m_copyjob, SIGNAL(totalSize(KIO::Job *, KIO::filesize_t)), 
                this, SLOT(slotTotalSize(KIO::Job *, KIO::filesize_t)));
        connect(m_copyjob, SIGNAL(processedSize(KIO::Job *, KIO::filesize_t)), 
                this, SLOT(slotProcessedSize(KIO::Job *, KIO::filesize_t)));
        connect(m_copyjob, SIGNAL(speed(KIO::Job *, unsigned long)), 
                this, SLOT(slotSpeed(KIO::Job *, unsigned long)));
    }
}

void transferMultiSegKio::slotResult( KIO::Job * kioJob )
{
    kDebug() << "slotResult  (" << kioJob->error() << ")" << endl;
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
            kDebug() << "--  E R R O R  (" << kioJob->error() << ")--" << endl;
            setStatus(Job::Aborted, i18n("Aborted"), SmallIcon("stop"));
            break;
    }
    // when slotResult gets called, the m_copyjob has already been deleted!
    m_copyjob=0;
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::slotInfoMessage( KIO::Job * kioJob, const QString & msg )
{
  Q_UNUSED(kioJob);
    m_log.append(QString(msg));
}

void transferMultiSegKio::slotConnected( KIO::Job * kioJob )
{
//     kDebug() << "CONNECTED" <<endl;

  Q_UNUSED(kioJob);
    setStatus(Job::Running, i18n("Downloading.."), SmallIcon("player_play"));
    setTransferChange(Tc_Status, true);
}

void transferMultiSegKio::slotPercent( KIO::Job * kioJob, unsigned long percent )
{
    kDebug() << "slotPercent" << endl;
    Q_UNUSED(kioJob);
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void transferMultiSegKio::slotTotalSize( KIO::Job * kioJob, KIO::filesize_t size )
{
    kDebug() << "slotTotalSize" << endl;

    slotConnected(kioJob);

    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);
}

void transferMultiSegKio::slotProcessedSize( KIO::Job * kioJob, KIO::filesize_t size )
{
    kDebug() << "slotProcessedSize" << endl; 

    if(status() != Job::Running)
        slotConnected(kioJob);

    m_processedSize = size;
    setTransferChange(Tc_ProcessedSize, true);
}

void transferMultiSegKio::slotSpeed( KIO::Job * kioJob, unsigned long bytes_per_second )
{
//     kDebug() << "slotSpeed" << endl;

    if(status() != Job::Running)
        slotConnected(kioJob);

    m_speed = bytes_per_second;
    setTransferChange(Tc_Speed, true);
}

#include "transferMultiSegKio.moc"
