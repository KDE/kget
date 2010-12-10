/* This file is part of the KDE project

   Copyright (C) 2010 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef BTJOBTRACKER_H
#define BTJOBTRACKER_H

#include <version.h>

#if LIBKTORRENT_VERSION >= 0x010100
#include <kjobtrackerinterface.h>

class BTJobTracker : public KJobTrackerInterface
{
    Q_OBJECT
    public:
        BTJobTracker(QObject * parent);
        ~BTJobTracker();
        
        void registerJob(KJob * job);
        void unregisterJob(KJob * job);
};
#endif

#endif
