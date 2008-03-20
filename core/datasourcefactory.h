/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/ 
#ifndef DATASOURCEFACTORY_H
#define DATASOURCEFACTORY_H

#include "kget_export.h"

#include "bitset.h"

#include <kio/job.h>
#include <QMap>
#include <QPair>

class TransferDataSource;

/**
 This class manages multiple DataSources and returns the data
 */
class KGET_EXPORT DataSourceFactory : public QObject
{
    Q_OBJECT
    public:
        DataSourceFactory(const KUrl &source, const KUrl &dest, const KIO::fileoffset_t &size, const KIO::fileoffset_t &segSize, QObject *parent);
        ~DataSourceFactory();

        void addDataSource(TransferDataSource *source);
        void removeDataSource(TransferDataSource *source);

    private slots:
        void assignSegment(TransferDataSource *source);
        void writeData(const KIO::fileoffset_t &offset, const QByteArray &data);

    private:
        KUrl m_source;
        KUrl m_dest;
        KIO::fileoffset_t m_size;
        KIO::fileoffset_t m_segSize;
        QMap< TransferDataSource*, QPair<KIO::fileoffset_t, KIO::fileoffset_t> > m_dataSources;//The QPair is <offset, bytes>
        BitSet *m_chunks;
};

#endif
