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
#include "transfer.h"

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

        /**
         * Returns the capabilities this TransferDataSource supports
         */
        Transfer::Capabilities capabilities() const;

        virtual void start() = 0;
        virtual void stop() = 0;

        /**
         * Tries to find the filesize if this capability is supported,
         * if successfull it emits foundFileSize(TransferDataSource*,KIO::filesize_t,QPair<int,int>)
         * and assigns all segements to itself
         * if not succesfull it will try to download the file nevertheless
         * @note if stop is called and no size is found yet then this is aborted, i.e. needs to be
         * called again if start is later called
         * @param segmentSize the segments should have
         */
        virtual void findFileSize(KIO::fileoffset_t segmentSize);

        /**
         * Adds multiple continuous segments that should be downloaded by this TransferDataSource
         * @param segmentSize first is always the general segmentSize, second the segmentSize
         * of the last segment in the range. If just one (the last) segment was assigned, then
         * first would not equal second, this is to ensure that first can be used to calculate the offset
         * TransferDataSources have to handle all that internally.
         * @param segmentRange first the beginning, second the end
         */
        virtual void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange) = 0;

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
         * If a connection of this TransferDataSource is assigned multiple (continuous) segments, then
         * this method will split them (the unfinished ones) in half, it returns the beginning
         * and the end of the now unassigned segments; (-1, -1) if there are none
         * @note if only one segment is assigned to a connection split will also return (-1, -1)
         */
        virtual QPair<int, int> split();//TODO should split also take the current running segment into account?


        //the following methods are used for managing the number of paralell connections
        //subclasses have to keep track of the currentSegments
        /**
         * @return the number of paralell segments this DataSource is allowed to use,
         * default is 1
         */
        virtual int paralellSegments() const;

        /**
         * Sets the number of paralell segments this DataSource is allowed to use
         */
        virtual void setParalellSegments(int paralellSegments);

        /**
         * @return the number of paralell segments this DataSources currently uses
         */
        virtual int currentSegments() const;

        /**
         * Returns the missmatch of paralellSegments() and currentSegments()
         * @return the number of segments to add/remove e.g. -1 means one segment to remove
         */
        virtual int changeNeeded() const;

    signals:
        /**
         * Emitted after findFileSize is called successfully
         * @param source that foudn the filesize
         * @param fileSize that was found
         * @param segmentRange that was calculated based on the segmentSize and that was assigned to
         * source automatically
         */
        void foundFileSize(TransferDataSource *source, KIO::filesize_t fileSize, const QPair<int, int> &segmentRange);

        /**
         * Emitted when the capabilities of the TransferDataSource change
         */
        void capabilitiesChanged();

        /**
         * Emitted when the TransferDataSource finished the download on its own, e.g. when findFileSize
         * is being called but no fileSize is found and instead the download finishes
         * @param source the source that emmited this signal
         * @param fileSize the fileSize of the finished file (calculated by the downloaded bytes)
         */
        void finishedDownload(TransferDataSource *source, KIO::filesize_t fileSize);

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
        void broken(TransferDataSource *source, TransferDataSource::Error error);

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

        /**
         * Emitted when a Datasource itself decides to not download a specific segmentRange,
         * e.g. when there are too many connections for this TransferDataSource
         */
        void freeSegments(TransferDataSource *source, QPair<int, int> segmentRange);

        void log(const QString &message, Transfer::LogLevel logLevel);

    protected:
        /**
         * Sets the capabilities and automatically emits capabilitiesChanged
         */
        void setCapabilities(Transfer::Capabilities capabilities);

    private Q_SLOTS:
        virtual void slotSpeed(ulong speed) {Q_UNUSED(speed)}

    protected:
        KUrl m_sourceUrl;
        ulong m_speed;
        KIO::filesize_t m_supposedSize;
        int m_paralellSegments;
        int m_currentSegments;

    private:
        Transfer::Capabilities m_capabilities;
};
#endif
