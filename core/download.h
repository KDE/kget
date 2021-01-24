/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "kget_export.h"

#include <QObject>
#include <QByteArray>

#include <QUrl>

#include <kio/job.h>

class KGET_EXPORT Download : public QObject
{
    Q_OBJECT
    public:
        Download(const QUrl &srcUrl, const QUrl &destUrl);
        ~Download() override;

    Q_SIGNALS:
        void finishedSuccessfully(QUrl dest, QByteArray data);
        void finishedWithError();

    private slots:
        void slotResult(KJob * job);
        void slotData(KIO::Job *job, const QByteArray& data);

    private:
        KIO::TransferJob *m_copyJob = nullptr;
        QUrl m_srcUrl;
        QUrl m_destUrl;
        QUrl m_destFile;
        QByteArray m_data;
};

#endif
