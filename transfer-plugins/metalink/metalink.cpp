/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDomElement>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "metalink.h"
#include "metalinker.h"
#include "../multisegmentkio/MultiSegKio.h"

metalink::metalink(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e)
{
    m_copyjob = 0;
}

void metalink::start()
{
    kDebug(5001) << "metalink::start" << endl;
    if(!m_copyjob)
        createJob();

    setStatus(Job::Running, i18n("Connecting.."), SmallIcon("connect_creating"));
    setTransferChange(Tc_Status, true);

}

void metalink::stop()
{
    kDebug(5001) << "metalink::Stop" << endl;
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

int metalink::elapsedTime() const
{
    return -1; //TODO
}

int metalink::remainingTime() const
{
    return -1; //TODO
}

bool metalink::isResumable() const
{
    return true;
}

void metalink::load(QDomElement e)
{
    Transfer::load(e);
}

void metalink::save(QDomElement e)
{
    Transfer::save(e);
}


//NOTE: INTERNAL METHODS

void metalink::createJob()
{
    kDebug(5001) << "metalink::createJob()" << endl;

    if(!m_copyjob)
    {
        Metalinker mlink;
        QList<MlinkFileData> mldata = mlink.parseMetalinkFile(m_source);
        if(mldata.empty())
            return;
        MlinkFileData ml = mldata.takeFirst();

        m_dest.adjustPath( KUrl::AddTrailingSlash );
        m_dest.setFileName( ml.fileName );
        kDebug(5001) <<  ml.fileName << endl;

        if(SegmentsData.empty())
        {
            m_copyjob = MultiSegfile_copy( ml.urls, m_dest, -1, ml.urls.size() );
        }
        else
        {
            m_copyjob = MultiSegfile_copy( ml.urls, m_dest, -1, m_processedSize, m_totalSize, SegmentsData, SegmentsData.size());
        }
/*        connect(m_copyjob, SIGNAL(updateSegmentsData()),
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
           SLOT(slotSpeed(KJob *, unsigned long)));*/
    }
}

#include "metalink.moc"
