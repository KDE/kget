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

class Transfer;

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
class TransferGroup : public JobQueue, public QValueList<Transfer *>
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

        TransferGroup(const QString & name);

        //TransferGroup info retrieval
        const QString & name()    {return m_name;}

        int totalSize() const     {return m_totalSize;}
        int processedSize() const {return m_processedSize;}
        int percent() const       {return m_percent;}
        int speed() const         {return m_speed;}

        /**
         * Finds the first transfer with source src
         *
         * @param src the url of the source location
         *
         * @return the transfer pointer if the transfer has been found. Otherwise
         * it returns 0
         */
        Transfer * findTransfer(KURL src);

    private:
        //TransferGroup info
        QString m_name;
        int m_totalSize;
        int m_processedSize;
        int m_percent;
        int m_speed;
};

#endif
