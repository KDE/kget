/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTDOWNLOAD_H
#define BTDOWNLOAD_H

#include <QObject>
#include <QByteArray>

#include <KUrl>

#include <kio/job.h>

class BTDownload : public QObject
{
    Q_OBJECT
    public:
        BTDownload(const KUrl &srcUrl);

    Q_SIGNALS:
        void finishedSuccessfully(KUrl dest);
        void finishedWithError();

    private slots:
        void slotResult(KJob * job);
        void slotData(KIO::Job *job, const QByteArray& data);

    private:
        KUrl m_srcUrl;
        KUrl m_destUrl;
        QByteArray m_data;
};

#endif
