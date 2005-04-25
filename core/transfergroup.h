/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _GROUP_H
#define _GROUP_H

#include "jobqueue.h"

class QDomElement;

class Transfer;
class TransferGroupHandler;

/**
 * class TransferGroup:
 *
 * This class abstracts the concept of transfer group by means of which
 * the user can sort his transfers into categories.
 * By definition, we want each TransferGroup (transfer group) to be a JobQueue.
 * Moreover this class calculates informations such as:
 * - the size obtained by the sum of all the transfer's size
 * - the size obtained by the sum of all the transfer's processed size
 * - the global progress percentage within the group
 * - the global speed within the group
 */
class TransferGroup : public JobQueue
{
    public:
        enum GroupChange
        {
            Gc_None          = 0x00000000,
            Gc_Status        = 0x00000001,
            Gc_TotalSize     = 0x00000002,
            Gc_ProcessedSize = 0x00000004,
            Gc_Percent       = 0x00000008,
            Gc_Speed         = 0x00000010
        };

        typedef int ChangesFlags;

        TransferGroup(Scheduler * scheduler, const QString & name);
        TransferGroup(Scheduler * scheduler, const QDomElement & e);

        virtual ~TransferGroup();

        /**
         * Appends a new transfer to the list of the transfers
         *
         * @param transfer the transfer to append
         */
        void append(Transfer * transfer);

        /**
         * Prepends a new transfer to the list of the transfers
         *
         * @param transfer the transfer to prepend
         */
        void prepend(Transfer * transfer);

        /**
         * Removes the given transfer from the list of the transfers
         *
         * @param transfer the transfer to remove
         */
        void remove(Transfer * transfer);

        /**
         * Moves a transfer in the list
         *
         * @param transfer The transfer to move
         * @param position The new position of the transfer.
         */
        void move(Transfer * transfer, int position);

        /**
         * @return the number of jobs in the queue
         */
        int size() const;

        /**
         * Finds the first transfer with source src
         *
         * @param src the url of the source location
         *
         * @return the transfer pointer if the transfer has been found. Otherwise
         * it returns 0
         */
        Transfer * findTransfer(KURL src);

        /**
         * @return the group name
         */
        const QString & name()    {return m_name;}

        /**
         * @return the sum of the sizes of the transfers belonging to 
         * this group
         */
        int totalSize() const     {return m_totalSize;}

        /**
         * @return the sum of the processed sizes of the transfers
         * belonging to this group
         */
        int processedSize() const {return m_processedSize;}

        /**
         * @return the progress percentage
         */
        int percent() const       {return m_percent;}

        /**
         * @return the sum of the download speeds of the running transfers 
         * belonging this group
         */
        int speed() const         {return m_speed;}

        /**
         * @return the handler associated with this group
         */
        TransferGroupHandler * handler();

        /**
         * Saves this group object to the given QDomNode 
         *
         * @param n The QDomNode where the group will be saved
         */
        void save(QDomElement e);

        /**
         * Adds all the groups in the given QDomNode * to the group
         *
         * @param n The QDomNode where the group will look for the transfers to add
         */
        void load(const QDomElement & e);

    private:
        TransferGroupHandler * m_handler;

        //TransferGroup info
        QString m_name;
        int m_totalSize;
        int m_processedSize;
        int m_percent;
        int m_speed;
};

#endif
