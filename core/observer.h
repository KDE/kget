/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _OBSERVER_H
#define _OBSERVER_H

class TransferHandler;
class TransferGroupHandler;

/**
 * @brief ModelObserver class
 *
 * A view should reimplement this class in order to receive notification about 
 * changes happened to the model
 */
class ModelObserver
{
    public:
        /**
         * Notifies that a new "transfer group" has been added to the model
         *
         * @param group The "transfer group" that has just been added
         */
        virtual void addedTransferGroupEvent(TransferGroupHandler * group){}

        /**
         * Notifies that a new "transfer group" has been removed from the model
         *
         * @param group The "transfer group" that has just been removed
         */
        virtual void removedTransferGroupEvent(TransferGroupHandler * group){}

        // In the future we will have also notifications about new searches..
};

class TransferGroupObserver
{
    public:
        /**
         * Notifies that a transfer has changed (status, progress, etc...)
         *
         * @param group The group that has changed
         */
        virtual void groupChangedEvent(TransferGroupHandler * group){}

        /**
         * Notifies that a new transfer has been added to this group
         *
         * @param transfer The transfer that has just been added
         * @param after The transfer after which it has been added
         */
        virtual void addedTransferEvent(TransferHandler * transfer, TransferHandler * after){}

        /**
         * Notifies that a new transfer has been removed from this group.
         * Note that from the moment the view receives this signal on, the
         * pointer to the transferHandler becomes invalid.
         *
         * @param transfer The transfer that has just been removed
         */
        virtual void removedTransferEvent(TransferHandler * transfer){}

        /**
         * Notifies that a transfer previously belonging to this group
         * has been moved whithin the group.
         *
         * @param transfer The transfer that has just been moved
         * @param after The transfer after which it has been moved
         */
        virtual void movedTransferEvent(TransferHandler * transfer, TransferHandler * after){}

        /**
         * Notifies that this group has been deleted
         *
         * @param group This group's handler
         */
        virtual void deleteEvent(TransferGroupHandler * group){}
};

class TransferObserver
{
    public:
        /**
         * Notifies that a transfer has changed (status, progress, ...)
         * 
         * @param transfer The transfer that has changed
         */
        virtual void transferChangedEvent(TransferHandler * transfer){}

        /**
         * Notifies that this transfer has been deleted
         *
         * @param transfer This transfer's handler
         */
        virtual void deleteEvent(TransferHandler * transfer){}
};


// Here we should add also other observers (SearchObserver)

#endif
