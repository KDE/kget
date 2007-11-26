/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransferfactory.h"

// header inclusion order is crucial because of signal emit clashes
#include "bttransfer.h"
#include "bttransferhandler.h"
#include "btdetailswidget.h"

#include <kdebug.h>
#include <KStandardDirs>

#include <QFile>

KGET_EXPORT_PLUGIN(BTTransferFactory)

BTTransferFactory::BTTransferFactory()
{
}

BTTransferFactory::~BTTransferFactory()
{
}

Transfer * BTTransferFactory::createTransfer(const KUrl &srcUrl, const KUrl &destUrl,
                                              TransferGroup * parent,
                                              Scheduler * scheduler, 
                                              const QDomElement * e )
{
    kDebug(5001) << "BTTransferFactory::createTransfer";

    //if (!m_srcUrl.isEmpty() || !destUrl.isEmpty() || !parent || !scheduler)
     //   return 0;

    if (srcUrl.fileName().endsWith(".torrent"))
    {
        return new BTTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

/**void BTTransferFactory::downloadTorrent()
{
    kDebug(5001);
    if (src.isLocalFile())
    {
        init();
    }
    else
    {
        kDebug(5001) << "DownloadFile:" << src.url();
        KIO::TransferJob *m_copyJob = KIO::get(src, KIO::NoReload, KIO::HideProgressInfo);
        connect(m_copyJob, SIGNAL(data(KIO::Job*,const QByteArray &)), SLOT(slotData(KIO::Job*, const QByteArray&)));
        connect(m_copyJob, SIGNAL(result(KJob *)), SLOT(slotResult(KJob *)));
        connect(m_copyJob, SIGNAL(finished()), SLOT(init()));
    }
}


void BTTransferFactory::init()
{
}**/

TransferHandler * BTTransferFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    BTTransfer * bttransfer = dynamic_cast<BTTransfer *>(transfer);

    if(!bttransfer)
    {
        kError(5001) << "BTTransferFactory::createTransferHandler: WARNING!\n"
                      "passing a non-BTTransfer pointer!!" << endl;
        return 0;
    }

    return new BTTransferHandler(bttransfer, scheduler);
}

QWidget * BTTransferFactory::createDetailsWidget( TransferHandler * transfer )
{
    BTTransferHandler * bttransfer = static_cast<BTTransferHandler *>(transfer);
    return new BTDetailsWidget(bttransfer);
}

const QList<KAction *> BTTransferFactory::actions()
{
    return QList<KAction *>();
}

/**void BTTransferFactory::slotData(KIO::Job *job, const QByteArray& data)
{
    kDebug(5001);
    if (data.size() == 0)
    {
        slotResult(job);
        return;
    }
    m_data.append(data);
}

void BTTransferFactory::slotResult(KJob * job)
{
    kDebug(5001);
    switch (job->error())
    {
        case 0://The download has finished
        {
            kDebug(5001) << "Downloading successfully finished";
            QFile torrentFile(KStandardDirs::locateLocal("appdata", "tmp/") + m_srcUrl.fileName());
            torrentFile.write(m_data);
            torrentFile.close();
            m_downloadTorrent = KStandardDirs::locateLocal("appdata", "tmp/") + torrentFile.fileName();
            m_data = 0;
            torrentDownloaded = true;
            break;
        }
        case KIO::ERR_FILE_ALREADY_EXIST:
            kDebug(5001) << "ERROR - File already exists";
            m_data = 0;
            torrentDownloaded = true;
        default:
            kDebug(5001) << "That sucks";
            m_data = 0;
            torrentDownloaded = false;
            break;
    }
}
**/