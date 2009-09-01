/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef TRANSFERDATASOURCE_H
#define TRANSFERDATASOURCE_H

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
        TransferDataSource(const KUrl &srcUrl, QObject *parent);
        virtual ~TransferDataSource();

        enum Error
        {
            Unknown,
            WrongDownloadSize,
            NotResumeable
        };

        virtual void start() = 0;
        virtual void stop() = 0;

        /**
         * Add a segment to be downloaded by this TransferDataSource
         * @param offset the offset of the file to be downloaded
         * @param bytes number of bytes to be downloaded
         * @param segmentNum the number of the segment to identify it
         */
        virtual void addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum) = 0;//TODO

        KUrl sourceUrl() const {return m_sourceUrl;}//TODO

        /**
         * returns the current speed of this data source
         * @return the speed
         */
        ulong currentSpeed() const {return m_speed;}

        /**
         * sets the maximum number of paralell downloads this TransferDataSource is allowed to have
         * @note default is 1
         * @param maxDownloads the maximum paralell downloads
         */
        void setMaximumParalellDownloads(int maxDownloads) {m_maxParalellDownloads = maxDownloads;}
        int maximumParalellDownloads() const {return m_maxParalellDownloads;}

        /**
         * Set the size the server used for downloading should report
         * @param supposedSize the size the file should have
         */
        virtual void setSupposedSize(KIO::filesize_t supposedSize) {m_supposedSize = supposedSize;}

    signals:
        /**
         * Returns data in the forms of chucks
         * @note if the receiver set worked to wrong the TransferDataSource should cache the data
         * @param offset the offset in the file
         * @param data the downloaded data
         * @param worked if the receiver could handle the data, if not, the sender should cache the data
         */
        void data(KIO::fileoffset_t offset, const QByteArray &data, bool &worked);

        /**
         * Returns data in the forms of URL List
         * @param data in form of KUrl list
         */
        void data(const QList<KUrl> &data);

        /**
         * Returns found checksums with their type
         * @param type the type of the checksum
         * @param checksum the checksum
         */
        void data(const QString type, const QString checksum);

        /**
         * emitted when there is no more data
         * @param source the datasource, sending the signal
         */
        void finished();

        /**
         * emitted when an assigned segment finishes
         * @param source the source that emmited this signal
         * @param segmentNumber the number of the segment, to identify it
         */
        void finishedSegment(TransferDataSource *source, int segmentNumber);

        /**
         * Alert that datasource is no able to send any data
         *@param source the datasource, sending the signal
         */
        void broken(TransferDataSource *source, Error error);

        /**
         * emitted when an assigned segment is broken
         * @param source the source that emmited this signal
         * @param segmentNumber the number of the segment, to identify it
         */
        void brokenSegment(TransferDataSource *source, int segmentNumber);

        /**
         * The speed of the download
         * @param speed speed of the download
         */
        void speed(ulong speed);

    private Q_SLOTS:
        virtual void slotSpeed(ulong speed) {Q_UNUSED(speed);}

    protected:
        KUrl m_sourceUrl;
        ulong m_speed;
        int m_maxParalellDownloads;
        KIO::filesize_t m_supposedSize;
};
#endif
