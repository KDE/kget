/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/


#ifndef _TRANSFER_H
#define _TRANSFER_H

#include <qpixmap.h>
#include <kurl.h>

#include "job.h"

class QStringList;
class QDomNode;

class TransferHandler;
class TransferGroup;
class Scheduler;

class Transfer : public Job
{
    public:

        /**
         * Here we define the flags that should be shared by all the transfers.
         * A transfer should also be able to define additional flags, in the future.
         */
        enum TransferChange
        {
            Tc_None          = 0x00000000,
            Tc_Status        = 0x00000001,
            Tc_CanResume     = 0x00000002,
            Tc_TotalSize     = 0x00000004,
            Tc_ProcessedSize = 0x00000008,
            Tc_Percent       = 0x00000010,
            Tc_Speed         = 0x00000020,
            Tc_Log           = 0x00000040,
            Tc_Group         = 0x00000080
        };

        typedef int ChangesFlags;

        Transfer(TransferGroup * parent, Scheduler * scheduler,
                 const KURL & src, const KURL & dest);
        Transfer(TransferGroup * parent, Scheduler * scheduler, QDomNode * n);

        virtual ~Transfer(){}

        const KURL & source() const;
        const KURL & dest() const;

        //Transfer status
        virtual unsigned long totalSize() const =0;
        virtual unsigned long processedSize() const =0;

        virtual int percent() const =0;
        virtual int speed() const =0;

        /**
         * Transfer history
         */
        const QStringList log() const;

        /**
         * Defines the order between transfers
         */
        bool operator<(const Transfer& t2) const;

        /**
         * The owner group
         */
        TransferGroup * group() {return m_group;}

    protected:
        //Function used to read the transfer's info from xml
        virtual void read(QDomNode * n);
        virtual void write(QDomNode * n);

        /**
         * Makes the TransferHandler associated with this transfer know that
         * a change in this transfer has occoured.
         *
         * change: the TransferChange flags to be set
         */
        virtual void setTransferChange(ChangesFlags change, bool postEvent=false);

        TransferGroup * m_group;
        TransferHandler * m_handler;

        // --- Transfer informations ---
        KURL m_source;
        KURL m_dest;

        QStringList * m_log;
        unsigned long m_totalSize;
        unsigned long m_processedSize;
        int           m_percent;
        int           m_speed;
        //TransferStatus m_transferStatus; ???

        QString m_statusText;
        QPixmap m_statusPixmap;
};


#endif
