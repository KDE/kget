/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFER_H
#define TRANSFER_H

#include "job.h"
#include "kget_export.h"

#include <QPixmap>

#include <kurl.h>
#include <kio/global.h>

class QDomElement;

class TransferHandler;
class TransferFactory;
class TransferGroup;
class Scheduler;
class TransferTreeModel;

class KGET_EXPORT Transfer : public Job
{
    friend class TransferHandler;
    friend class TransferTreeModel;

    public:

        /**
         * Here we define the flags that should be shared by all the transfers.
         * A transfer should also be able to define additional flags, in the future.
         */
        enum TransferChange
        {
            Tc_None          = 0x00000000,
            // These flags respect the Model columns order
            Tc_FileName      = 0x00000001,
            Tc_Status        = 0x00000002,
            Tc_TotalSize     = 0x00000004,
            Tc_Percent       = 0x00000008,
            Tc_Speed         = 0x00000010,
            // Misc
            Tc_UploadLimit   = 0x00008000,
            Tc_DownloadLimit = 0x00009000,
            Tc_CanResume     = 0x00010000,
            Tc_ProcessedSize = 0x00020000,
            Tc_Log           = 0x00040000,
            Tc_Group         = 0x00080000,
            Tc_Selection     = 0x00100000
        };

        enum LogLevel
        {
            info,
            warning,
            error
        };
        typedef int ChangesFlags;

        Transfer(TransferGroup * parent, TransferFactory * factory,
                 Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                 const QDomElement * e = 0);

        virtual ~Transfer();

        const KUrl & source() const         {return m_source;}
        const KUrl & dest() const           {return m_dest;}

        //Transfer status
        KIO::filesize_t totalSize() const     {return m_totalSize;}
        KIO::filesize_t processedSize() const {return m_processedSize;}
        QString statusText() const          {return m_statusText;}
        QPixmap statusPixmap() const        {return m_statusPixmap;}

        int percent() const                 {return m_percent;}
        int speed() const                   {return m_speed;}

        virtual bool supportsSpeedLimits() const {return false;}

        /**
         * Set the Transfer's UploadLimit
         * @note this is not displayed in any GUI, use setVisibleUploadLimit(int) instead
         * @param visibleUlLimit upload Limit
         */
        virtual void setUploadLimit(int ulLimit) {Q_UNUSED(ulLimit);}

        /**
         * Set the Transfer's UploadLimit, which are displayed in the GUI
         * @note this is not displayed in any GUI, use setVisibleDownloadLimit(int) instead
         * @param visibleUlLimit upload Limit
         */
        virtual void setDownloadLimit(int dlLimit) {Q_UNUSED(dlLimit);}

        /**
         * @return the UploadLimit, which is invisible in the GUI
         */
        int uploadLimit() const {return m_ulLimit;}

        /**
         * @return the DownloadLimit, which is invisible in the GUI
         */
        int downloadLimit() const {return m_dlLimit;}

        /**
         * Set the Transfer's UploadLimit, which are displayed in the GUI
         * @note use this, when a user changes the UploadLimit manually
         * @param visibleUlLimit upload Limit
         */
        void setVisibleUploadLimit(int visibleUlLimit);

        /**
         * Set the Transfer's UploadLimit, which are displayed in the GUI
         * @note use this, when a user changes the UploadLimit manually
         * @param visibleUlLimit upload Limit
         */
        void setVisibleDownloadLimit(int visibleDlLimit);

        /**
         * @return the visible UploadLimit
         */
        int visibleUploadLimit() const {return m_visibleUlLimit;}

        /**
         * @return the visible DownloadLimit
         */
        int visibleDownloadLimit() const {return m_visibleDlLimit;}

        // --- Job virtual functions ---
        virtual void setDelay(int seconds);
        virtual void delayTimerEvent();


        bool isSelected() const             {return m_isSelected;}

        /**
         * Transfer history
         */
        const QStringList log() const;

        /**
         * Set Transfer history
         */
         void setLog(const QString& message, LogLevel level = info);

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
         * @returns the TransferTreeModel that owns this group
         */
        TransferTreeModel * model();

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
        virtual void save(const QDomElement &element);

    protected:
        //Function used to load and save the transfer's info from xml
        virtual void load(const QDomElement &e);

        /**
         * Sets the Job status to jobStatus, the status text to text and
         * the status pixmap to pix.
         */
        void setStatus(Job::Status jobStatus, const QString &text, const QPixmap &pix);

        /**
         * Makes the TransferHandler associated with this transfer know that
         * a change in this transfer has occurred.
         *
         * @param change: the TransferChange flags to be set
         */
        virtual void setTransferChange(ChangesFlags change, bool postEvent=false);

        // --- Transfer information ---
        KUrl m_source;
        KUrl m_dest;

        QStringList   m_log;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_processedSize;
        int           m_percent;
        int           m_speed;

        int           m_dlLimit;
        int	      m_ulLimit;

        bool m_isSelected;

    private:
        int m_visibleUlLimit;
        int m_visibleDlLimit;

        QString m_statusText;
        QPixmap m_statusPixmap;

        TransferHandler * m_handler;
        TransferFactory * m_factory;
};

#endif
