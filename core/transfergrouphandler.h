/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef TRANSFERGROUPHANDLER_H
#define TRANSFERGROUPHANDLER_H

#include <QVariant>

#include "handler.h"
#include "transfergroup.h"
#include "../kget_export.h"
#include "kget.h"

class QAction;

class QObjectInterface;
class TransferGroupObserver;
class TransferHandler;
class KGetKJobAdapter;
class Scheduler;

class KGET_EXPORT TransferGroupHandler : public Handler
{
    Q_OBJECT
    friend class GenericObserver;
    friend class TransferGroup;
    friend class TransferTreeModel;
    friend class KGet;
    public:

        typedef TransferGroup::ChangesFlags ChangesFlags;

        TransferGroupHandler(Scheduler * scheduler, TransferGroup * parent);

        ~TransferGroupHandler();

        JobQueue::Status status() const {return m_group->status();}

        /**
         * Moves a list of transfers belonging to this group to a new position,
         * after the transfer named "after". All the transfers must belong to
         * this group
         *
         * @param transfers The transfers to be be moved
         * @param after The transfer after which the given transfers should be moved
         */
        void move(QList<TransferHandler *> transfers, TransferHandler * after);

        /**
         * Sets the maximum number of jobs belonging to this queue that 
         * should executed simultaneously by the scheduler
         *
         * @param n The maximum number of jobs
         */
        void setMaxSimultaneousJobs(int n);

        /**
         * @returns the Job in the queue at the given index i
         */
        TransferHandler * operator[] (int i);

        /**
         * @returns the number of Transfers owned by this object
         */
        int size()     {return m_group->size();}

        /**Set the group name
         * @param name group name
         */
        void  setName(const QString &name);

        /**
         * @return the group name
         */
        const QString & name()    {return m_group->name();}

        /**
         * @return the sum of the sizes of the transfers belonging to 
         * this group
         */
        int totalSize() const     {return m_group->totalSize();}

        /**
         * @return the sum of the downloaded sizes of the transfers
         * belonging to this group
         */
        int downloadedSize() const {return m_group->downloadedSize();}

        /**
         * @return the sum of the uploaded sizes of the transfers
         * belonging to this group
         */
        int uploadedSize() const {return m_group->uploadedSize();}

        /**
         * @return the progress percentage
         */
        int percent() const       {return m_group->percent();}

        /**
         * @return the sum of the download speeds of the running transfers 
         * belonging this group
         */
        int downloadSpeed() const         {return m_group->downloadSpeed();}

        /**
         * @return the sum of the upload speeds of the running transfers 
         * belonging this group
         */
        int uploadSpeed() const         {return m_group->uploadSpeed();}
        
        /**
         * @return the changes flags which are currently set on the transfer
         */
        ChangesFlags changesFlags();

        /**
         * Set a default Folder for the group
         * @param folder the new default folder
         */
        void setDefaultFolder(QString folder) {m_group->setDefaultFolder(folder);}

        /**
         * @return the groups default folder
         */
        QString defaultFolder() {return m_group->defaultFolder();}

        /**
         * Sets the regular expression of the group
         * @param regexp the regular expression
         */
        void setRegExp(const QRegExp &regexp) {m_group->setRegExp(regexp);}

        /**
         * @returns the regular expression of the group
         */
        QRegExp regExp() {return m_group->regExp();}

#ifdef HAVE_NEPOMUK
        /**
         * Sets the Nepomuk tags of the group
         * @param tags the Nepomuk tags
         */
        void setTags(const QList<Nepomuk::Tag> &tags) {m_group->setTags(tags);}

        /**
         * @returns the Nepomuk tags of the group
         */
        QList<Nepomuk::Tag> tags() const {return m_group->tags();}
#endif //HAVE_NEPOMUK

        /**
         * Set a Download-Limit for the group
         * @param limit the new download-limit
         * @note if limit is 0, no download-limit is set
         */
         void setDownloadLimit(int dlLimit, Transfer::SpeedLimit limit) {m_group->setDownloadLimit(dlLimit, limit);}

        /**
         * @return the group's Download-Limit
         */
         int downloadLimit(Transfer::SpeedLimit limit) {return m_group->downloadLimit(limit);}

        /**
         * Set a Upload-Limit for the group
         * @param limit the new upload-limit
         * @note this will not be displayed in the GUI
         */
         void setUploadLimit(int ulLimit, Transfer::SpeedLimit limit) {m_group->setUploadLimit(ulLimit, limit);}

        /**
         * @return the group's Upload-Limit
         */
         int uploadLimit(Transfer::SpeedLimit limit) {return m_group->uploadLimit(limit);}

        /**
         * Set the group's icon
         * @param name the icon's name
         */
        void setIconName(const QString &name) {m_group->setIconName(name);}

        /**
         * @returns the group's icon's name
         */
        QString iconName() const {return m_group->iconName();}

        /**
         * @returns the group's icon
         */
        QPixmap pixmap() {return m_group->pixmap();}

        /**
         * @returns the data associated to this TransferGroup item. This is
         * necessary to make the interview model/view work
         */
        QVariant data(int column);

        /**
         * @returns the number of columns associated to the group's data
         */
        int columnCount() const     {return 6;}

        /**
         * @returns the index for the given transfer. If the transfer can't
         *          be found, it returns -1
         */
        int indexOf(TransferHandler * transfer);

        /**
         * @returns a list containing all the transfers belonging to this group.
         */
        const QList<TransferHandler *> transfers();

        /**
         * @returns a pointer to a QObjectInterface object which is a QObject
         * by means of which you can connect signals and slots for this 
         * transfer group.
         */
        const QList<QAction *> & actions();

        /**
         * Calculates the whole SpeedLimits
         */
        void calculateSpeedLimits() {m_group->calculateSpeedLimits();}

        /**
         * Calculates the DownloadLimits
         */
        void calculateDownloadLimit() {m_group->calculateDownloadLimit();}

        /**
         * Calculates the DownloadLimits
         */
        void calculateUploadLimit() {m_group->calculateUploadLimit();}
        
    public slots:
        /**
         * These are all JobQueue-related functions
         */
        void start();
        void stop();
        
    signals:
        void groupChangedEvent(TransferGroupHandler * transfer, TransferGroupHandler::ChangesFlags flags);

    private:
        /**
         * Sets a change flag in the ChangesFlags variable.
         *
         * @param change The TransferChange flag to be set
         * @param notifyModel notify the model about the change
         */
        void setGroupChange(ChangesFlags change, bool notifyModel = false);

        /**
         * Resets the changes flags
         */
        void resetChangesFlags();

        /**
         * Creates all the QActions
         */
        void createActions();

        TransferGroup * m_group;

        QList<QAction *> m_actions;

        ChangesFlags m_changesFlags;
};

#endif
