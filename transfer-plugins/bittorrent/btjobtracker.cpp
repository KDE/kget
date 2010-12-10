/* This file is part of the KDE project

   Copyright (C) 2010 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btjobtracker.h"

#include <KDebug>

#if LIBKTORRENT_VERSION >= 0x010100
#include <torrent/job.h>
#include <torrent/torrentcontrol.h>
#include "scandlg.h"
BTJobTracker::BTJobTracker(QObject * parent)
  : KJobTrackerInterface(parent)
{

}
BTJobTracker::~BTJobTracker()
{
}

void BTJobTracker::registerJob(KJob * job)
{
    kDebug() << "Register job";
    bt::Job * j = static_cast<bt::Job*>(job);
    if (j->torrentStatus() == bt::CHECKING_DATA) {
        kDebug() << "It's checkin data";
        kt::ScanDlg *scanDlg = new kt::ScanDlg(job, 0);
        scanDlg->show();
    }
}

void BTJobTracker::unregisterJob(KJob * job)
{
}
#endif
