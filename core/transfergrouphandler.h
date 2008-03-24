/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERGROUPHANDLER_H
#define TRANSFERGROUPHANDLER_H

#include <QVariant>

#include "transfergroup.h"
#include "kget_export.h"

class QAction;
class KMenu;

class QObjectInterface;
class TransferGroupObserver;
class TransferHandler;
class Scheduler;

class KGET_EXPORT TransferGroupHandler
{
    friend class TransferGroup;
    friend class TransferTreeModel;

    public:

        typedef TransferGroup::ChangesFlags ChangesFlags;

        TransferGroupHandler(TransferGroup * group, Scheduler * scheduler);

        ~TransferGroupHandler();

        /**
         * Adds an observer to this TransferGroup
         * Note that the observer with pointer == 0 is added by default, and
         * it's used by the TransferTreeModel class
         *
         * @param observer The new observer that should be added
         */
        void addObserver(TransferGroupObserver * observer);

        /**
         * Removes an observer from this TransferGroup
         *
         * @param observer The observer that should be removed
         */
        void delObserver(TransferGroupObserver * observer);

        /**
         * These are all JobQueue-related functions
         */
        void start();
        void stop();
        JobQueue::Status status() const {return m_group->status();}

        /**
         * Moves a list of transfers belonging to this group to a new position,
         * after the transfer named "after". All the transfers must belong to
         * this group
         *
         * @param transfers The transfers to be be moved
         * @param after The transfer after which the given transfers should be moved
         */
        void move(QList<TransferHandler *> transfers, TransferHandler * after);

        /**
         * Sets the maximum number of jobs belonging to this queue that 
         * should executed simultaneously by the scheduler
         *
         * @param n The maximum number of jobs
         */
        void setMaxSimultaneousJobs(int n);

        /**
         * @returns the Job in the queue at the given index i
         */
        TransferHandler * operator[] (int i);

        /**
         * @returns the number of Transfers owned by this object
         */
        int size()     {return m_group->size();}

        /**Set the group name
         * @param name group name
         */
        void  setName(const QString &name);

        /**
         * @return the group name
         */
        const QString & name()    {return m_group->name();}

        /**
         * @return the sum of the sizes of the transfers belonging to 
         * this group
         */
        int totalSize() const     {return m_group->totalSize();}

        /**
         * @return the sum of the processed sizes of the transfers
         * belonging to this group
         */
        int processedSize() const {return m_group->processedSize();}

        /**
         * @return the progress percentage
         */
        int percent() const       {return m_group->percent();}

        /**
         * @return the sum of the download speeds of the running transfers 
         * belonging this group
         */
        int speed() const         {return m_group->speed();}

        /**
         * Set a default Folder for the group
         * @param folder the new default folder
         */
        void setDefaultFolder(QString folder) {m_group->setDefaultFolder(folder);}

        /**
         * @return the groups default folder
         */
        QString defaultFolder() {return m_group->defaultFolder();}

        /**
         * Set a Download-Limit for the group
         * @param limit the new download-limit
         * @note this will be displayed in the GUI
         */
         void setVisibleDownloadLimit(int limit) {m_group->setVisibleDownloadLimit(limit);}

        /**
         * @return the group's Download-Limit
         */
         int visibleDownloadLimit() {return m_group->visibleDownloadLimit();}

        /**
         * Set a Upload-Limit for the group
         * @param limit the new upload-limit
         * @note this will be displayed in the GUI
         */
         void setVisibleUploadLimit(int limit) {m_group->setVisibleUploadLimit(limit);}

        /**
         * @return the group's Upload-Limit
         */
         int visibleUploadLimit() {return m_group->visibleUploadLimit();}

        /**
         * Set a Download-Limit for the group
         * @param limit the new download-limit
         * @note if limit is 0, no download-limit is set
         */
         void setDownloadLimit(int limit) {m_group->setDownloadLimit(limit);}

        /**
         * @return the group's Download-Limit
         */
         int downloadLimit() {return m_group->downloadLimit();}

        /**
         * Set a Upload-Limit for the group
         * @param limit the new upload-limit
         * @note this will not be displayed in the GUI
         */
         void setUploadLimit(int limit) {m_group->setUploadLimit(limit);}

        /**
         * @return the group's Upload-Limit
         */
         int uploadLimit() {return m_group->uploadLimit();}

        /**
         * Set the group's icon
         * @param name the icon's name
         */
        void setIconName(QString name) {m_group->setIconName(name);}

        /**
         * @return the group's icon
         */
        QPixmap pixmap() {return m_group->pixmap();}

        /**
         * @returns the data associated to this TransferGroup item. This is
         * necessary to make the interview model/view work
         */
        QVariant data(int column);

        /**
         * @returns the number of columns associated to the group's data
         */
        int columnCount() const     {return 6;}

        /**
         * Returns the changes flags
         *
         * @param observer The observer that makes this request
         */
        ChangesFlags changesFlags(TransferGroupObserver * observer);

        /**
         * Resets the changes flags for a given TransferObserver
         *
         * @param observer The observer that makes this request
         */
        void resetChangesFlags(TransferGroupObserver * observer);

        /**
         * @returns the index for the given transfer. If the transfer can't
         *          be found, it returns -1
         */
        int indexOf(TransferHandler * transfer);

        /**
         * @returns a list containing all the transfers belonging to this group.
         */
        const QList<TransferHandler *> transfers();

        /**
         * @returns a pointer to a QObjectInterface object which is a QObject
         * by means of which you can connect signals and slots for this 
         * transfer group.
         */
        const QList<QAction *> & actions();

        /**
         * @returns a KMenu for this transfer group.
         */
        KMenu * popupMenu();

        /**
         * @returns a pointer to a QObjectInterface object which is a QObject
         * by means of which you can connect signals and slots for this 
         * transfer group.
         */
        QObjectInterface * qObject();

    private:
        /**
         * Sets a change flag in the ChangesFlags variable.
         *
         * @param change The TransferChange flag to be set
         * @param postEvent if false the handler will not post an event to the
         * observers, if true the handler will post an event to the observers
         */
        void setGroupChange(ChangesFlags change, bool postEvent=false);

        /**
         * Posts a TransferGroupChangedEvent to all the observers
         */
        void postGroupChangedEvent();

        /**
         * Posts an addedTransferEvent to all the observers
         *
         * @param transfer the transfer that has been added to the group
         * @param after the transfer after which it has been added
         */
        void postAddedTransferEvent(Transfer * transfer, Transfer * after);

        /**
         * Posts an removedTransferEvent to all the observers
         *
         * @param transfer the transfer that has been removed from the group
         */
        void postRemovedTransferEvent(Transfer * transfer);

        /**
         * Posts an movedTransferEvent to all the observers
         *
         * @param transfer the transfer that has been removed from the group
         * @param after the transfer after which the it has been moved
         */
        void postMovedTransferEvent(Transfer * transfer, Transfer * after);

        /**
         * Posts a deleteEvent to all the observers
         */
        void postDeleteEvent();

        /**
         * Creates all the QActions
         */
        void createActions();

        TransferGroup * m_group;
        Scheduler * m_scheduler;

        QObjectInterface * m_qobject;
        QList<QAction *> m_actions;

        QList<TransferGroupObserver *> m_observers;
        QMap<TransferGroupObserver *, ChangesFlags> m_changesFlags;
};


class QObjectInterface : public QObject
{
    Q_OBJECT
    public:
        QObjectInterface(TransferGroupHandler * handler);

    public slots:
        void slotStart();        //Starts the download process
        void slotStop();         //Stops the download process

    signals:

    private:
        TransferGroupHandler * m_handler;
};

#endif
