/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MIRROR_H
#define MIRROR_H

#include <QtCore/QObject>
#include <kio/job.h>

class mirror : public QObject
{
    Q_OBJECT

    public:
        mirror();
        void search(const KUrl &url, QObject *receiver, const char *member);
        void search(const QString &fileName, QObject *receiver, const char *member);

    Q_SIGNALS:

        void urls (QList<KUrl>&);

    private Q_SLOTS:

        void slotData(KIO::Job *, const QByteArray& data);
        void slotResult( KJob *job );

    private:

        QString m_search_engine;
        KIO::TransferJob *m_job;
        KUrl m_url;
        QList<KUrl> m_Urls;
        QByteArray m_data;
};

void MirrorSearch ( const KUrl &url, QObject *receiver, const char *member );

#endif // MIRROR_H
