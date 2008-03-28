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

class TransferDataSource;

namespace KIO
{
    class FileJob;
}

/**
 This class manages multiple DataSources and saves the data to the file
 */
class KGET_EXPORT DataSourceFactory : public QObject
{
    Q_OBJECT
    public:
        DataSourceFactory(const KUrl &dest, const KIO::fileoffset_t &size, const KIO::fileoffset_t &segSize, QObject *parent);
        ~DataSourceFactory();

        void start();
        void stop();

        void addDataSource(TransferDataSource *source, const KUrl &url);
        void removeDataSource(TransferDataSource *source);

    private slots:
        void assignSegment(TransferDataSource *source = 0);
        void writeData(const KIO::fileoffset_t &offset, const QByteArray &data);

    private:
        KUrl m_dest;
        KIO::fileoffset_t m_size;
        KIO::fileoffset_t m_segSize;
        QMap< TransferDataSource*, KUrl> m_dataSources;//The QPair is <offset, bytes>
        BitSet *m_chunks;
        KIO::FileJob* m_putJob;
};

#endif
