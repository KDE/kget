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
class Scheduler;

class TransferGroupHandler : public TransferGroup
{
    friend class TransferGroup;

    public:

        TransferGroupHandler(TransferGroup * group, Scheduler * scheduler);

        ~TransferGroupHandler();

        /**
         * Adds an observer to this TransferGroup
         *
         * observer: the new observer that should be added
         */
        void addObserver(TransferGroupObserver * observer);

        /**
         * Removes an observer from this TransferGroup
         *
         * observer: the observer that should be removed
         */
        void delObserver(TransferGroupObserver * observer);

        /**
         * Returns the changes flags
         *
         * observer: the observer that makes this request
         */
        ChangesFlags changesFlags(TransferGroupObserver * observer);

        /**
         * Resets the changes flags for a given TransferObserver
         *
         * observer: the observer that makes this request
         */
        void resetChangesFlags(TransferGroupObserver * observer);

    private:
        /**
         * Sets a change flag in the ChangesFlags variable.
         *
         * change: the TransferChange flag to be set
         * postEvent: if false the handler will not post an event to the observers
         *            if true the handler will post an event to the observers
         */
        void setGroupChange(ChangesFlags change, bool postEvent=false);

        /**
         * Posts a TransferGroupChangedEvent to all the observers.
         */
        void postGroupChangedEvent();

        TransferGroup * m_group;
        Scheduler * m_scheduler;

        QValueList<TransferGroupObserver *> m_observers;
        QMap<TransferGroupObserver *, ChangesFlags> m_changesFlags;

};


#endif
