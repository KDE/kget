/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "mmsTransfer.h"

#include "core/kget.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include <QFile>
#include <QTimer>

MmsTransfer::MmsTransfer(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      mmsx(0)
{
    kDebug(5001) << "Mms transfer initialized: " + m_source.prettyUrl();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(read()));
}

void MmsTransfer::start()
{
    kDebug(5001) << "Trying to start Mms-transfer: " + m_source.prettyUrl();
    setStatus(Job::Running, i18nc("transfer state: running", "Running.."), SmallIcon("network-connect")); // should be "network-connecting", but that doesn't exist for KDE 4.0 yet
    setTransferChange(Tc_Status, true);
    mmsx = mmsx_connect(NULL, NULL, m_source.url().toAscii().data(), 1);
    if (mmsx)
        kDebug(5001) << "Connection ok";
    else
        kDebug(5001) << "WARNING ----------- No connection to source";
    timer->start(500);//Read data every 500 ms
}

void MmsTransfer::stop()
{
    if(status() == Stopped)
        return;
    timer->stop();
    kDebug(5001) << "Stop";
    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

bool MmsTransfer::isResumable() const
{
    return true;
}

void MmsTransfer::postDeleteEvent()
{
    /**if (status() != Job::Finished)//if the transfer is not finished, we delete the *.part-file
    {
        QString dest = m_dest.path() + ".part";
        kDebug(5001) << dest;

        QFile destFile(dest);

        destFile.remove();
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)**/
}

void MmsTransfer::read()
{
    char *data;
    int result = mmsx_read(NULL, mmsx, data, mmsx_get_asf_packet_len(mmsx));
    kDebug(5001) << data;
}

#include "mmsTransfer.moc"
