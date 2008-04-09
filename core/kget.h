/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   Based on:
       kmainwidget.{h,cpp}
       Copyright (C) 2002 by Patrick Charbonnier <pch@freeshell.org>
       that was based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KGET_H
#define KGET_H

#include <kservice.h>
#include <kurl.h>
#include <kactioncollection.h>
#include <ktabwidget.h>

#include "kuiserverjobs.h"
#include "scheduler.h"
#include "kget_export.h"
#include "observer.h"

class QDomElement;
class QAbstractItemView;

class Transfer;
class TransferDataSource;
class TransferGroup;
class TransferHandler;
class TransferGroupHandler;
class TransferFactory;
class TransferTreeModel;
class TransferTreeSelectionModel;
class ModelObserver;
class KGetPlugin;
class MainWindow;
class NewTransferDialog;
class TransferGroupScheduler;

/**
 * This is our KGet class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

class KGET_EXPORT KGet
{
    friend class NewTransferDialog;
    friend class GenericTransferObserver;

    public:
        static KGet& self( MainWindow * mainWindow=0 );

        /**
         * Adds a new observer of the KGet. See observer.h for more info about it.
         *
         * @param observer The new observer
         */
        static void addObserver(ModelObserver * observer);

        /**
         * Removes an observer of the KGet. See observer.h for more info about it.
         *
         * @param observer The observer to remove
         */
        static void delObserver(ModelObserver * observer);

        /**
         * Adds a new group to the KGet.
         *
         * @param groupName The name of the new group
         *
         * @returns true if the group has been successully added, otherwise
         *          it returns false, probably because a group with that named
         *          already exists
         */
        static bool addGroup(const QString& groupName);

        /**
         * Removes a group from the KGet.
         *
         * @param groupName The name of the group to be deleted
         */
        static void delGroup(const QString& groupName);

        /**
         * Changes the name of the group
         * 
         * @param oldName the name of the group to be changed
         * @param newName the new name of the group
         */
        static void renameGroup(const QString& oldName, const QString& newName);

        /**
         * @returns the name of the available transfers groups
         */
        static QStringList transferGroupNames();

        /**
         * Adds a new transfer to the KGet
         *
         * @param srcUrl The url to be downloaded
         * @param destDir The destination directory. If empty we show a dialog
         * where the user can choose it.
         * @param groupName The name of the group the new transfer will belong to
         * @param start Specifies if the newly added transfers should be started.
         * If the group queue is already in a running state, this flag does nothing
         */
        static void addTransfer(KUrl srcUrl, QString destDir = QString(),
                                const QString& groupName = QString(), bool start=false);

        /**
         * Adds a new transfer to the KGet
         *
         * @param e The transfer's dom element
         * @param groupName The name of the group the new transfer will belong to
         */
        static void addTransfer(const QDomElement& e, const QString& groupName = QString());

        /**
         * Adds new transfers to the KGet
         *
         * @param srcUrls The urls to be downloaded
         * @param destDir The destination directory. If empty we show a dialog
         * where the user can choose it.
         * @param groupName The name of the group the new transfer will belong to
         * @param start Specifies if the newly added transfers should be started.
         * If the group queue is already in a running state, this flag does nothing
         */
        static void addTransfer(KUrl::List srcUrls, QString destDir = QString(),
                                const QString& groupName = QString(), bool start=false);

        /**
         * Removes a transfer from the KGet
         *
         * @param transfer The transfer to be removed
         */
        static bool delTransfer(TransferHandler * transfer);

        /**
         * Moves a transfer to a new group
         *
         * @param transfer The transfer to be moved
         * @param groupName The name of the new transfer's group
         */
        static void moveTransfer(TransferHandler * transfer, const QString& groupName);

        /**
         * Redownload a transfer
         * @param transfer the transfer to redownload
         */
        static void redownloadTransfer(TransferHandler * transfer);

        /**
         * @returns the list of selected transfers
         */
        static QList<TransferHandler *> selectedTransfers();

        /**
         * @returns the list of selected groups
         */
        static QList<TransferGroupHandler *>
        selectedTransferGroups();

        /**
         * @returns a pointer to the QItemSelectionModel object
         */
        static TransferTreeSelectionModel * selectionModel();

        /**
         * Imports the transfers and groups included in the provided xml file
         *
         * @param filename the file name to 
         */
        static void load( QString filename=QString() );

        /**
         * Exports all the transfers and groups to the given file
         *
         * @param filename the file name
         */
        static void save( QString filename=QString() );

        /**
         * @returns The factory of a given transfer
         *
         * @param transfer the transfer about which we want to have the factory
         */
        static TransferFactory * factory(TransferHandler * transfer);

        /**
         * @return a pointer to the KActionCollection objects
         */
        static KActionCollection * actionCollection();
	
	/**
	 * if running == true starts the scheduler
	 * if running == false stops the scheduler
	 */
	static void setSchedulerRunning(bool running=true);

       /**
        * Returns true if the scheduler has running jobs.
        */
        static bool schedulerRunning();

        /**
         * Sets the given view to the TransferTreeModel object
         */
        static void addTransferView(QAbstractItemView * view);

        /**
         * Adds a Settings tab for every plugins that needs one
         * to the KTabWidget.
         */
        static void setPluginsSettingsWidget(KTabWidget * widget);

        /**
         * Gets all transfers
         */
        static QList<TransferHandler*> allTransfers();

        /**
         * Gets all transfer-groups
         */
        static QList<TransferGroupHandler*> allTransferGroups();

        /**
         * Get the transfer with the given url
         * @param src the url
         */
        static TransferHandler * findTransfer(const KUrl &src);

        /**
         * Run this function for enabling the systemTray 
         * (will be automatically done, if there is download running)
         */
        static void checkSystemTray();

        /**
         * This will be called when the settings have been changed
         */
        static void settingsChanged();

        /**
         * Returns the kget kuiserver jobs manager
         *
         */
         static void registerKJob(KJob *);
         static void unregisterKJob(KJob *);
         static void reloadKJobs();

         /**
          * @return a list of the defaultFolders for a transfer
          */
         static QStringList defaultFolders(const KUrl &filename, const QString &groupname);

        /**
         * Scans for all the available plugins and creates the proper
         * transfer DataSource object for transfers Containers
         *
         * @param src Source Url
         */
        static TransferDataSource * createTransferDataSource(const KUrl &src);

        /**
         * Sets the global download limit
         * @param limit the new global download limit
         */
        static void setGlobalDownloadLimit(int limit);

        /**
         * Sets the global upload limit
         * @param limit the new global upload limit
         */
        static void setGlobalUploadLimit(int limit);

        /**
         * Recalculates the global speedlimits
         */
        static void calculateGlobalSpeedLimits();

        /**
         * Recalculates the global download-limit
         */
        static void calculateGlobalDownloadLimit();

        /**
         * Recalculates the global upload-limit
         */
        static void calculateGlobalUploadLimit();

    private:
        KGet();
        ~KGet();

        /**
         * Scans for all the available plugins and creates the proper
         * transfer object for the given src url
         *
         * @param src the source url
         * @param dest the destination url
         * @param groupName the group name
         * @param start Specifies if the newly added transfers should be started.
         */
        static bool createTransfer(const KUrl &src, const KUrl &dest, const QString& groupName = QString(), bool start = false, const QDomElement * e = 0);

        /**
         * Posts an addedTransferGroupEvent to all the observers
         *
         * @param group the group that has been added to the group
         */
        static void postAddedTransferGroupEvent(TransferGroup * group, ModelObserver * observer = 0);

        /**
         * Posts an removedTransferGroupEvent to all the observers
         *
         * @param group the group that has been removed from the group
         */
        static void postRemovedTransferGroupEvent(TransferGroup * group, ModelObserver * observer = 0);

        static KUrl urlInputDialog();
        static QString destInputDialog();
        static QString getSaveDirectoryFromExceptions(const KUrl &filename);

        static bool isValidSource(const KUrl &source);
        static bool isValidDestDirectory(const QString& destDir);

        /**
         * if the given url is a file that already exists the function asks
         * the user to confirm the overwriting action
         *
         * @param destFile the url of the destination file
         * @return true if the destination file is ok, otherwise returns false
         */
        static bool isValidDestUrl(const KUrl &destUrl);

        static KUrl getValidDestUrl(const QString& destDir, const KUrl &srcUrl);

        static TransferGroup * findGroup(const QString& groupName);

        //Plugin-related functions
        static void loadPlugins();
        static void unloadPlugins();
        static KGetPlugin * createPluginFromService( const KService::Ptr &service );


        /**
         * Deletes the given file, if possible.
         *
         * @param url The file to delete
         *
         * @return true if the file was successully deleted: if the given url
         * is a directory or if it is not local it returns false and shows a
         * warning message.
         */
        static bool safeDeleteFile( const KUrl& url );

        static QList<ModelObserver *> m_observers;

        //Interview models
        static TransferTreeModel * m_transferTreeModel;
        static TransferTreeSelectionModel * m_selectionModel;

        //Lists of available plugins
        static QList<TransferFactory *> m_transferFactories;

        //pointer to the Main window
        static MainWindow * m_mainWindow;

        //Scheduler object
        static TransferGroupScheduler * m_scheduler;

        //pointer to the kget uiserver jobs manager
        static KUiServerJobs *m_jobManager;
};

#endif
