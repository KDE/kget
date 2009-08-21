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
#include "kget_export.h"
#include "observer.h"

class QAction;
class KPassivePopup;

class TransferObserver;
class GenericTransferGroupObserver;

/**
 * Class TransferHandler:
 *
 * --- Overview ---
 * This class is the rapresentation of a Transfer object from the views'
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
 *
 * --- Notifies about the transfer changes ---
 * When a view is interested in receiving notifies about the specific transfer
 * rapresented by this TransferHandler object, it should add itself to the list
 * of observers calling the addObserver(TransferObserver *) function. On the 
 * contrary call the delObserver(TransferObserver *) function to remove it.
 *
 * --- Interrogation about what has changed in the transfer ---
 * When a TransferObserver receives a notify about a change in the Transfer, it
 * can ask to the TransferHandler for the ChangesFlags.
 */

class KGET_EXPORT TransferHandler : public QObject, public Handler
{
    Q_OBJECT
    
    friend class KGet;
    friend class TransferTreeModel;
    friend class Transfer;
    friend class TransferFactory;
    friend class TransferGroupHandler;

    public:

        typedef Transfer::ChangesFlags ChangesFlags;

        TransferHandler(Transfer * transfer, Scheduler * scheduler);

        virtual ~TransferHandler();

        bool supportsSpeedLimits() {return m_transfer->supportsSpeedLimits();}

        /**
         * Adds an observer to this Transfer
         * Note that the observer with pointer == 0 is added by default, and
         * it's used by the TransferTreeModel class
         *
         * @param observer The new observer that should be added
         */
        void addObserver(TransferObserver * observer);

        /**
         * Removes an observer from this Transfer
         *
         * @param observer The observer that should be removed
         */
        void delObserver(TransferObserver * observer);

        /**
         * These are all Job-related functions
         */
        virtual void start();
        virtual void stop();
        void setDelay(int seconds);
        Job::Status status() const {return m_transfer->status();}
        Job::Status startStatus() const {return m_transfer->startStatus();}
        int elapsedTime() const;
        int remainingTime() const;
        bool isResumable() const;

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
         *
         * @param observer The observer that makes this request
         */
        ChangesFlags changesFlags(TransferObserver * observer) const;

        /**
         * Resets the changes flags for a given TransferObserver
         *
         * @param observer The observer that makes this request
         */
        void resetChangesFlags(TransferObserver * observer);

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

    signals:
        void transferChangedEvent(TransferHandler * transfer, TransferHandler::ChangesFlags flags);

    private:
        /**
         * Sets a change flag in the ChangesFlags variable.
         *
         * @param change The TransferChange flag to be set
         * @param postEvent if false the handler will not post an event to the observers,
         * if true the handler will post an event to the observers
         */
        void setTransferChange(ChangesFlags change, bool postEvent=false);

        /**
         * Posts a TransferChangedEvent to all the observers.
         */
        void postTransferChangedEvent();

        /**
         * Posts a deleteEvent to all the observers
         */
        void postDeleteEvent();

        Transfer * m_transfer;
        
        QString m_dBusObjectPath;

        QList<TransferObserver *> m_observers;
        QMap<TransferObserver *, ChangesFlags> m_changesFlags;
};


class GenericTransferObserver : public QObject, public TransferObserver
{
    Q_OBJECT
    public:
        GenericTransferObserver(GenericTransferGroupObserver *groupObserver);

        void transferChangedEvent(TransferHandler * transfer);

#ifdef HAVE_KWORKSPACE
    private slots:
        void slotShutdown();
#endif

    private:
        bool allTransfersFinished();
        KPassivePopup* popupMessage(const QString &title, const QString &message);

        void checkAndFinish();

#ifdef HAVE_KWORKSPACE
        void checkAndShutdown();
#endif
        void checkAndUpdateSystemTray();


        QString prevStatus;
        GenericTransferGroupObserver *m_groupObserver;
};

#endif
