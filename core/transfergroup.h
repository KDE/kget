/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>
   Coypright (C) 2010 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#ifndef GROUP_H
#define GROUP_H

#include <QRegExp>

#include <kio/netaccess.h>
#include <KIcon>
#include <KDebug>

#include "jobqueue.h"
#include "../kget_export.h"
#include "transfer.h"

#ifdef HAVE_NEPOMUK
#include <Nepomuk/Tag>
#endif

class QDomElement;

class TransferGroupHandler;
class TransferTreeModel;

/**
 * class TransferGroup:
 *
 * This class abstracts the concept of transfer group by means of which
 * the user can sort his transfers into categories.
 * By definition, we want each TransferGroup (transfer group) to be a JobQueue.
 * Moreover this class calculates information such as:
 * - the size obtained by the sum of all the transfer's size
 * - the size obtained by the sum of all the transfer's processed size
 * - the global progress percentage within the group
 * - the global speed within the group
 */
class KGET_EXPORT TransferGroup : public JobQueue
{
    Q_OBJECT
    public:
        enum GroupChange
        {
            Gc_None                = 0x00000000,
            // These flags respect the Model columns order
            Gc_GroupName           = 0x00000001,
            Gc_Status              = 0x00000002,
            Gc_TotalSize           = 0x00000004,
            Gc_Percent             = 0x00000008,
            Gc_UploadSpeed         = 0x00000010,
            Gc_DownloadSpeed       = 0x00000020,
            // Misc
            Gc_ProcessedSize       = 0x00010000
        };

        typedef int ChangesFlags;

        TransferGroup(TransferTreeModel * model, Scheduler * parent, const QString & name=QString());

        virtual ~TransferGroup();

        /**
         * This function is reimplemented by JobQueue::setStatus
         *
         * @param queueStatus the new JobQueue status
         */
        void setStatus(Status queueStatus);

        /**
         * Appends a new transfer to the list of the transfers
         *
         * @param transfer the transfer to append
         */
        void append(Transfer * transfer);

        /**
         * Appends new transfers to the list of the transfers
         *
         * @param transfers to append
         */
        void append(const QList<Transfer*> &transfer);

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
         * Removes the given transfers from the list of the transfers
         *
         * @param transfers the transfers to remove
         */
        void remove(const QList<Transfer*> &transfers);

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
        Transfer * findTransfer(const KUrl &src);

        /**
         * Finds the first transfer with destination dest
         *
         * @param dest the url of the destination location
         *
         * @return the transfer pointer if the transfer has been found, else return 0
         */
         Transfer *findTransferByDestination(const KUrl &dest);

        /**
         * @returns the Job in the queue at the given index i
         */
        Transfer * operator[] (int i) const {return (Transfer *)((* (JobQueue *)this)[i]);}

        /**Set the group name
         * @param name group name
         */
        void  setName(const QString &name)    {m_name=name;}

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
         * @return the sum of the downloaded sizes of the transfers
         * belonging to this group
         */
        int downloadedSize() const {return m_downloadedSize;}

        /**
         * @return the sum of the uploaded sizes of the transfers
         * belonging to this group
         */
        int uploadedSize() const {return m_uploadedSize;}

        /**
         * @return the progress percentage
         */
        int percent() const       {return m_percent;}

        /**
         * @return the sum of the download speeds of the running transfers 
         * belonging this group
         */
        int downloadSpeed();

        /**
         * @return the sum of the download speeds of the running transfers 
         * belonging this group
         */
        int uploadSpeed();

        /**
         * Set a default Folder for the group
         * @param folder the new default folder
         */
        void setDefaultFolder(QString folder) {m_defaultFolder = folder;}

        /**
         * @return the groups default folder
         */
        QString defaultFolder() {return m_defaultFolder;}

        /**
         * Sets the regular expression of the group
         * @param regexp the regular expression
         */
        void setRegExp(const QRegExp &regExp) {m_regExp = regExp;}

        /**
         * @returns the regular expression of the group
         */
        QRegExp regExp() {return m_regExp;}

#ifdef HAVE_NEPOMUK
        /**
         * Sets the Nepomuk tags of the group
         * @param tags the Nepomuk tags
         */
        void setTags(const QList<Nepomuk::Tag> &tags) {m_tags = tags;}

        /**
         * @returns the Nepomuk tags of the group
         */
        QList<Nepomuk::Tag> tags() const {return m_tags;}
#endif //HAVE_NEPOMUK

        /**
         * @return true if the group supports SpeedLimits
         */
        bool supportsSpeedLimits();

        /**
         * Set a Download-Limit for the group
         * @param limit the new download-limit
         * @note if limit is 0, no download-limit is set
         */
        void setDownloadLimit(int dlLimit, Transfer::SpeedLimit limit);

        /**
         * @return the group's Download-Limit
         */
        int downloadLimit(Transfer::SpeedLimit limit) const;

        /**
         * Set a Upload-Limit for the group
         * @param limit the new upload-limit
         * @note if limit is 0, no upload-limit is set
         */
        void setUploadLimit(int ulLimit, Transfer::SpeedLimit limit);

        /**
         * @return the group's Upload-Limit
         */
        int uploadLimit(Transfer::SpeedLimit limit) const;

        /**
         * Set the group's icon
         * @param name the icon's name
         */
        void setIconName(const QString &name) {m_iconName = name;}

        /**
         * @returns the group's icon's name
         */
        QString iconName() const {return m_iconName;}

        /**
         * @return the group's icon
         */
        QPixmap pixmap() {return KIcon(m_iconName).pixmap(32);}

        /**
         * @return the handler associated with this group
         */
        TransferGroupHandler * handler() const {return m_handler;}

        /**
         * @returns the TransferTreeModel that owns this group
         */
        TransferTreeModel * model()     {return m_model;}

        /**
         * Calculates the whole SpeedLimits
         */
        void calculateSpeedLimits();

        /**
         * Calculates the DownloadLimits
         */
        void calculateDownloadLimit();

        /**
         * Calculates the DownloadLimits
         */
        void calculateUploadLimit();

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
        int m_downloadedSize;
        int m_uploadedSize;
        int m_percent;
        int m_downloadSpeed;
        int m_uploadSpeed;
        int m_downloadLimit;
        int m_uploadLimit;
        int m_visibleDownloadLimit;
        int m_visibleUploadLimit;
        QString m_iconName;
        QString m_defaultFolder;
        QRegExp m_regExp;
#ifdef HAVE_NEPOMUK
        QList<Nepomuk::Tag> m_tags;
#endif //HAVE_NEPOMUK
};

#endif
