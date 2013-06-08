/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef TRANSFERHANDLER_H
#define TRANSFERHANDLER_H

#include <QVariant>

#include "handler.h"
#include "transfer.h"
#include "transfergroup.h"
#include "../kget_export.h"

class QAction;
class KPassivePopup;

class KGetKJobAdapter;

/**
 * Class TransferHandler:
 *
 * --- Overview ---
 * This class is the representation of a Transfer object from the views'
 * perspective (proxy pattern). In fact the views never handle directly the 
 * Transfer objects themselves (because this would break the model/view policy).
 * As a general rule, all the code strictly related to the views should placed 
 * here (and not in the transfer implementation).
 * Here we provide the same api available in the transfer class, but we change
 * the implementation of some methods.
 * Let's give an example about this:
 * The start() function of a specific Transfer (let's say TransferKio) is a 
 * low level command that really makes the transfer start and should therefore
 * be executed only by the scheduler.
 * The start() function in this class is implemented in another way, since
 * it requests the scheduler to execute the start command to this specific transfer.
 * At this point the scheduler will evaluate this request and execute, if possible,
 * the start() function directly in the TransferKio.
 */

class KGET_EXPORT TransferHandler : public Handler
{
    Q_OBJECT
    friend class KGet;
    friend class TransferTreeModel;
    friend class Transfer;
    friend class TransferFactory;
    friend class TransferGroupHandler;
    public:

        typedef Transfer::ChangesFlags ChangesFlags;

        TransferHandler(Transfer * parent, Scheduler * scheduler);

        virtual ~TransferHandler();

        Job::Status status() const {return m_transfer->status();}
        Job::Error error() const {return m_transfer->error();}
        Job::Status startStatus() const {return m_transfer->startStatus();}
        int elapsedTime() const;
        int remainingTime() const;
        
        void resolveError(int errorId) {m_transfer->resolveError(errorId);}

        /**
         * Returns the capabilities the Transfer supports
         */
        Transfer::Capabilities capabilities() const;

        /**
         * Tries to repair file
         * @param file the file of a download that should be repaired,
         * if not defined all files of a download are going to be repaird
         * @return true if a repair started, false if it was not nescessary
         */
        bool repair(const KUrl &file = KUrl()) {return m_transfer->repair(file);}

        /**
         * @return the transfer's group handler
         */
        TransferGroupHandler * group() const {return m_transfer->group()->handler();}

        /**
         * @return the source url
         */
        const KUrl & source() const {return m_transfer->source();}

        /**
         * @return the dest url
         */
        const KUrl & dest() const {return m_transfer->dest();}

        /**
         * @returns all files of this transfer
         */
        QList<KUrl> files() const {return m_transfer->files();}

        /**
         * @returns the directory the Transfer will be stored to
         */
        KUrl directory() const {return m_transfer->directory();}

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        bool setDirectory(const KUrl &newDirectory) {return m_transfer->setDirectory(newDirectory);}

        /**
         * The mirrors that are available
         * bool if it is used, int how many paralell connections are allowed
         * to the mirror
         * @param file the file for which the availableMirrors should be get
         */
        QHash<KUrl, QPair<bool, int> > availableMirrors(const KUrl &file) const {return m_transfer->availableMirrors(file);}

        /**
         * Set the mirrors, int the number of paralell connections to the mirror
         * bool if the mirror should be used
         * @param file the file for which the availableMirrors should be set
         */
        void setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors) {m_transfer->setAvailableMirrors(file, mirrors);}

        /**
         * @return the total size of the transfer in bytes
         */
        KIO::filesize_t totalSize() const;

        /**
         * @return the downloaded size of the transfer in bytes
         */
        KIO::filesize_t downloadedSize() const;

        /**
         * @return the uploaded size of the transfer in bytes
         */
        KIO::filesize_t uploadedSize() const;

        /**
         * @return the progress percentage of the transfer
         */
        int percent() const;

        /**
         * @return the download speed of the transfer in bytes/sec
         */
        int downloadSpeed() const;

        /**
         * @return the average download speed of the transfer in bytes/sec
         */
        int averageDownloadSped() const;

        /**
         * @return the upload speed of the transfer in bytes/sec
         */
        int uploadSpeed() const;

        /**
         * Set an UploadLimit for the transfer
         * @note this UploadLimit is not visible in the GUI
         * @param ulLimit upload Limit
         */
        void setUploadLimit(int ulLimit, Transfer::SpeedLimit limit) {m_transfer->setUploadLimit(ulLimit, limit);}

        /**
         * Set a DownloadLimit for the transfer
         * @note this DownloadLimit is not visible in the GUI
         * @param dlLimit download Limit
         */
        void setDownloadLimit(int dlLimit, Transfer::SpeedLimit limit) {m_transfer->setDownloadLimit(dlLimit, limit);}

        /**
         * @return the upload Limit of the transfer in KiB
         */
        int uploadLimit(Transfer::SpeedLimit limit) const {return m_transfer->uploadLimit(limit);}

        /**
         * @return the download Limit of the transfer in KiB
         */
        int downloadLimit(Transfer::SpeedLimit limit) const {return m_transfer->downloadLimit(limit);}

        /**
         * Set the maximum share-ratio
         * @param ratio the new maximum share-ratio
         */
        void setMaximumShareRatio(double ratio) {m_transfer->setMaximumShareRatio(ratio);}

        /**
         * @return the maximum share-ratio
         */
        double maximumShareRatio() {return m_transfer->maximumShareRatio();}

        /**
         * Recalculate the share ratio
         */
        void checkShareRatio() {m_transfer->checkShareRatio();}

        /**
         * @return a string describing the current transfer status
         */
        QString statusText() const {return m_transfer->statusText();}

        /**
         * @return a pixmap associated with the current transfer status
         */
        QPixmap statusPixmap() const {return m_transfer->statusPixmap();}

        /**
         * @returns the data associated to this Transfer item. This is
         * necessary to make the interview model/view work
         */
        QVariant data(int column);

        /**
         * @returns the number of columns associated to the transfer's data
         */
        int columnCount() const     {return 6;}

        /**
         * Selects the current transfer. Selecting transfers means that all
         * the actions executed from the gui will apply also to the current
         * transfer.
         *
         * @param select if true the current transfer is selected
         *               if false the current transfer is deselected
         */
        void setSelected( bool select );

        /**
         * @returns true if the current transfer is selected
         * @returns false otherwise
         */
        bool isSelected() const;

        /**
         * Returns the changes flags
         */
        ChangesFlags changesFlags() const;

        /**
         * @returns a list of a actions, which are associated with this TransferHandler
         */
        QList<QAction*> contextActions();

        /**
         * @returns a list of the transfer's factory's actions
         */
        QList<QAction*> factoryActions();
        
        /**
         * @returns the object path that will be shown in the DBUS interface
         */
        QString dBusObjectPath()       {return m_dBusObjectPath;}
        
        /**
         * @returns the kJobAdapter object
         */
        KGetKJobAdapter * kJobAdapter() {return m_kjobAdapter;}

        /**
         * @returns a pointer to the FileModel containing all files of this download
         */
        virtual FileModel * fileModel() {return m_transfer->fileModel();}

        /**
         * @param file for which to get the verifier
         * @return Verifier that allows you to add checksums manually verify a file etc.
         */
        virtual Verifier * verifier(const KUrl &file) {return m_transfer->verifier(file);}

        /**
         * @param file for which to get the signature
         * @return Signature that allows you to add signatures and verify them
         */
        virtual Signature * signature(const KUrl &file) {return m_transfer->signature(file);}

#ifdef HAVE_NEPOMUK
        /**
         * Sets the NepomukHandler for the transfer
         * @param handler the new NepomukHandler
         */
        void setNepomukHandler(NepomukHandler *handler) {m_transfer->setNepomukHandler(handler);}

        /**
         * @returns the NepomukHandler of the transfer
         */
        NepomukHandler * nepomukHandler() {return m_transfer->nepomukHandler();}
#endif
    public slots:
        /**
         * These are all Job-related functions
         */
        virtual void start();
        virtual void stop();

    signals:
        /**
         * Emitted when the capabilities of the Transfer change
         */
        void capabilitiesChanged();
        void transferChangedEvent(TransferHandler * transfer, TransferHandler::ChangesFlags flags);

    private:
        /**
         * This functions gets called just before the handler is deleted
         */
        void destroy();
        
        /**
         * Sets a change flag in the ChangesFlags variable.
         *
         * @param change The TransferChange flag to be set
         * @param notifyModel if false the handler will not post an event to the model,
         * if true the handler will post an event to the model
         */
        void setTransferChange(ChangesFlags change, bool notifyModel = false);

        /**
         * Resets the changes flags
         */
        void resetChangesFlags();

        Transfer * m_transfer;
        
        KGetKJobAdapter * m_kjobAdapter;
        
        QString m_dBusObjectPath;

        ChangesFlags m_changesFlags;
};

#endif
