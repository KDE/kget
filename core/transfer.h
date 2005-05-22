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
class QDomElement;

class TransferHandler;
class TransferFactory;
class TransferGroup;
class Scheduler;

class Transfer : public Job
{
    friend class TransferHandler;

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
            Tc_Group         = 0x00000080,
            Tc_Selection     = 0x00000100
        };

        typedef int ChangesFlags;

        Transfer(TransferGroup * parent, TransferFactory * factory,
                 Scheduler * scheduler, const KURL & src, const KURL & dest,
                 const QDomElement * e = 0);

        virtual ~Transfer();

        const KURL & source() const         {return m_source;}
        const KURL & dest() const           {return m_dest;}

        //Transfer status
        unsigned long totalSize() const     {return m_totalSize;}
        unsigned long processedSize() const {return m_processedSize;}

        int percent() const                 {return m_percent;}
        int speed() const                   {return m_speed;}

        bool isSelected() const             {return m_isSelected;}

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
        TransferGroup * group() const   {return (TransferGroup *) m_jobQueue;}

        /**
         * @return the associated TransferHandler
         */
        TransferHandler * handler();

        /**
         * @returns a pointer to the TransferFactory object
         */
        TransferFactory * factory() const   {return m_factory;}

        /**
         * Saves this transfer to the given QDomNode
         *
         * @param n The pointer to the QDomNode where the transfer will be saved
         * @return  The created QDomElement
         */
        virtual void save(QDomElement e);

    protected:
        //Function used to load and save the transfer's info from xml
        virtual void load(QDomElement e);

        /**
         * Makes the TransferHandler associated with this transfer know that
         * a change in this transfer has occoured.
         *
         * change: the TransferChange flags to be set
         */
        virtual void setTransferChange(ChangesFlags change, bool postEvent=false);

        // --- Transfer informations ---
        KURL m_source;
        KURL m_dest;

        QStringList   m_log;
        unsigned long m_totalSize;
        unsigned long m_processedSize;
        int           m_percent;
        int           m_speed;

        bool m_isSelected;

        QString m_statusText;
        QPixmap m_statusPixmap;

    private:
        TransferHandler * m_handler;
        TransferFactory * m_factory;
};


#endif
