/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef KGETBTCACHE_H
#define KGETBTCACHE_H

#include <diskio/cache.h>
#include <interfaces/cachefactory.h>
#include <kio/job.h>

#include <QString>
#include <QByteArray>

class QStringList;
class KJob;

namespace bt
{
    class Torrent;
    class TorrentFile;
    class Chunk;
    class PreallocationThread;
}

using namespace bt;

class BTCache : public QObject, public bt::Cache
{
    Q_OBJECT
    public:
        BTCache(bt::Torrent & tor,const QString & tmpdir,const QString & datadir);
        ~BTCache();

        /**
         * Load the file map of a torrent.
         * If it doesn't exist, it needs to be created.
         */
        virtual void loadFileMap() {}
        
        /**
         * Save the file map of a torrent
         */
        virtual void saveFileMap() {}
        
        /**
         * Get the actual output path.
         * @return The output path
         */
        virtual QString getOutputPath() const {return QString();}
        
        /**
         * Changes the tmp dir. All data files should already been moved.
         * This just modifies the tmpdir variable.
         * @param ndir The new tmpdir
         */
        virtual void changeTmpDir(const QString & ndir) {Q_UNUSED(ndir)}
        
        /**
         * Changes output path. All data files should already been moved.
         * This just modifies the datadir variable.
         * @param outputpath New output path
         */
        virtual void changeOutputPath(const QString & outputpath) {Q_UNUSED(outputpath)}
        
        /**
         * Move the data files to a new directory.
         * @param ndir The directory
         * @return The job doing the move
         */
        virtual KJob* moveDataFiles(const QString & ndir) {return 0;}
        
        /**
         * A move of a bunch of data files has finished
         * @param job The job doing the move
         */
        virtual void moveDataFilesFinished(KJob* job) {Q_UNUSED(job)}
        
        /**
         * Load a chunk into memory. If something goes wrong,
         * an Error should be thrown.
         * @param c The Chunk
         */
        virtual void load(Chunk* c);
        
        /**
         * Save a chunk to disk. If something goes wrong,
         * an Error should be thrown.
         * @param c The Chunk
         */
        virtual void save(Chunk* c);
        
        /**
         * Prepare a chunk for downloading.
         * @param c The Chunk
         * @return true if ok, false otherwise
         */
        virtual bool prep(Chunk* c);
        
        /**
         * Create all the data files to store the data.
         */
        virtual void create() {}
        
        /**
         * Close the cache file(s).
         */
        virtual void close() {}
        
        /**
         * Open the cache file(s)
         */
        virtual void open() {}
        
        /// Does nothing, can be overridden to be alerted of download status changes of a TorrentFile
        virtual void downloadStatusChanged(TorrentFile*, bool) {}
        
        /**
         * Preallocate diskspace for all files
         * @param prealloc The thread doing the preallocation
         */
        virtual void preallocateDiskSpace(PreallocationThread* prealloc) {Q_UNUSED(prealloc)}
        
        /**
         * Test all files and see if they are not missing.
         * If so put them in a list
         */
        virtual bool hasMissingFiles(QStringList & sl) {return false;} //We never have missing files, cause we don't have files :P
        
        /**
         * Delete all data files, in case of multi file torrents
         * empty directories should also be deleted.
         */
        virtual KJob* deleteDataFiles() {return 0;}//TODO: Implement!!!
        virtual bt::PieceData* loadPiece(bt::Chunk*, bt::Uint32, bt::Uint32) {return 0;}
        virtual bt::PieceData* preparePiece(bt::Chunk*, bt::Uint32, bt::Uint32) {return 0;}
        virtual void savePiece(bt::PieceData*) {}
        
        /**
         * Get the number of bytes all the files of this torrent are currently using on disk.
         * */
        virtual Uint64 diskUsage() {return 0;};//We always use 0 Bytes on HDD, cause we don't write to HDD

    signals:
        void dataArrived(const KIO::fileoffset_t &offset, const QByteArray &data);

    private:
        Torrent *m_tor;
};

class BTCacheFactory : public QObject, public CacheFactory
{
    Q_OBJECT
    public:
        BTCacheFactory() {}
        ~BTCacheFactory() {}

        virtual Cache* create(Torrent & tor,const QString & tmpdir,const QString & datadir);

    signals:
        void cacheAdded(BTCache* cache);
};

#endif
