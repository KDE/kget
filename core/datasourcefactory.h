/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/ 
#ifndef DATASOURCEFACTORY_H
#define DATASOURCEFACTORY_H

#include "kget_export.h"

#include "transferdatasource.h"
#include "job.h"
#include "core/verifier.h"

#include <kio/job.h>

#include <QtXml/QDomElement>

class BitSet;
class KioDownload;
class TransferDataSource;
class QTimer;

namespace KIO
{
    class FileJob;
}

/**
 * Contains information about the datasources and handles its deletion
 */
class DataSource
{
    public:
        DataSource(TransferDataSource *transferDataSource);
        ~DataSource();

        TransferDataSource *transferDataSource() {return m_dataSource;}

        /**
         * @return the number of paralell segments this DataSource is allowed to use
         */
        int paralellSegments() const {return m_paralellSegments;}

        /**
         * Sets the number of paralell segments this DataSource is allowed to use
         */
        void setParalellSegments(int paralellSegments) {m_paralellSegments = paralellSegments;}

        /**
         * @return the number of paralell segments this DataSources currently uses
         */
        int currentSegments() const {return m_currentSegments;}

        /**
         * Sets the number of paralell segments this DataSources currently uses
         */
        void setCurrentSegments(int currentSegments) {m_currentSegments = currentSegments;}

        /**
         * Returns the missmatch of paralellSegments() and currentSegments()
         * @return the number of segments to add/remove e.g. -1 means one segment to remove
         */
        int changeNeeded() const {return m_paralellSegments - m_currentSegments;}

    private:
        TransferDataSource *m_dataSource;
        int m_paralellSegments;
        int m_currentSegments;
};

/**
 This class manages multiple DataSources and saves the received data to the file
 */
class KGET_EXPORT DataSourceFactory : public QObject
{
    Q_OBJECT
    public:
        /**
         * In general use this constructor, if the size is 0, the datasourcefactory will try to
         * find the filesize
         */
        DataSourceFactory(const KUrl &dest, KIO::filesize_t size, KIO::fileoffset_t segSize, QObject *parent);

        /**
         * @note us this constructor only if you are loading a DataSourceFactory
         */
        DataSourceFactory(QObject* parent);

        ~DataSourceFactory();

        /**
         * @return true if the DataSourceFactory has enough information to start a download
         */
        bool isValid() const;

        void start();
        void stop();
        KIO::filesize_t size() const;
        KIO::filesize_t downloadedSize() const;
        ulong currentSpeed() const;

        KUrl dest() const;

        /**
         * The maximum number of mirrors that will be used for downloading, default is 3
         */
        int maxMirrorsUsed() const;

        /**
         * Change the maximum number off mirrors that will be used for downloading,
         * if the download started already some mirrors might be added or removed automatically
         */
        void setMaxMirrorsUsed(int maxMirrorsUsed);

        /**
         * Add a mirror that can be used for downloading
         * @param url the url to the file
         * @param used defines wether the mirror should initially be used for downloading or not,
         * if true m_maxMirrorsUsed might be increased if needed
         * @param numParalellConnections the number of simultanous connections allowed to that mirror,
         * minimum is 1
         * @note when you add an already existing mirror only the numParalellConnections are adapted
         * to the new value, so to change the number of paralell connections of a mirror you are already
         * using simply call addMirror again
         */
        void addMirror(const KUrl &url, bool used, int numParalellConnections = 1);

        /**
         * Add a mirror that can be used for downloading, if it will be used depends if maxMirrorsUsed
         * has been reached yet
         * @param url the url to the file
         * @param numParalellConnections the number of simultanous connections allowed to that mirror,
         * minimum is 1
         * @note when you add an already existing mirror only the numParalellConnections are adapted
         * to the new value, so to change the number of paralell connections of a mirror you are already
         * using simply call addMirror again
        */
        void addMirror(const KUrl &url, int numParalellConnections = 1);

        /**
         * Does not use the specified mirror for downloading the file
         * @note if the mirror has bee unsed for downloading it will be moved to m_unusedMirrors,
         * otherwise nohting will happen
         * @param url the mirror that should not be used anymore
         */
        void removeMirror(const KUrl &url);

        /**
         * Sets the mirrors that should be used/not used for downloading
         * @param mirrors url of the mirror, if it should be used and its number of paralell connections
         * (minimum is 1)
         * @note if you want the download to work at least one entry should be set to true
         */
        void setMirrors(const QHash<KUrl, QPair<bool, int> > &mirrors);

        /**
         * Return all mirrors, where bool defines if the mirror is used,
         * while in defines the number of paralell connections for that mirror
         */
        QHash<KUrl, QPair<bool, int> > mirrors() const;

        /**
         * Returns wether the datasourcefactory should download the file or not,
         * true by default
         * @note can be used for multiple datasourcefactory downloads
         */
        bool doDownload() const;

        /**
         * Set if the datasourcefactory should download the file or not,
         * if set to false the download will be stopped if needed
         * @note can be used for multiple datasourcefactory downloads
         */
        void setDoDownload(bool doDownload);

        void postDeleteEvent();

        bool setNewDestination(const KUrl &newDest);

        Job::Status status() const;

        /**
         * Tries to repair a broken download, via completly redownloading it
         * or only the borken parts
         * @note call this if verification returned NotVerified
         */
        void repair();

        Verifier *verifier();

    Q_SIGNALS:
        void processedSize(KIO::filesize_t size);
        void totalSize(KIO::filesize_t size);
        void speed(ulong speed);
        void percent(ulong percent);
        void statusChanged(Job::Status status);

    public slots:
        void save(const QDomElement &element);
        void load(const QDomElement *e);

    private slots:
        /**
         * Tries to find the size of the file, automatically called
         * by start if no file size has been specified
         */
        void findFileSize();

        void assignSegment(TransferDataSource *source);
        /**
         * Called when a segment is broken
         */
        void brokenSegment(TransferDataSource *source, int segmentNumber);
        void finishedSegment(TransferDataSource *source, int segmentNumber);

        /**
         * A TransferDataSource is broken
         */
        void broken(TransferDataSource *source, TransferDataSource::Error error);
        void slotWriteData(KIO::fileoffset_t offset, const QByteArray &data, bool &worked);
        void slotOffset(KIO::Job *job, KIO::filesize_t offset);
        void slotDataWritten(KIO::Job *job, KIO::filesize_t offset);
        void slotPercent(KJob *job, ulong percent);
        void open(KIO::Job *job);
        void speedChanged();
        void sizeFound(KIO::filesize_t size);
        void finished();
        /**
         * Kills the putjob and starts the moving of files
         */
        void startMove();
        void newDestResult(KJob *job);

        void slotRepair(const QList<QPair<KIO::fileoffset_t,KIO::filesize_t> > &brokenPieces);

    private:
        /**
        * Add a mirror that can be used for downloading
        * @param used always true if usedDefined is false
        * @param usedDefined true if the user defined used, otherwise false, needed to know if m_maxMirrorsUsed
        * should be changed or not
        */
        void addMirror(const KUrl &url, bool used, int numParalellConnections, bool usedDefined);

        bool checkLocalFile();

        void init();
        void killPutJob();
        void changeStatus(Job::Status status, bool loaded = false);

    private:
        KUrl m_dest;
        KUrl m_newDest;
        KIO::filesize_t m_size;
        KIO::filesize_t m_downloadedSize;
        KIO::filesize_t m_prevDownloadedSize;
        KIO::fileoffset_t m_segSize;
        ulong m_speed;
        QHash<int, KUrl> m_assignedChunks;
        /**
         * the cache of data that could not be written yet
         */
        QHash<KIO::fileoffset_t, QByteArray> m_cache;

        KIO::filesize_t m_tempOffset;
        QByteArray m_tempData;

        BitSet *m_startedChunks;
        BitSet *m_finishedChunks;
        KIO::FileJob* m_putJob;
        bool m_doDownload;
        bool m_open;
        /**
         * the write access is currently blocked, the data gets cached in m_cache
         */
        bool m_blocked;
        /**
         * If start() was called but did not work this is true, once the conditions changed
         * start() could be recalled
         */
        bool m_startTried;

        bool m_assignTried;
        bool m_movingFile;

        bool m_finished;

        int m_maxMirrorsUsed;
        QHash<KUrl, DataSource*> m_sources;
        QList<KUrl> m_unusedUrls;
        QList<int> m_unusedConnections;
        QTimer *m_speedTimer;
        KioDownload *m_tempDownload;
        Job::Status m_status;
        Job::Status m_statusBeforeMove;

        Verifier *m_verifier;
};

#endif
