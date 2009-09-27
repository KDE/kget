/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

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
#include <KNotification>
#include <ktabwidget.h>

#include <QtXml/QDomElement>

#include "kuiserverjobs.h"
#include "scheduler.h"
#include "kget_export.h"
#include "transfer.h"
#include "transfergrouphandler.h"

class QDomElement;
class QAbstractItemView;

class KComboBox;

class TransferDataSource;
class TransferGroup;
class TransferHandler;
class TransferFactory;
class TransferTreeModel;
class TransferTreeSelectionModel;
class KGetPlugin;
class MainWindow;
class NewTransferDialog;
class TransferGroupScheduler;
class KPassivePopup;


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
    friend class GenericObserver;
    friend class TransferTreeModel;
    public:
        enum AfterFinishAction {
            Quit = 0,
            Shutdown = 1
        };
        ~KGet();

        static KGet* self( MainWindow * mainWindow=0 );

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
        static void delGroup(TransferGroupHandler * group, bool askUser = true);
        
        /**
         * Removes specific groups from the KGet.
         *
         * @param groups The names of the groups to be deleted.
         */
        static void delGroups(QList<TransferGroupHandler*> groups, bool askUser = true);

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
	 * @param suggestedFileName a suggestion of a simple filename to be saved in destDir
         * @param groupName The name of the group the new transfer will belong to
         * @param start Specifies if the newly added transfers should be started.
         * If the group queue is already in a running state, this flag does nothing
         */
        static TransferHandler * addTransfer(KUrl srcUrl, QString destDir = QString(), QString suggestedFileName = QString(),
                                const QString& groupName = QString(), bool start = false);

        /**
         * Adds a new transfer to the KGet
         *
         * @param e The transfer's dom element
         * @param groupName The name of the group the new transfer will belong to
         */
        static TransferHandler * addTransfer(const QDomElement& e, const QString& groupName = QString());

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
        static const QList<TransferHandler *> addTransfer(KUrl::List srcUrls, QString destDir = QString(),
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
        * @returns the list of the finished transfers
        */
        static QList<TransferHandler *> finishedTransfers();

        /**
         * @returns the list of selected groups
         */
        static QList<TransferGroupHandler *>
        selectedTransferGroups();

        /**
         * @returns a pointer to the TransferTreeModel object
         */
        static TransferTreeModel * model();
        
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
         * @param plain should list be in plain mode or kget mode
         */
        static void save( QString filename=QString(), bool plain=false );
        
        /**
         * @returns a list of all transferfactories
         */
        static QList<TransferFactory*> factories();

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
         * Sets the given comboBox to the TransferTreeModel object
         */
        static void addTransferView(KComboBox * view);

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
         * Get the group with the given name
         * @param name the name
         */
        static TransferGroupHandler * findGroup(const QString &name);

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
          * @return a list of the groups assigned to the filename of a transfer
          */
        static QList<TransferGroupHandler*> groupsFromExceptions(const KUrl &filename);

        /**
         * Scans for all the available plugins and creates the proper
         * transfer DataSource object for transfers Containers
         *
         * @param src Source Url
         * @param type the type of the DataSource that should be created e.g. <TransferDataSource type="search" />
         * this is only needed when creating a "special" TransferDataSource like the search for Urls
         * you can set additional information and the TransferDataSource will use it if it can
         */
        static TransferDataSource * createTransferDataSource(const KUrl &src, const QDomElement &type = QDomElement());

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


        /**
        * Shows a knotification 
        * @param parent QWidget parent of the notification
        * @param eventId Notification type
        * @param text Description of the information showed by the notification
        * @param icon Pixmap showed in the notification, by default 'dialog-error'
        */
        static void showNotification(QWidget *parent, KNotification::StandardEvent eventId, 
                                     const QString &text,
                                     const QString &icon = QString("dialog-error"));

        static void loadPlugins();

        /**
         * Returns a download directory
         * @param preferXDGDownloadDir if true the XDG_DOWNLOAD_DIR will be taken if it is not empty
         * @note depending if the directories exist it will return them in the following order:
         * (preferXDGDownloadDirectory >) lastDirectory > Desktop > Home
         */
        static QString generalDestDir(bool preferXDGDownloadDir = false);

    private:
        KGet();

        /**
         * Scans for all the available plugins and creates the proper
         * transfer object for the given src url
         *
         * @param src the source url
         * @param dest the destination url
         * @param groupName the group name
         * @param start Specifies if the newly added transfers should be started.
         */
        static TransferHandler * createTransfer(const KUrl &src, const KUrl &dest, const QString& groupName = QString(), bool start = false, const QDomElement * e = 0);

        static KUrl urlInputDialog();
        static QString destDirInputDialog();
        static KUrl destFileInputDialog(QString destDir = QString(), const QString& suggestedFileName = QString());

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

        static KUrl getValidDestUrl(const QString& destDir, const KUrl &srcUrl, const QString& destFileName = QString());

        //Plugin-related functions
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

class GenericObserver : public QObject
{
    Q_OBJECT
    public:
        GenericObserver(QObject *parent = 0);
        virtual ~GenericObserver ();

    public slots:
        void groupAddedEvent(TransferGroupHandler *handler);
        void groupRemovedEvent(TransferGroupHandler *handler);
        void transferAddedEvent(TransferHandler *handler, TransferGroupHandler *group);
        void transferRemovedEvent(TransferHandler *handler, TransferGroupHandler *group);
        void transfersChangedEvent(QMap<TransferHandler*, Transfer::ChangesFlags> transfers);
        void groupsChangedEvent(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags> groups);
        void transferMovedEvent(TransferHandler *, TransferGroupHandler *);
        
#ifdef HAVE_KWORKSPACE
    private slots:
        void slotShutdown();
#endif

    private:
        bool allTransfersFinished();
        KPassivePopup* popupMessage(const QString &title, const QString &message);

        void checkAndFinish();

#ifdef HAVE_KWORKSPACE
        void checkAndShutdown();
#endif
        void checkAndUpdateSystemTray();
};
#endif
