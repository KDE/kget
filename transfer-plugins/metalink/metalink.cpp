/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "metalink.h"
#include "metalinker.h"
#include "core/kget.h"
#include "core/transfergroup.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <QDomElement>

metalink::metalink(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e)
{
    m_copyjob = 0;
}

void metalink::start()
{
    kDebug(5001) << "metalink::start";
    if(!m_copyjob)
        createJob();

    setStatus(Job::Running, i18nc("transfer state: connecting", "Connecting.."), SmallIcon("connect-creating"));
    setTransferChange(Tc_Status, true);
}

void metalink::stop()
{
    kDebug(5001) << "metalink::Stop";
    if(status() == Stopped)
        return;

    if(m_copyjob)
    {
        m_copyjob->kill(KJob::EmitResult);
        m_copyjob=0;
    }

    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

bool metalink::isResumable() const
{
    return false;
}

void metalink::load(const QDomElement &e)
{
    Transfer::load(e);
}

void metalink::save(const QDomElement &e)
{
    Transfer::save(e);
}


//NOTE: INTERNAL METHODS

void metalink::createJob()
{
    kDebug(5001) << "metalink::createJob()";

    if(!m_copyjob)
    {
        m_copyjob = KIO::get(m_source , KIO::NoReload, KIO::HideProgressInfo);
        connect(m_copyjob,SIGNAL(data(KIO::Job*,const QByteArray &)), SLOT(slotData(KIO::Job*, const QByteArray& )));
        connect(m_copyjob, SIGNAL(result(KJob *)),
                this, SLOT(slotResult(KJob * )));
    }
}

void metalink::slotData(KIO::Job *, const QByteArray& data)
{
    kDebug(5001) << "metalink::slotData() ";
    if (data.size() == 0)
        return;
    m_data.append(data);
}

void metalink::slotResult(KJob * job)
{
    switch (job->error())
    {
        case 0:                            //The download has finished
        case KIO::ERR_FILE_ALREADY_EXIST:  //The file has already been downloaded.
            setStatus(Job::Finished, i18nc("transfer state: finished", "Finished"), SmallIcon("dialog-ok"));
            m_percent = 100;
            m_downloadSpeed = 0;
            m_downloadedSize = m_totalSize;
            setTransferChange(Tc_Percent | Tc_DownloadSpeed);
            break;
        default:
            //There has been an error
            kDebug(5001) << "--  E R R O R  (" << job->error() << ")--";
            setStatus(Job::Aborted, i18nc("transfer state: aborted", "Aborted"), SmallIcon("process-stop"));
            break;
    }
    m_copyjob=0;
    setTransferChange(Tc_Status, true);

    QList<MlinkFileData> mldata =     Metalinker::parseMetalinkFile( m_data );
    if(mldata.isEmpty())
        return;

//     KGet::addGroup(m_source.fileName(), group()->name());
    KGet::addGroup(m_source.fileName());

    QDomDocument doc;
    QDomElement e;
    QDomElement url;
    QList<MlinkFileData>::iterator it = mldata.begin();
    QList<MlinkFileData>::iterator itEnd = mldata.end();

    for ( ; it!=itEnd ; ++it )
    {
        m_dest.setFileName( (*it).fileName );
        e = doc.createElement("Transfer");
        e.setAttribute("Dest", m_dest.url());

        if( (*it).urls.size() > 1 )
        {
            kDebug(5001) << "urls:  " << (*it).urls.size();
            KUrl::List::iterator KUrlit = (*it).urls.begin();
            KUrl::List::iterator KUrlitEnd = (*it).urls.end();
            for ( ; KUrlit!=KUrlitEnd ; ++KUrlit )
            {
                url = doc.createElement("Urls");
                e.appendChild(url);
                url.setAttribute("Url", (*KUrlit).url()); 
            }
        }
        KUrl src = (*it).urls.takeFirst();
        e.setAttribute("Source", src.url());

        KGet::addTransfer(e, m_source.fileName());

        url.clear();
        e.clear();
        KGet::delTransfer(handler());
    }
}

#include "metalink.moc"
