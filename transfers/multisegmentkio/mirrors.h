/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef MIRROR_H
#define MIRROR_H

#include <QtCore/QObject>
#include <QtCore/QEventLoop>

#include <kdebug.h>
#include <kio/job.h>

class KIO_EXPORT mirror : public QObject
{
   Q_OBJECT

public:
   mirror();
   QList<KUrl> search(const KUrl &url);

private Q_SLOTS:

   void slotData(KIO::Job *, const QByteArray& data);
   void slotResult( KJob *job );

private:

   void URLRequest(const KUrl &url);

private:

   QString m_search_engine;
   KIO::TransferJob *m_job;
   QList<KUrl> m_Urls;
   QByteArray m_data;
   QEventLoop EventLoop;
};

QList<KUrl> MirrorSearch ( const KUrl &url );

QList<KUrl> Urls( QDomElement root );

#endif // MIRROR_H
