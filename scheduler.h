#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qdatetime.h>
#include <qstringlist.h>
#include <kurl.h>

class QString;

class Transfer;

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
	Scheduler();
	~Scheduler();
	
	enum Operation {};
	
signals:
	void addedItems(QValueList<Transfer *>);
	void removedItems(QValueList<Transfer *>);
	void changedItems(QValueList<Transfer *>);
	void clear();
	void globalStatus(GlobalStatus *);
	
public slots:
	void slotNewURLs(const KURL::List &);
	void slotNewURL(const KURL &);
    
	void slotRemoveItems(QValueList<Transfer *>);
	void slotRemoveItem(Transfer *);
    
	void slotSetPriority(QValueList<Transfer *>, int);
	void slotSetPriority(Transfer *, int);
    
	void slotSetOperation(QValueList<Transfer *>, enum Operation);
	void slotSetOperation(Transfer *, enum Operation);
    
	void slotSetGroup(QValueList<Transfer *>, const QString &);
	void slotSetGroup(Transfer *, const QString &);

    /**
     * Used to import the URLS
     *
     */
    void slotImportTextFile();
        
    /**
     * Used to import the transfers included in a .kgt file. The function
     * opens a KFileDialog where it is possible to select the file.
     */
    void slotImportTransfers();
    
    /**
     * Used to export the all the transfers in a .kgt file. The function
     * opens a KFileDialog where it is possible to select the file.
     */
    void slotExportTransfers();
    
private:
    /**
     * KGET CONFIGURATION FILES related
     */
    
    /**
     * Called by slotImportTransfers(). If ask_for_name is true the function
     * opens a KFileDialog where you can choose the file to read from. If
     * ask_for_name is false, the function opens the transfers.kgt file
     * placed in the application data directory
     */
    void readTransfers(bool ask_for_name);
    
    /**
     * Low level function. It is called from readTransfers(...)  and
     * addTransfers(...).
     * This function adds the transfers included in the file location
     * calling the readTransfer function in the transferList object.
     * It checks if the file is valid.
     */
    void readTransfers(const KURL & file);
    
    /**
     * Called by slotExportTransfers(). If ask_for_name is true the function
     * opens a KFileDialog where you can choose the file to write to. If
     * ask_for_name is false, the function saves to the transfers.kgt file
     * placed in the application data directory.
     */
    void writeTransfers(bool ask_for_name);
    
    /**
     * Low level function.
     * This function reads the transfers included in the file location
     * calling the writeTransfer function in the transferList object.
     * It checks if the file is valid.
     */
    void writeTransfers(const QString & file);
    
    
    /**
     * Functions used to add Transfers from URLS
     */
         
    /**
     * Called from the KMainWidget class in the dropEvent function.
     * This function is used to add new Transfers. If an URL in the
     * KURL::List is referred to a .kgt file, the transfers included
     * in that configuration file are automatically added. The destDir
     * contains the destination directory.
     */
    void addTransfers(const KURL::List& src, const QString& destDir);
    
    /**
     * Called by the KMainWidget class in the dropEvent, and in the 
     * slotImportTextFile(...) function.
     * Like the function above. 
     * You can add only a Transfer, with destDir being requested with
     * a KFileDialog
     */
    void addTransfer(const QString& src);
    
    /**
     * Low level function called by addTransfers(...) and addTransfer(...).
     * It adds a Transfer. destFile must be a file, not a directory!
     */
    void addTransferEx(const KURL& url, const KURL& destFile = KURL());

    /**
     * Checks if the given url is valid or not.
     */
    bool sanityChecksSuccessful( const KURL& url );
    
    void checkQueue();
};

#endif
