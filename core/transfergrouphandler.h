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
#include "kget_export.h"
#include "observer.h"
#include "kget.h"

class QAction;

class QObjectInterface;
class GenericTransferObserver;
class TransferGroupObserver;
class TransferHandler;
class KGetKJobAdapter;
class Scheduler;

class KGET_EXPORT TransferGroupHandler : public Handler
{
    friend class GenericTransferObserver;
    friend class TransferGroup;
    friend class TransferTreeModel;
    friend class KGet;
    public:

        typedef TransferGroup::ChangesFlags ChangesFlags;

        TransferGroupHandler(TransferGroup * group, Scheduler * scheduler);

        ~TransferGroupHandler();

        /**
         * Adds an observer to this TransferGroup
         * Note that the observer with pointer == 0 is added by default, and
         * it's used by the TransferTreeModel class
         *
         * @param observer The new observer that should be added
         */
        void addObserver(TransferGroupObserver * observer);

        /**
         * Removes an observer from this TransferGroup
         *
         * @param observer The observer that should be removed
         */
        void delObserver(TransferGroupObserver * observer);

        /**
         * These are all JobQueue-related functions
         */
        void start();
        void stop();
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
        void setTags(const QStringList &tags) {m_group->setTags(tags);}

        /**
         * @returns the Nepomuk tags of the group
         */
        QStringList tags() const {return m_group->tags();}
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
         * Returns the changes flags
         *
         * @param observer The observer that makes this request
         */
        ChangesFlags changesFlags(TransferGroupObserver * observer);

        /**
         * Resets the changes flags for a given TransferObserver
         *
         * @param observer The observer that makes this request
         */
        void resetChangesFlags(TransferGroupObserver * observer);

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
         * @returns a pointer to a QObjectInterface object which is a QObject
         * by means of which you can connect signals and slots for this 
         * transfer group.
         */
        QObjectInterface * qObject();

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

    private:
        /**
         * Sets a change flag in the ChangesFlags variable.
         *
         * @param change The TransferChange flag to be set
         * @param postEvent if false the handler will not post an event to the
         * observers, if true the handler will post an event to the observers
         */
        void setGroupChange(ChangesFlags change, bool postEvent = false);

        /**
         * Posts a TransferGroupChangedEvent to all the observers
         */
        void postGroupChangedEvent();

        /**
         * Posts an addedTransferEvent to all the observers
         *
         * @param transfer the transfer that has been added to the group
         * @param after the transfer after which it has been added
         */
        void postAddedTransferEvent(Transfer * transfer, Transfer * after);

        /**
         * Posts an removedTransferEvent to all the observers
         *
         * @param transfer the transfer that has been removed from the group
         */
        void postRemovedTransferEvent(Transfer * transfer);

        /**
         * Posts an movedTransferEvent to all the observers
         *
         * @param transfer the transfer that has been removed from the group
         * @param after the transfer after which the it has been moved
         */
        void postMovedTransferEvent(Transfer * transfer, Transfer * after);

        /**
         * Posts a deleteEvent to all the observers
         */
        void postDeleteEvent();

        /**
         * Creates all the QActions
         */
        void createActions();

        TransferGroup * m_group;

        QObjectInterface * m_qobject;
        QList<QAction *> m_actions;

        QList<TransferGroupObserver *> m_observers;
        QMap<TransferGroupObserver *, ChangesFlags> m_changesFlags;
};


class QObjectInterface : public QObject
{
    Q_OBJECT
    public:
        QObjectInterface(TransferGroupHandler * handler);

    public slots:
        void slotStart();        //Starts the download process
        void slotStop();         //Stops the download process

    signals:

    private:
        TransferGroupHandler * m_handler;
};

class GenericTransferGroupObserver : public TransferGroupObserver, public QObject
{
    public:
        GenericTransferGroupObserver(QObject * parent);
        ~GenericTransferGroupObserver();

        virtual void groupChangedEvent(TransferGroupHandler * group);

        virtual void addedTransferEvent(TransferHandler * transfer, TransferHandler * after);

        virtual void removedTransferEvent(TransferHandler * transfer);

        virtual void movedTransferEvent(TransferHandler * transfer, TransferHandler * after);

        void addTransferGroup(TransferGroupHandler *group);

        void postTransferChanged(TransferHandler *);

    private:
        void registerTransferAsKJob(TransferHandler *);

    private:
        GenericTransferObserver *m_transferObserver;
        QMap <TransferHandler *, KGetKJobAdapter *> m_adapters;
};

#endif
