/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _TRANSFERGROUPHANDLER_H
#define _TRANSFERGROUPHANDLER_H

#include <qmap.h>

#include "transfergroup.h"

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
         * Appends a new transfer to the list of the transfers
         *
         * @param transfer the transfer to append
         */
        void append(TransferHandler * transfer);

        /**
         * Prepends a new transfer to the list of the transfers
         *
         * @param transfer the transfer to prepend
         */
        void prepend(TransferHandler * transfer);

        /**
         * Removes the given transfer from the list of the transfers
         *
         * @param transfer the transfer to remove
         */
        void remove(TransferHandler * transfer);

//        /**
//         * @return a const list with the transfers belonging to this group
//         */
//        const QValueList<Transfer *> jobs() const {return m_transfers;}

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
         */
        void postAddedTransferEvent(Transfer * transfer);

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
         * @param position the new position of the transfer
         */
        void postMovedTransferEvent(Transfer * transfer, int position);

        /**
         * Posts a deleteEvent to all the observers
         */
        void postDeleteEvent();

        TransferGroup * m_group;
        Scheduler * m_scheduler;

        QValueList<TransferGroupObserver *> m_observers;
        QMap<TransferGroupObserver *, ChangesFlags> m_changesFlags;

};


#endif
