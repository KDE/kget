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
 * This Class is an interface for inter-plugins data change.
 * allowing to use already implemented features from others plugins
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
         * If true this TransferDataSource can handle multiple segments for one connection
         * @note false by default
         */
        virtual bool canHandleMultipleSegments() const;

        /**
         * Add a segment to be downloaded by this TransferDataSource
         * @param offset the offset of the file to be downloaded
         * @param bytes number of bytes to be downloaded
         * @param segmentNum the number of the segment to identify it
         * @note instead of assigning only one segment (that results in one connection),
         * one can assign multiple segments (all downloaded with one connection) via
         * assignSegments, thus constant calling of addSegment and then reconnecting
         * can be avoided
         */
        virtual void addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum) = 0;//TODO

        /**
         * Adds multiple continuous segments that should be downloaded by this TransferDataSource
         * @param offset the offset of the file to be downloaded
         * @param segmentSize first is the general segmentSize, second the segmentSize
         * of the last segment in the range, if just one segment is assigned both need to have
         * the same value
         * @param segmentRange first the beginning, second the end
         * @note the default implemention will just call addSegment multiple times, then
         * split -- even if implemented -- would not work
         */
        virtual void addSegments(const KIO::fileoffset_t offset, const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);

        /**
         * Removes one connection, useful when setMaximumParalellDownloads was called with a lower number
         * @return the segments that are removed (unassigned) now
         *
         */
        virtual QPair<int, int> removeConnection();

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

        /**
         * Returns the assignedSegments to this TransferDataSource
         * Each connection is represented by a QPair, where the first int is the beginning
         * segment and the last the ending segment
         * @note an empty list is returned by default, the elements can also be (-1, -1)
         */
        virtual QList<QPair<int, int> > assignedSegments() const;

        /**
         * Returns the number of unfinished Segments of the connection with the most
         * unfinished segments
         * Each TransferDataSource can have multiple connections and each connection
         * can have multiple segments assigned
         * @note default implemention returns 0
         */
        virtual int countUnfinishedSegments() const;

        /**
         * Removes one segment of the connection with the most unfinished segments
         * @note this method is useful, if a TransferDataSource does not support multiple-segments
         * as in that case split would result in many unassigned segments
         */
        virtual int takeOneSegment();

        /**
         * If a connection of this TransferDataSource is assigned multiple (continuous) segments, then
         * this method will split them (the unfinished ones) in half, it returns the beginning
         * and the end of the now unassigned segments; (-1, -1) if there are none
         * @note if only one segment is assigned to a connection split will also return (-1, -1)
         */
        virtual QPair<int, int> split();//TODO should split also take the current running segment into account?

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
         * @param connectionFinished true if all segments of this connection have been finished,
         * if one segement (instead of a group of segments) has been asigned this is always true
         */
        void finishedSegment(TransferDataSource *source, int segmentNumber, bool connectionFinished = true);

        /**
         * Alert that datasource is no able to send any data
         *@param source the datasource, sending the signal
         */
        void broken(TransferDataSource *source, Error error);

        /**
         * emitted when an assigned segment is broken
         * @param source the source that emmited this signal
         * @param segmentRange the range of the segments e.g. (1,1,) or (0, 10)
         */
        void brokenSegments(TransferDataSource *source, QPair<int, int> segmentRange);

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
