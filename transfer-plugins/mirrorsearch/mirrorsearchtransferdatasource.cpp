/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "mirrorsearchtransferdatasource.h"
#include "mirrors.h"

#include "kget_debug.h"
#include <QDebug>

MirrorSearchTransferDataSource::MirrorSearchTransferDataSource(const QUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent)
{
    m_filename = m_sourceUrl.fileName();
    qCDebug(KGET_DEBUG) << m_filename;
}

void MirrorSearchTransferDataSource::start()
{
    qCDebug(KGET_DEBUG);
    if(!m_filename.isEmpty())
        MirrorSearch (m_filename, this, SLOT(slotSearchUrls(QList<QUrl>&)));
}

void MirrorSearchTransferDataSource::stop()
{
    qCDebug(KGET_DEBUG);
}

void MirrorSearchTransferDataSource::addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange)
{
    Q_UNUSED(segmentSize)
    Q_UNUSED(segmentRange)
    qCDebug(KGET_DEBUG);
}

void MirrorSearchTransferDataSource::slotSearchUrls(QList<QUrl>& Urls)
{
    emit data(Urls);
}


