/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

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
#include <QTime>

#include <kurl.h>
#include <kio/global.h>

class QDomElement;

class Signature;
class TransferHandler;
class TransferFactory;
class TransferGroup;
class Scheduler;
class TransferTreeModel;
class NepomukHandler;
class FileModel;
class Verifier;

class KGET_EXPORT Transfer : public Job
{
    Q_OBJECT
    friend class TransferHandler;
    friend class TransferTreeModel;
    public:

        /**
         * Here we define the flags that should be shared by all the transfers.
         * A transfer should also be able to define additional flags, in the future.
         */
        enum TransferChange
        {
            Tc_None           = 0x00000000,
            // These flags respect the Model columns order NOTE: The model only checks the last 8 bits, so all values which need to be updated by the model should look like: 0x000000xx
            Tc_FileName       = 0x00000001,
            Tc_Status         = 0x00000002,
            Tc_TotalSize      = 0x00000004,
            Tc_Percent        = 0x00000008,
            Tc_DownloadSpeed  = 0x00000010,
            Tc_RemainingTime  = 0x00000020,
            // Misc
            Tc_UploadSpeed    = 0x00000100,
            Tc_UploadLimit    = 0x00000200,
            Tc_DownloadLimit  = 0x00000400,
            Tc_CanResume      = 0x00000800,
            Tc_DownloadedSize = 0x00001000,
            Tc_UploadedSize   = 0x00002000,
            Tc_Log            = 0x00004000,
            Tc_Group          = 0x00008000,
            Tc_Selection      = 0x00010000
        };

        enum LogLevel
        {
            info,
            warning,
            error
        };

        enum SpeedLimit
        {
            VisibleSpeedLimit   = 0x01,
            InvisibleSpeedLimit = 0x02
        };
        typedef int ChangesFlags;

        Transfer(TransferGroup * parent, TransferFactory * factory,
                 Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                 const QDomElement * e = 0);

        virtual ~Transfer();

        /**
         * This functions gets called whenever a Transfer gets created. As opposed
         * to init(), this isn't a virtual function and is not meant to be used in
         * transfer plugins
         */
        void create();
        
        /**
         * This functions gets called whenever a Transfer is going to be deleted. As opposed
         * to deinit(), this isn't a virtual function and is not meant to be used in
         * transfer plugins
         */
        void destroy();
        
        /**
         * This function is called after the creation of a Transfer
         * In transfer plugins you can put here whatever needs to be initialized
         * @note this function creates a NepomukHandler
         */
        virtual void init();

        /**
         * This function is called before the deletion of a Transfer
         * In transfer plugins you can put here whatever needs to be deinitialized
         */
        virtual void deinit() {}        
        
        /**
         * Tries to repair file
         * @param file the file of a download that should be repaired,
         * if not defined all files of a download are going to be repaird
         * @return true if a repair started, false if it was not nescessary
         */
        virtual bool repair(const KUrl &file = KUrl()) {Q_UNUSED(file) return false;}

        const KUrl & source() const            {return m_source;}
        const KUrl & dest() const              {return m_dest;}

        /**
         * @returns the directory the Transfer will be stored to
         */
        virtual KUrl directory() const {return m_dest.upUrl();}

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        virtual bool setDirectory(const KUrl &newDirectory);

        //Transfer status
        KIO::filesize_t totalSize() const      {return m_totalSize;}
        KIO::filesize_t downloadedSize() const {return m_downloadedSize;}
        KIO::filesize_t uploadedSize() const   {return m_uploadedSize;}
        QString statusText() const             {return m_statusText;}
        QPixmap statusPixmap() const           {return m_statusPixmap;}

        static QString statusText(Job::Status status);
        static QPixmap statusPixmap(Job::Status status);

        int percent() const                    {return m_percent;}
        int downloadSpeed() const              {return m_downloadSpeed;}
        int averageDownloadSpeed() const;
        int uploadSpeed() const                {return m_uploadSpeed;}
        virtual int remainingTime() const      {return KIO::calculateRemainingSeconds(totalSize(), downloadedSize(), downloadSpeed());}
        virtual int elapsedTime() const;

        virtual bool supportsSpeedLimits() const {return false;}

        /**
         * The mirrors that are available
         * bool if it is used, int how many paralell connections are allowed
         * to the mirror
         * @param file the file for which the availableMirrors should be get
         */
        virtual QHash<KUrl, QPair<bool, int> > availableMirrors(const KUrl &file) const;

        /**
         * Set the mirrors, int the number of paralell connections to the mirror
         * bool if the mirror should be used
         * @param file the file for which the availableMirrors should be set
         */
        virtual void setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors) {Q_UNUSED(file) Q_UNUSED(mirrors)}

        /**
         * Set the Transfer's UploadLimit
         * @note this is not displayed in any GUI, use setVisibleUploadLimit(int) instead
         * @param visibleUlLimit upload Limit
         */
        void setUploadLimit(int ulLimit, SpeedLimit limit);

        /**
         * Set the Transfer's UploadLimit, which are displayed in the GUI
         * @note this is not displayed in any GUI, use setVisibleDownloadLimit(int) instead
         * @param visibleUlLimit upload Limit
         */
        void setDownloadLimit(int dlLimit, SpeedLimit limit);

        /**
         * @return the UploadLimit, which is invisible in the GUI
         */
        int uploadLimit(SpeedLimit limit) const;

        /**
         * @return the DownloadLimit, which is invisible in the GUI
         */
        int downloadLimit(SpeedLimit limit) const;

        /**
         * Set the maximum share-ratio
         * @param ratio the new maximum share-ratio
         */
        void setMaximumShareRatio(double ratio);

        /**
         * @return the maximum share-ratio
         */
        double maximumShareRatio() {return m_ratio;}

        /**
         * Recalculate the share ratio
         */
        void checkShareRatio();

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
         * @returns a pointer to the FileModel containing all files of this download
         */
        virtual FileModel * fileModel() {return 0;}

        /**
         * @param file for which to get the verifier
         * @return Verifier that allows you to add checksums manually verify a file etc.
         */
        virtual Verifier * verifier(const KUrl &file) {Q_UNUSED(file) return 0;}

        /**
         * @param file for which to get the signature
         * @return Signature that allows you to add signatures and verify them
         */
        virtual Signature * signature(const KUrl &file) {Q_UNUSED(file) return 0;}

#ifdef HAVE_NEPOMUK
        /**
         * Sets the NepomukHandler for the transfer
         * @param handler the new NepomukHandler
         */
        void setNepomukHandler(NepomukHandler *handler);

        /**
         * @returns a pointer to the NepomukHandler of this transfer
         */
        NepomukHandler * nepomukHandler() {return m_nepomukHandler;}
#endif

        /**
         * Saves this transfer to the given QDomNode
         *
         * @param element The pointer to the QDomNode where the transfer will be saved
         */
        virtual void save(const QDomElement &element);

        /**
        * Loads the transfer's info from the QDomElement
        *
        * @param element The pointer to the QDomNode where info will be loaded from
        */
        virtual void load(const QDomElement *element);

    protected:
        /**
         * Sets the Job status to jobStatus, the status text to text and
         * the status pixmap to pix.
         */
        void setStatus(Job::Status jobStatus, const QString &text = QString(), const QPixmap &pix = QPixmap());

        /**
         * Makes the TransferHandler associated with this transfer know that
         * a change in this transfer has occurred.
         *
         * @param change: the TransferChange flags to be set
         */
        virtual void setTransferChange(ChangesFlags change, bool postEvent=false);

        /**
         * Function used to set the SpeedLimits to the transfer
         */
        virtual void setSpeedLimits(int uploadLimit, int downloadLimit) {Q_UNUSED(uploadLimit) Q_UNUSED(downloadLimit) }

        // --- Transfer information ---
        KUrl m_source;
        KUrl m_dest;

        QStringList   m_log;
        KIO::filesize_t m_totalSize;
        KIO::filesize_t m_downloadedSize;
        KIO::filesize_t m_uploadedSize;
        int           m_percent;
        int           m_downloadSpeed;
        int           m_uploadSpeed;

        int           m_uploadLimit;
        int           m_downloadLimit;

        bool m_isSelected;

    private:
        int m_visibleUploadLimit;
        int m_visibleDownloadLimit;
        int m_runningSeconds;
        double m_ratio;

        static const QStringList STATUSTEXTS;
        static const QStringList STATUSICONS;
        QString m_statusText;
        QPixmap m_statusPixmap;
        QTime m_runningTime;

        TransferHandler * m_handler;
        TransferFactory * m_factory;
#ifdef HAVE_NEPOMUK
        NepomukHandler * m_nepomukHandler;
#endif
};

#endif
