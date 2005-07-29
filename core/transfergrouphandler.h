/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERGROUPHANDLER_H
#define _TRANSFERGROUPHANDLER_H

#include <qobject.h>
#include <qmap.h>

#include "transfergroup.h"

class KAction;
class KPopupMenu;

class QObjectInterface;
class TransferGroupObserver;
class TransferHandler;
class Scheduler;

class TransferGroupHandler
{
    friend class TransferGroup;

    public:

        typedef TransferGroup::ChangesFlags ChangesFlags;

        TransferGroupHandler(TransferGroup * group, Scheduler * scheduler);

        ~TransferGroupHandler();

        /**
         * Adds an observer to this TransferGroup
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
         * Moves a transfer in the group. Note that the transfers in the
         * list can belong to different groups, while the transfer named
         * "after" must belong to this group.
         *
         * @param transfers The transfers to be be moved
         * @param after The transfer after which the given transfers should be moved
         */
        void move(QValueList<TransferHandler *> transfers, TransferHandler * after);

        /**
         * Sets the maximum number of jobs belonging to this queue that 
         * should executed simultaneously by the scheduler
         *
         * @param n The maximum number of jobs
         */
        void setMaxSimultaneousJobs(int n);

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
         * @returns a pointer to a QObjectInterface object which is a QObject
         * by means of which you can connect signals and slots for this 
         * transfer group.
         */
        const QValueList<KAction *> & actions();

        /**
         * @returns a KPopupMenu for this transfer group.
         */
        KPopupMenu * popupMenu();

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
         * Creates all the KActions
         */
        void createActions();

        TransferGroup * m_group;
        Scheduler * m_scheduler;

        QObjectInterface * m_qobject;
        QValueList<KAction *> m_actions;

        QValueList<TransferGroupObserver *> m_observers;
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
