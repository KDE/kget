/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
 
#include "mirrorsearchtransferdatasource.h"
#include "mirrors.h"
#include <kdebug.h>

MirrorSearchTransferDataSource::MirrorSearchTransferDataSource(const KUrl &srcUrl, QObject *parent)
  : TransferDataSource(srcUrl, parent)
{
    m_filename = m_sourceUrl.fileName();
    kDebug(5001) << m_filename;
}

void MirrorSearchTransferDataSource::start()
{
    kDebug(5001);
    if(!m_filename.isEmpty())
        MirrorSearch (m_filename, this, SLOT(slotSearchUrls(QList<KUrl>&)));
}

void MirrorSearchTransferDataSource::stop()
{
    kDebug(5001);
}

void MirrorSearchTransferDataSource::addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum)
{
    Q_UNUSED(offset)
    Q_UNUSED(bytes)
    Q_UNUSED(segmentNum)
    kDebug(5001);
}

void MirrorSearchTransferDataSource::slotSearchUrls(QList<KUrl>& Urls)
{
    emit data(Urls);
}

#include "mirrorsearchtransferdatasource.moc"
