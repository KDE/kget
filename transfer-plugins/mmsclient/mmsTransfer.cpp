/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "client.cpp"
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
    : Transfer(parent, factory, scheduler, source, dest, e)
{
    kDebug(5001) << "Mms transfer initialized: " + m_source.prettyUrl();
    m_mmsClientThread = new MMSClientThread(source.prettyUrl());
//     timer = new QTimer(this);
//     connect(timer, SIGNAL(timeout()), SLOT(read()));
}

void MmsTransfer::start()
{
    m_mmsClientThread->start();
    kDebug(5001) << "Trying to start Mms-transfer: " + m_source.prettyUrl();
    setStatus(Job::Running, i18nc("transfer state: running", "Running...."), SmallIcon("network-connect")); // should be "network-connecting", but that doesn't exist for KDE 4.0 yet
    setTransferChange(Tc_Status, true);
}

void MmsTransfer::stop()
{
    if(status() == Stopped)
        return;

    kDebug(5001) << "Stop";
    setStatus(Job::Stopped, i18nc("transfer state: stopped", "Stopped"), SmallIcon("process-stop"));
    m_downloadSpeed = 0;
    setTransferChange(Tc_Status | Tc_DownloadSpeed, true);
}

bool MmsTransfer::isResumable() const
{
    return true;
}


void MmsTransfer::read()
{
    char *data;
    kDebug(5001) << data;
}

#include "mmsTransfer.moc"
