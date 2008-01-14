/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef KGET_TRANSFERDATASOURCE_H
#define KGET_TRANSFERDATASOURCE_H

#include "kget_export.h"

#include <QObject>

#include <kio/job.h>

#include "core/bitset.h"

class KGET_EXPORT TransferDataSource : public QObject
{
    Q_OBJECT
    public:
        TransferDataSource(QObject *parent);
        virtual ~TransferDataSource();

	virtual void start() = 0;
	virtual void stop() = 0;
        virtual void addSegment(const KUrl &srcUrl, const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes) = 0;

    signals:
        void data( const KIO::fileoffset_t &offset,const QByteArray &data );
        void finished();
        void broken();
};
#endif
