/* This file is part of the KDE project
   
   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2004 Enrico Ros <eros.kde@email.it>

   Based on:
       kmainwidget.{h,cpp} Copyright (C) 2002 by Patrick Charbonnier
       that was based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <kurl.h>

#include "globals.h"

#include "group.h"
#include "transfer.h"
#include "transferlist.h"
#include "interrogator.h"

class GlobalStatus
{
public:
	// not yet used
	QDateTime timeStamp;
	struct {
		QString interface;
		float speed;
		float maxSpeed;
		float minSpeed;
	} connection;
	struct {
		float totalSize;
		float percentage;
		int transfersNumber;
	} files;
	QStringList others;
};



class Scheduler : public QObject, TransferInterrogator
{
Q_OBJECT
public:
    Scheduler(KMainWidget * _mainWidget);
    ~Scheduler();
    
    bool isRunning()const {return running;}
    
    const TransferList & getTransfers() const {return *transfers;}
    const GroupList & getGroups() {return *groups;}
    
public slots:
    void run();
    void stop();
    
    /**
     * Just an idea for these slots: we can handle 3 cases:
     *  1) src = empty list -> means that the src url must be inserted 
     *     manually from the user (with a dialog popping up)
     *  2) destDir = empty -> means that the destDir must be inserted
     *     manually from the user (with a dialog popping up)
     *  3) destDir = QString("KGet::default") means that the destination 
     *     is the kget default
     *  In this way we can take care of every possible situation using
     *  only a function.
     */
    void slotNewURLs(const KURL::List & src, const QString& destDir);
    
   
    void slotRemoveItems(TransferList);

    void slotSetPriority(TransferList, int);

    void slotSetCommand(TransferList, TransferCommand);

    void slotSetGroup(TransferList, const QString&);
    void slotAddGroup(GroupList);
    void slotDelGroup(GroupList);
    void slotModifyGroup(const QString&, Group);

    
    void slotReqOperation(SchedulerOperation);
    void slotReqOperation(SchedulerDebugOp);

    /**
     * This slot is called from the Transfer object when its progress
     * has changed
     */
    void slotTransferChanged(Transfer *);
    
    /**
     * KGET TRANSFERS FILE related
     */
    
    /**
     * Used to import the transfers included in a .kgt file. 
     * If ask_for_name is true the function opens a KFileDialog 
     * where you can choose the file to read from. If ask_for_name 
     * is false, the function opens the transfers.kgt file
     * placed in the application data directory.
     */
    void slotImportTransfers(bool ask_for_name = false);
    
    /** 
     * This function adds the transfers included in the file location
     * calling the readTransfer function in the transferList object.
     * It checks if the file is valid.
     */
    void slotImportTransfers(const KURL & file);
    
    /**
     * Used to export all the transfers in a .kgt file. 
     * If ask_for_name is true the function opens a KFileDialog 
     * where you can choose the file to write to. If ask_for_name 
     * is false, the function saves to the transfers.kgt file
     * placed in the application data directory.
     */
    void slotExportTransfers(bool ask_for_name = false);

    /**
     * This function reads the transfers included in the file location
     * calling the writeTransfer function in the transferList object.
     * It checks if the file is valid.
     */
    void slotExportTransfers(QString & file);
      
signals:
    void addedItems(const TransferList&);
    void removedItems(const TransferList&);
    void changedItems(const TransferList&);
    void addedGroups(const GroupList&);
    void removedGroups(const GroupList&);
    void changedGroups(const GroupList&);
    
    void clear();
    void globalStatus(GlobalStatus *);
    
private:
    
    /**
     * See the above function for details
     * TEMP(Dario) I have removed the const qualifier to the KURL object
     * becouse I need to modify it when no src is passed to the function
     */
    void slotNewURL(KURL src, const QString& destDir);
    
    /**
     * Called in the slotImportTextFile(...) function.
     * You can add only a Transfer, with destDir being requested with
     * a KFileDialog
     */
    void addTransfer(const QString& src);
    
    /**
     * Low level function called by addTransfers(...) and addTransfer(...).
     * It adds a Transfer. destFile must be a file, not a directory!
     */
    Transfer * addTransferEx(const KURL& url, const KURL& destFile = KURL());

    /**
     * Checks if the given url is valid or not.
     */
    bool isValidURL( KURL url );
    
    /**
     * Checks if the given destination dir is valid or not. If not
     * dialogs appear where the user can insert a valid one.
     */
    KURL getValidDest( const QString& filename, const KURL& dest);
    
    QString getSaveDirectoryFor( const QString& filename ) const;
    
    /**
     * This function is a low-level function that executes a command on
     * a given transfer. It differs from slotSetCommand becouse slotSetCommand
     * is a higher-level function, that checks the runningTransfers queue
     * to be sure the given commands can be executed now (calling 
     * queueEvaluateTransfers)
     */
    bool setTransferCommand(TransferList, TransferCommand);
    bool setTransferCommand(Transfer *, TransferCommand);
    
    /**
     * This function checks if the given list contains transfers
     * having a higher priority (lower getPriority()) than those 
     * we are currently downloading. If this is true, it replaces
     * them.
     * If force=true, when the given Transfers have the same priority
     * than the currently downloading ones, they will replace
     * them
     */
    void queueEvaluateItems(TransferList, bool force=false);
    
    /**
     * This function checks if the given list contains transfers
     * that we are currently downloading. If yes it removes (and stops)
     * them from the runningTransfers list.
     * It is called whenever we remove a list of transfer.
     */
    void queueRemovedItems(TransferList);
        
    /**
     * Checks the number of running transfers. It starts the
     * transfers, trying to make the number of running transfers
     * equal to the value specified by the user (in the configuration dlg).
     */
    void queueUpdate();
    
    /**
     * Returns the ConnectionInterface for the selected transfer.
     * ..FIXME.. behavior not well defined yet.
     * @param transfer if null this will return the default connection.
     */
    Connection * connectionFromTransfer( Transfer * transfer );
     
    TransferList * transfers;
    TransferList * removedTransfers;
    TransferList * runningTransfers;
    GroupList * groups;
    KMainWidget * mainWidget;
    QValueList<Connection*> connections;
    bool running;
};

#endif
