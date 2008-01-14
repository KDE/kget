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

/**
This Class is an interface for inter-plugins data change.
allowing to use already implemented features from others plugins
*/
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
	/**
	Returns data in the forms of chucks
	*/
        void data( const KIO::fileoffset_t &offset,const QByteArray &data );

	/**
	Returns data in the forms of URL List
	*/
        void data( const QList<KUrl> &data );

	/**
	emited when there is no more data
	*/
        void finished();

	/**
	Alert that datasource is no able to send any data
	*/
        void broken();
};
#endif
