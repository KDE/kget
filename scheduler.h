#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <kurl.h>

#include "globals.h"

#include "transfer.h"
#include "transferlist.h"

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



class Scheduler : public QObject
{
Q_OBJECT
public:
	Scheduler(KMainWidget * _mainWidget);
	~Scheduler();

    
signals:
	void addedItems(TransferList &);
	void removedItems(TransferList &);
	void changedItems(TransferList &);
	void clear();
	void globalStatus(GlobalStatus *);
	
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
    
    /**
     * See the above function for details
     * TEMP(Dario) I have removed the const qualifier to the KURL object
     * becouse I need to modify it when no src is passed to the function
     */
	void slotNewURL(KURL src, const QString& destDir);
    
	void slotRemoveItems(TransferList &);
	void slotRemoveItem(Transfer *);
    
	void slotSetPriority(TransferList &, int);
	void slotSetPriority(Transfer *, int);
    
	void slotSetCommand(TransferList &, TransferCommand);
	void slotSetCommand(Transfer *, TransferCommand);
    
	void slotSetGroup(TransferList &, const QString &);
	void slotSetGroup(Transfer *, const QString &);

	void slotReqOperation(SchedulerOperation);

    /**
     * This slot is called from the Transfer object when its status
     * has changed
     */
    void slotTransferMessage(Transfer *, TransferMessage);
    
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
     * Used to export all the transfers in a .kgt file. 
     * If ask_for_name is true the function opens a KFileDialog 
     * where you can choose the file to write to. If ask_for_name 
     * is false, the function saves to the transfers.kgt file
     * placed in the application data directory.
     */
    void slotExportTransfers(bool ask_for_name = false);
  
    /** 
     * This function adds the transfers included in the file location
     * calling the readTransfer function in the transferList object.
     * It checks if the file is valid.
     */
    void slotReadTransfers(const KURL & file);
    
private:
    
    /**
     * This function reads the transfers included in the file location
     * calling the writeTransfer function in the transferList object.
     * It checks if the file is valid.
     */
    void writeTransfers(const QString & file);
    
    /**
     * Functions used to add Transfers from URLS
     */
         
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
    bool isValidURL( const KURL& url );
    
    /**
     * Checks if the given destination dir is valid or not.
     */
    bool isValidDest( const KURL& url);
    
    QString getSaveDirectoryFor( const QString& filename ) const;
    
    void checkQueue();
    
    /**
     * Returns the ConnectionInterface for the selected transfer.
     * ..FIXME.. behavior not well defined yet.
     * @param transfer if null this will return the default connection.
     */
    Connection * connectionFromTransfer( Transfer * transfer );
     
    TransferList * transfers;
    TransferList * removedTransfers;
    KMainWidget * mainWidget;
    QValueList<Connection*> connections;
};

#endif
