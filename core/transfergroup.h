/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef GROUP_H
#define GROUP_H

#include <kio/netaccess.h>

#include "jobqueue.h"

class QDomElement;

class Transfer;
class TransferGroupHandler;
class TransferTreeModel;

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

        TransferGroup(TransferTreeModel * model, Scheduler * scheduler, const QString & name=QString());

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
         * inserts a transfer to the current group after the given transfer
         *
         * @param transfer The transfer to add in the current Group
         * @param after The transfer after which to add the transfer
         */
        void insert(Transfer * transfer, Transfer * after);

        /**
         * Removes the given transfer from the list of the transfers
         *
         * @param transfer the transfer to remove
         */
        void remove(Transfer * transfer);

        /**
         * Moves a transfer in the list
         *
         * @param transfer The transfer to move. Note that this transfer can
         *                  belong to other groups. In this situation this
         *                  transfer is deleted from the previous group and
         *                  moved inside this one.
         * @param after The transfer after which we have to move the given one
         */
        void move(Transfer * transfer, Transfer * after);

        /**
         * Finds the first transfer with source src
         *
         * @param src the url of the source location
         *
         * @return the transfer pointer if the transfer has been found. Otherwise
         * it returns 0
         */
        Transfer * findTransfer(KUrl src);

        /**
         * @returns the Job in the queue at the given index i
         */
        Transfer * operator[] (int i) const;

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
         * @returns the TransferTreeModel that owns this group
         */
        TransferTreeModel * model()     {return m_model;}

        /**
         * Notifies that the given transfer has changed
         *
         * @param transfer The transfer that has changed
         */
        void transferChangedEvent(Transfer * transfer);

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
        TransferTreeModel * m_model;
        TransferGroupHandler * m_handler;

        //TransferGroup info
        QString m_name;
        int m_totalSize;
        int m_processedSize;
        int m_percent;
        int m_speed;
};

#endif
