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

#include <kio/job.h>

#include <QtXml/QDomElement>

class BitSet;
class KioDownload;
class TransferDataSource;
class QTimer;
class Signature;
class Verifier;

namespace KIO
{
    class FileJob;
}

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
         * @note when you want to load a datasourcefactory you do not have to specify the url and segSize
         */
        explicit DataSourceFactory(QObject *parent, const KUrl &dest = KUrl(), KIO::filesize_t size = 0, KIO::fileoffset_t segSize = 512000);

        ~DataSourceFactory();

        /**
         * The capabilities the DataSourceFactory supports
         */
        Transfer::Capabilities capabilities() const {return m_capabilities;}

        /**
         * Deletes the created (downloadInitialized() is true) file if the download was not finished
         * Does not delete anything if the download never got started
         * @see downloadInitialized()
         */
        void deinit();

        /**
         * @return true if the DataSourceFactory has enough information to start a download
         */
        bool isValid() const;

        void start();
        void stop();
        KIO::filesize_t size() const {return m_size;}
        KIO::filesize_t downloadedSize() const {return m_downloadedSize;}
        ulong currentSpeed() const {return m_speed;}
        ulong percent() const {return m_percent;}

        KUrl dest() const {return m_dest;}

        /**
         * The maximum number of mirrors that will be used for downloading, default is 3
         */
        int maxMirrorsUsed() const {return m_maxMirrorsUsed;}

        /**
         * Change the maximum number off mirrors that will be used for downloading,
         * if the download started already some mirrors might be added or removed automatically
         */
        void setMaxMirrorsUsed(int maxMirrorsUsed) {m_maxMirrorsUsed = maxMirrorsUsed;}

        /**
         * Add a mirror that can be used for downloading
         * @param url the url to the file
         * @param used defines whether the mirror should initially be used for downloading or not,
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
         * Returns whether the datasourcefactory should download the file or not,
         * true by default
         * @note can be used for multiple datasourcefactory downloads
         */
        bool doDownload() const {return m_doDownload;}

        /**
         * Set if the datasourcefactory should download the file or not,
         * if set to false the download will be stopped if needed
         * @note can be used for multiple datasourcefactory downloads
         */
        void setDoDownload(bool doDownload);

        bool setNewDestination(const KUrl &newDest);

        Job::Status status() const {return m_status;}

        /**
         * @return true if the download was already initialized, i.e. a file has been
         * created and maybe even written to
         * @see deinit()
         */
        bool downloadInitialized() const {return m_downloadInitialized;}

        /**
         * Tries to repair a broken download, via completely redownloading it
         * or only the borken parts
         * @note call this if verification returned NotVerified
         */
        void repair();

        Verifier *verifier();
        Signature *signature();

    Q_SIGNALS:
        void capabilitiesChanged();
        void dataSourceFactoryChange(Transfer::ChangesFlags change);
        void log(const QString &message, Transfer::LogLevel logLevel);

    public slots:
        void save(const QDomElement &element);
        void load(const QDomElement *e);

    private slots:
        void slotUpdateCapabilities();

        void slotRemovedFile();

        /**
         * Tries to find the size of the file, automatically called
         * by start if no file size has been specified
         */
        void findFileSize();

        void slotFoundFileSize(TransferDataSource *source, KIO::filesize_t fileSize, const QPair<int,int> &segmentRange);

        void assignSegments(TransferDataSource *source);
        /**
         * Called when segments are broken
         */
        void brokenSegments(TransferDataSource *source, const QPair<int, int> &segmentRange);
        void finishedSegment(TransferDataSource *source, int segmentNumber, bool connectionFinished = true);

        /**
         * A TransferDataSource is broken
         */
        void broken(TransferDataSource *source, TransferDataSource::Error error);
        /**
         * Emitted when a Datasource itself decides to not download a specific segmentRange,
         * e.g. when there are too many connections for this TransferDataSource
         */
        void slotFreeSegments(TransferDataSource *source, QPair<int, int> segmentRange);
        void slotWriteData(KIO::fileoffset_t offset, const QByteArray &data, bool &worked);
        void slotOffset(KIO::Job *job, KIO::filesize_t offset);
        void slotDataWritten(KIO::Job *job, KIO::filesize_t offset);
        void slotPercent(KJob *job, ulong percent);
        void open(KIO::Job *job);
        void speedChanged();
        /**
         * Kills the putjob and starts the moving of files
         */
        void startMove();
        void slotPutJobDestroyed(QObject *job);
        void newDestResult(KJob *job);

        void slotRepair(const QList<KIO::fileoffset_t> &offsets, KIO::filesize_t length);

        void slotFinishedDownload(TransferDataSource *source, KIO::filesize_t size);

    private:
        /**
         * Add a mirror that can be used for downloading
         * @param used always true if usedDefined is false
         * @param usedDefined true if the user defined used, otherwise false,
         * needed to know if m_maxMirrorsUsed should be changed or not
         */
        void addMirror(const KUrl &url, bool used, int numParalellConnections, bool usedDefined);

        /**
         * Checks if an assign is needed, i.e. there are no (running) TransferDataSources,
         * yet some segements are still not finished
         */
        bool assignNeeded() const;

        bool checkLocalFile();

        void init();
        void killPutJob();
        void changeStatus(Job::Status status);

    private:
        Transfer::Capabilities m_capabilities;
        KUrl m_dest;
        KUrl m_newDest;
        KIO::filesize_t m_size;
        KIO::filesize_t m_downloadedSize;
        QList<KIO::filesize_t> m_prevDownloadedSizes;
        KIO::fileoffset_t m_segSize;
        ulong m_speed;
        ulong m_percent;

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
        bool m_findFilesizeTried;

        bool m_assignTried;
        bool m_movingFile;

        bool m_finished;

        /**
         * True if download gets started the first time, if it gets never started there
         * is no reason to remove any -- maybe preexisting -- file
         */
        bool m_downloadInitialized;

        /**
         * Wether the file-size has been initially defined (it is to be trusted) or not
         */
        bool m_sizeInitiallyDefined;

        /**
         * Downloadsize has only been found out once the download was finished
         */
        bool m_sizeFoundOnFinish;

        int m_maxMirrorsUsed;
        QHash<KUrl, TransferDataSource*> m_sources;
        QList<KUrl> m_unusedUrls;
        QList<int> m_unusedConnections;
        QTimer *m_speedTimer;
        Job::Status m_status;
        Job::Status m_statusBeforeMove;

        Verifier *m_verifier;
        Signature *m_signature;
};

#endif
