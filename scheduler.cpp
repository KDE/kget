#include <qregexp.h>

#include <kurl.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kinputdialog.h>

#include "scheduler.h"
#include "connection.h"
#include "safedelete.h"
#include "settings.h"
#include "kmainwidget.h"

Scheduler::Scheduler(KMainWidget * _mainWidget)
    : QObject(),
      mainWidget(_mainWidget)
{
    transfers = new TransferList(this);
    removedTransfers = new TransferList(this);
    connections.append( new Connection(this) );
}

Scheduler::~Scheduler()
{

}

void Scheduler::run()
{

}

void Scheduler::stop()
{

}

void Scheduler::slotNewURLs(const KURL::List & src, const QString& destDir)
{
    sDebugIn << endl;
    
    KURL::List urlsToDownload;

    for ( KURL::List::ConstIterator it = src.begin(); it != src.end(); ++it )
    {
        ///sDebug << "AAA" << endl;

        KURL url = *it;
        if ( url.fileName().endsWith( ".kgt" ) )
            slotReadTransfers(url);
        else
            urlsToDownload.append( url );
    }

    //sDebug << "BBB" << endl;
    
    if ( urlsToDownload.isEmpty() )
        return;

    //sDebug << "CCC" << endl;

    
    if ( urlsToDownload.count() == 1 ) // just one file -> ask for filename
        slotNewURL(src.first(), destDir);

    //sDebug << "DDD" << endl;
    
    // multiple files -> ask for directory, not for every single filename
    KURL dest;
    if ( destDir.isEmpty() || !QFileInfo( destDir ).isDir() )
    {
        //sDebug << "EEE" << endl;
        if ( !destDir.isEmpty()  )
            dest.setPath( destDir );
        else
            dest.setPath( getSaveDirectoryFor( src.first().fileName() ) );

        // ask in any case, when destDir is empty
        if ( destDir.isEmpty() || !QFileInfo( dest.path() ).isDir() )
        {
            QString dir = KFileDialog::getExistingDirectory( dest.path() );
            if ( dir.isEmpty() ) // aborted
                return;

            dest.setPath( dir );
            ksettings.lastDirectory = dir;
        }
    }

    // dest is now finally the real destination directory for all the files
    dest.adjustPath(+1);

    TransferList list(this);
    
    // create new transfer items
    KURL::List::ConstIterator it = urlsToDownload.begin();
    for ( ; it != urlsToDownload.end(); ++it )
    {
        KURL srcURL = *it;

        if ( !isValidURL( *it ) )
            continue;

        KURL destURL = dest;
        QString fileName = (*it).fileName();
        if ( fileName.isEmpty() ) // simply use the full url as filename
            fileName = KURL::encode_string_no_slash( (*it).prettyURL() );

        destURL.setFileName( fileName );

        if(KIO::NetAccess::exists(destURL, false, mainWidget))
        {
            if (KMessageBox::warningYesNo(mainWidget,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL() ) )
                                          == KMessageBox::Yes)
            {
                SafeDelete::deleteFile( destURL );
            }
        }

        //Transfer *item = transfers->addTransfer(*it, destURL);
        Transfer *item = new Transfer(this, *it, destURL);
        list.addTransfer(item);
    }

    transfers->addTransfers(list);
    
    emit addedItems(list);
    
    
    //transfers->clearSelection();

/*    if (ksettings.b_useSound) {
        KAudioPlayer::play(ksettings.audioAdded);
    }
*/    
    //checkQueue();

    sDebugOut << endl;
}


void Scheduler::slotNewURL(KURL src, const QString& destDir)
{
    sDebugIn << endl;
    
    /**
     * No src location: we let the user insert it manually
     */
    if ( src.isEmpty() )
        {
        //sDebug << "AAA" << endl;
        
        QString newtransfer;
        bool ok = false;
    
        while (!ok) 
            {
            //sDebug << "BBB" << endl;
            
            newtransfer = KInputDialog::getText(i18n("Open Transfer"), i18n("Open transfer:"), newtransfer, &ok, mainWidget);
    
            // user presses cancel
            if (!ok) 
                {
                return;
            }
    
            src = KURL::fromPathOrURL(newtransfer);
    
            if (!src.isValid()) 
                {
                KMessageBox::error(mainWidget, i18n("Malformed URL:\n%1").arg(newtransfer), i18n("Error"));
                ok = false;
            }
        }
    }
        
    KURL destFile;

    /**
     * destDir not empty: if the file exists the function
     * asks the user to confirm the overwriting action
     */
    if ( !destDir.isEmpty() )
    {
        // create a proper destination file from destDir
        KURL destURL = KURL::fromPathOrURL( destDir );
        QString fileName = src.fileName();

        // in case the fileName is empty, we simply ask for a filename in
        // addTransferEx. Do NOT attempt to use an empty filename, that
        // would be a directory (and we don't want to overwrite that!)
        if ( !fileName.isEmpty() )
        {
            destURL.adjustPath( +1 );
            destURL.setFileName( fileName );
            if(KIO::NetAccess::exists(destURL, false, mainWidget))
            {
                if (KMessageBox::warningYesNo(mainWidget,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()) )
                    == KMessageBox::Yes)
                {
                    SafeDelete::deleteFile( destURL );
                    destFile = destURL;
                }
            }
        }
    }

    //sDebug << "CCC" << endl;
    
    Transfer * item = addTransferEx( src, destFile );

    //sDebug << "DDD" << endl;
    
    //In this case we have inserted nothing
    if (item == 0)
        return;
    
    TransferList list(this);
    list.addTransfer(item);
       
    emit addedItems(list);
    
    sDebugOut << endl;
}

void Scheduler::slotRemoveItems(TransferList & list)
{
    transfers->removeTransfers(list);
    removedTransfers->addTransfers(list);
    
    emit removedItems(list);
}

void Scheduler::slotRemoveItem(Transfer * item)
{
    transfers->removeTransfer(item);
    removedTransfers->addTransfer(item);
    
    TransferList list(this);
    list.addTransfer(item);
    
    emit removedItems(list);
}

void Scheduler::slotSetPriority(TransferList & list, int priority)
{
    sDebugIn << endl;
    
    transfers->moveToBegin(list, priority);
    
    emit changedItems(list);
    
    sDebugOut << endl;
}

void Scheduler::slotSetPriority(Transfer * item, int priority)
{
    transfers->moveToBegin(item, priority);
    
    TransferList list(this);
    list.addTransfer(item);
    
    emit changedItems(list);
}

void Scheduler::slotSetCommand(TransferList & list, TransferCommand op)
{
    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();
    
    for(; it != endList; ++it)
        {
        slotSetCommand(*it, op);
    }
}

void Scheduler::slotSetCommand(Transfer * item, TransferCommand op)
{
    switch (op)
        {
        case CmdResume:
                item->slotResume();
                break;        
        case CmdRestart:
                item->slotRequestRestart();
                break;
        case CmdPause:
                item->slotRequestPause();
                break;
    }
}

void Scheduler::slotSetGroup(TransferList & list, const QString & groupName)
{
    TransferList::iterator it;
    TransferList::iterator endList = list.end();
    
    for(it = list.begin(); it != endList; ++it)
        {
        (*it)->setGroup(groupName);
    }
    
    emit changedItems(list);
}

void Scheduler::slotSetGroup(Transfer * item, const QString & groupName)
{
    item->setGroup(groupName);

    TransferList list(this);
    list.addTransfer(item);
    
    emit changedItems(list);
}

void Scheduler::slotReqOperation(SchedulerOperation operation)
{
    switch(operation)
    {
        case OpPasteTransfer:
            kdDebug() << "PASTE TRANSFER NOT IMPLEMENTED" << endl;
// 	        QString newtransfer;
// 
//     newtransfer = QApplication::clipboard()->text();
//     newtransfer = newtransfer.stripWhiteSpace();
// 
//     if (!ksettings.b_expertMode) {
//         bool ok = false;
//         newtransfer = KInputDialog::getText(i18n("Open Transfer"), i18n("Open transfer:"), newtransfer, &ok, this);
// 
//         if (!ok)
//             return;
//     }
// 
//     if (!newtransfer.isEmpty())
//         ### addTransfer(newtransfer);
            break;

        case OpImportTextFile:
            kdDebug() << "IMPORT TRANSFER LIST NOT IMPLEMENTED" << endl;
//     QString tmpFile;
//     QString list;
//     int i, j;
// 
//     KURL filename = KFileDialog::getOpenURL(ksettings.lastDirectory);
//     if (!filename.isValid())
//         return;
// 
//     if (KIO::NetAccess::download(filename, tmpFile, this)) {
//         list = kFileToString(tmpFile);
//         KIO::NetAccess::removeTempFile(tmpFile);
//     } else
//         list = kFileToString(filename.path()); // file not accessible -> give error message
// 
//     i = 0;
//     while ((j = list.find('\n', i)) != -1) {
//         QString newtransfer = list.mid(i, j - i);
//         addTransfer(newtransfer);
//         i = j + 1;
//     }
            break;

        case OpImportTransfers:
            slotImportTransfers(false);
            break;

        case OpExportTransfers:
            slotExportTransfers(false);
            break;

        case OpRun:
            run();
            break;

        case OpStop:
            stop();
            break;

        default:
            kdDebug() << "SCHEDULER::SLOTREQOPERATION (" << operation << ") NOT IMPLEMENTED" << endl;
            break;
    }
}

void Scheduler::slotTransferMessage(Transfer * item, TransferMessage msg)
{
    sDebugIn << endl;    
    TransferList list(this);
    list.addTransfer(item);
    
    emit changedItems(list);
    sDebugOut << endl;    
}


void Scheduler::slotImportTransfers(bool ask_for_name)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    KURL url;

    if (ask_for_name)
        url = KFileDialog::getOpenURL(ksettings.lastDirectory, i18n("*.kgt|*.kgt\n*|All Files"));
    else
        url.setPath( locateLocal("appdata", "transfers.kgt") );

    slotReadTransfers(url);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void Scheduler::slotReadTransfers(const KURL & file)
{

#ifdef _DEBUG
  sDebugIn << endl;
#endif

    if (!file.isValid()) {

#ifdef _DEBUG
        sDebugOut<< " string empty" << endl;
#endif
        return;
    }
#ifdef _DEBUG
    sDebug << "Read from file: " << file << endl;
#endif
    transfers->readTransfers(file);
    //checkQueue(); <--- TO BE ENABLED
    
    
    //slotTransferTimeout();
    //transfers->clearSelection();

    emit addedItems(*transfers);

#ifdef _DEBUG
    sDebugOut << endl;
#endif

}

void Scheduler::slotExportTransfers(bool ask_for_name)
{
/*
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString str;
    QString txt;

    if (ask_for_name)
        txt = KFileDialog::getSaveFileName(ksettings.lastDirectory, i18n("*.kgt|*.kgt\n*|All Files"));
    else
        txt = locateLocal("appdata", "transfers.kgt");


    writeTransfers(txt);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
*/
}


void Scheduler::writeTransfers(const QString & file)
{
/*

    if (file.isEmpty())
    {
#ifdef _DEBUG
        sDebugOut<< " because Destination File name isEmpty"<< endl;
#endif
        return;
    }
    if (!txt.endsWith(".kgt"))
        txt += ".kgt";

#ifdef _DEBUG
    sDebug << "Writing transfers " << txt << endl;
#endif
    //FIX
    transfers->writeTransfers(file);
*/
}

// destFile must be a filename, not a directory! And it will be deleted, if
// it exists already, without further notice.
Transfer * Scheduler::addTransferEx(const KURL& url, const KURL& destFile)
{

#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if ( !isValidURL( url ) )
        return 0;

    KURL destURL = destFile;

    // Malformed destination url means one of two things.
    // 1) The URL is empty.
    // 2) The URL is only a filename, like a default (suggested) filename.
    if ( !destURL.isValid() )
    {
        // Setup destination
        QString destDir = getSaveDirectoryFor( url.fileName() );
        bool b_expertMode = ksettings.b_expertMode;
        bool bDestisMalformed = true;

        while (bDestisMalformed)
        {
            if (!b_expertMode) {
                // open the filedialog for confirmation
                KFileDialog dlg( destDir, QString::null,
                                  0L, "save_as", true);
                dlg.setCaption(i18n("Save As"));
                dlg.setOperationMode(KFileDialog::Saving);

                // Use the destination name if not empty...
                if (destURL.isEmpty())
                    dlg.setSelection(url.fileName());
                else
                    dlg.setSelection(destURL.url());

                if ( dlg.exec() == QDialog::Rejected )
                {
#ifdef _DEBUG
                    sDebugOut << endl;
#endif
                    return 0;
                }
                else
                {
                    destURL = dlg.selectedURL();
                    ksettings.lastDirectory = destURL.directory();
                }
            }
            else {
                // in expert mode don't open the filedialog
                // if destURL is not empty, it's the suggested filename
                destURL = KURL::fromPathOrURL( destDir + "/" +
                                               ( destURL.isEmpty() ?
                                                   url.fileName() : destURL.url() ));
            }

            //check if destination already exists
            if(KIO::NetAccess::exists(destURL, false, mainWidget))
            {
                if (KMessageBox::warningYesNo(mainWidget,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()) )
                                              == KMessageBox::Yes)
                {
                    bDestisMalformed=false;
                    SafeDelete::deleteFile( destURL );
                }
                else
                {
                    b_expertMode=false;
                    ksettings.lastDirectory = destURL.directory();
                }
            }
            else
                bDestisMalformed=false;
        }
    }

    else // destURL was given, check whether the file exists already
    {
        // simply delete it, the calling process should have asked if you
        // really want to delete (at least khtml does)
        if(KIO::NetAccess::exists(destURL, false, mainWidget))
            SafeDelete::deleteFile( destURL );
    }

    // create a new transfer item
    return transfers->addTransfer(url, destURL);

    /*if (ksettings.b_useSound) {
        KAudioPlayer::play(ksettings.audioAdded);
    }
    checkQueue();
    */
#ifdef _DEBUG
    sDebugOut << endl;
#endif

}

bool Scheduler::isValidURL( const KURL& url )
{

    if (!url.isValid() || !KProtocolInfo::supportsReading( url ) )
    {
        if (!ksettings.b_expertMode)
            KMessageBox::error(mainWidget, i18n("Malformed URL:\n%1").arg(url.prettyURL()), i18n("Error"));

        return false;
    }
    // if we find this URL in the list
    Transfer *transfer = transfers->find( url );
    if ( transfer )
    {
        if ( transfer->getStatus() != Transfer::ST_FINISHED )
        {
            if ( !ksettings.b_expertMode )
            {
                KMessageBox::error(mainWidget, i18n("Already saving URL\n%1").arg(url.prettyURL()), i18n("Error"));
            }

            return false;
        }

        else // transfer is finished, ask if we want to download again
        {
            if ( ksettings.b_expertMode ||
                 (KMessageBox::questionYesNo(mainWidget, i18n("Already saved URL\n%1\nDownload again?").arg(url.prettyURL()), i18n("Question"))
                     == KMessageBox::Yes) )
            {
                transfer->slotRequestRemove();
                //checkQueue();
                return true;
            }
        }

        return false;
    }

    // why restrict this to ftp and http? (pfeiffer)
//     // don't download file URL's TODO : uncomment?
//     if (url.protocol() == "http" && url.protocol() != "ftp") {
//         KMessageBox::error(this, i18n("File protocol not accepted!\n%1").arg(url.prettyURL()), i18n("Error"));
// #ifdef _DEBUG
//         sDebugOut << endl;
// #endif
//         return false;
//     }

    return true;
}

bool Scheduler::isValidDest( const KURL& dest )
{
    //Copy from addTransferEx
}

QString Scheduler::getSaveDirectoryFor( const QString& filename ) const
{
    /**
     * first set destination directory to current directory 
     * ( which is also last used )
     */
     
    QString destDir = ksettings.lastDirectory;

    if (!ksettings.b_useLastDir) {
        // check wildcards for default directory
        DirList::Iterator it;
        for (it = ksettings.defaultDirList.begin(); 
             it != ksettings.defaultDirList.end(); ++it) 
             {
             QRegExp rexp((*it).extRegexp);

            rexp.setWildcard(true);

            if ((rexp.search( filename )) != -1) {
                destDir = (*it).defaultDir;
                break;
            }
        }
    }

    return destDir;
}


/*

void Scheduler::checkQueue()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    uint numRun = 0;
    int status;
    Transfer *item;

    if (!ksettings.b_offlineMode && b_online) {

        TransferIterator it(transfers);

        // count running transfers
        for (; it.current(); ++it) {
            status = it.current()->getStatus();
            if (status == Transfer::ST_RUNNING || status == Transfer::ST_TRYING)
                numRun++;
        }
        sDebug << "Found " << numRun << " Running Jobs" << endl;
        it.reset();
        bool isRunning;
        bool isQuequed;
        for (; it.current() && numRun < ksettings.maxSimultaneousConnections; ++it) {
            item = it.current();
            isRunning = (item->getStatus() == Transfer::ST_RUNNING) || (item->getStatus() == Transfer::ST_TRYING);

            isQuequed = (item->getMode() == Transfer::MD_QUEUED);

            if (!isRunning && isQuequed && !ksettings.b_offlineMode) {
                log(i18n("Starting another queued job."));
                item->slotResume();
                numRun++;
            }
        }

        slotUpdateActions();

        updateStatusBar();

    } else {
        log(i18n("Cannot continue offline status"));
    }


#ifdef _DEBUG
    sDebugOut << endl;
#endif

}

*/

/*

void Scheduler::slotMoveToBegin()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    transfers->moveToBegin((Transfer *) transfers->currentItem());


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotMoveToEnd()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    transfers->moveToEnd((Transfer *) transfers->currentItem());

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotOpenIndividual()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Transfer *item = (Transfer *) transfers->currentItem();
    if (item)
        item->showIndividual();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

*/

/*
void Scheduler::slotResumeCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(transfers);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotResume();
    slotUpdateActions();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotPauseCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(transfers);

    m_paPause->setEnabled(false);
    m_paRestart->setEnabled(false);
    update();

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestPause();
    slotUpdateActions();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}



void Scheduler::slotRestartCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(transfers);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestRestart();
    slotUpdateActions();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotDeleteCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    m_paDelete->setEnabled(false);
    m_paPause->setEnabled(false);
    update();
    TransferIterator it(transfers);
    QValueList<QGuardedPtr<Transfer> > selectedItems;
    QStringList itemNames;

    while (it.current()) {
        if (it.current()->isSelected()) {
            selectedItems.append( QGuardedPtr<Transfer>( it.current() ));
            itemNames.append( it.current()->getSrc().prettyURL() );
        }
        ++it;
    }

    if (!ksettings.b_expertMode)
    {
        if ( selectedItems.count() > 1 )
        {
            if (KMessageBox::questionYesNoList(this, i18n("Are you sure you want to delete these transfers?"),
                                               itemNames, i18n("Question"),
                                               KStdGuiItem::yes(),KStdGuiItem::no(),
                                               QString("multiple_delete_transfer"))
                != KMessageBox::Yes)
                return; // keep 'em
        }
        else
        {
            if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to delete this transfer?"),
                                           i18n("Question"), KStdGuiItem::yes(),KStdGuiItem::no(),
                                           QString("delete_transfer"))
                != KMessageBox::Yes)
                return;
        }
    }

    // If we reach this, we want to delete all selected transfers
    // Some of them might have finished in the meantime tho. Good that
    // we used a QGuardedPtr :)

    int transferFinishedMeanwhile = 0;
    QValueListConstIterator<QGuardedPtr<Transfer> > lit = selectedItems.begin();;
    while ( lit != selectedItems.end() )
    {
        if ( *lit )
            (*lit)->slotRequestRemove();
        else
            ++transferFinishedMeanwhile;

        ++lit;
    }

    checkQueue(); // needed !

    if ( !ksettings.b_expertMode && transferFinishedMeanwhile > 0 )
        KMessageBox::information(this, i18n("The transfer you wanted to delete completed before it could be deleted.",
                                            "%n transfers you wanted to delete completed before they could be deleted.",
                                            transferFinishedMeanwhile ),
                                 QString::null, "completedBeforeDeletion" );

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::pauseAll()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    log(i18n("Pausing all jobs"), false);

    TransferIterator it(transfers);
    Transfer::TransferStatus Status;
    for (; it.current(); ++it) {
        Status = it.current()->getStatus();
        if (Status == Transfer::ST_TRYING || Status == Transfer::ST_RUNNING)
            it.current()->slotRequestPause();
    }


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotQueueCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(transfers);

    for (; it.current(); ++it) {
        if (it.current()->isSelected()) {
            it.current()->slotQueue();
        }
    }

    //    transfers->clearSelection();
    slotUpdateActions();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotTimerCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif


    TransferIterator it(transfers);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestSchedule();

#ifdef _DEBUG
    sDebugOut << endl;
#endif

}


void Scheduler::slotDelayCurrent()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    TransferIterator it(transfers);

    for (; it.current(); ++it)
        if (it.current()->isSelected())
            it.current()->slotRequestDelay();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void Scheduler::slotTransferTimeout()
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    Transfer *item;
    TransferIterator it(transfers);

    bool flag = false;

    for (; it.current(); ++it) {
        item = it.current();
        if (item->getMode() == Transfer::MD_SCHEDULED && item->getStartTime() <= QDateTime::currentDateTime()) {
            item->setMode(Transfer::MD_QUEUED);
            flag = true;
        }
    }

    if (flag) {
        checkQueue();
    }

    // CONTROLLO AUTODISCONNESSIONE
    if (ksettings.b_autoDisconnect && ksettings.b_timedDisconnect && ksettings.disconnectTime <= QTime::currentTime() && ksettings.disconnectDate == QDate::currentDate())
    { // AUTODISCONNETTI }

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}
*/



/*
void Scheduler::slotStatusChanged(Transfer * item, int _operation)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif

    switch (_operation) {

    case Transfer::OP_FINISHED:
        if (ksettings.b_removeOnSuccess && !item->keepDialogOpen() )
        {
            delete item;
            item = 0L;
        }
        else
            item->setMode(Transfer::MD_NONE);

        if (transfers->isQueueEmpty()) {
            // no items in the TransferList or we have donwload all items
            // CONTROLLO AUTODISCONNESSIONE
            if (ksettings.b_autoDisconnect)
                {}//AUTODISCONNETTI

            if (ksettings.b_autoShutdown) {
                slotQuit();
                return;
            }
            // play(ksettings.audioFinishedAll);
        }

        if ( item )
            item->slotUpdateActions();

        break;

    case Transfer::OP_RESUMED:
        slotUpdateActions();
        item->slotUpdateActions();
        //                play(ksettings.audioStarted);
        break;
    case Transfer::OP_PAUSED:
        break;
    case Transfer::OP_REMOVED:
        delete item;
        return;                 // checkQueue() will be called only once after all deletions

    case Transfer::OP_ABORTED:
        break;
    case Transfer::OP_DELAYED:
    case Transfer::OP_QUEUED:
        slotUpdateActions();
        item->slotUpdateActions();
        break;
    case Transfer::OP_SCHEDULED:
        slotUpdateActions();
        item->slotUpdateActions();
        slotTransferTimeout();  // this will check schedule times
        return;                 // checkQueue() is called from slotTransferTimeout()
    }
    checkQueue();

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}
*/

#include "scheduler.moc"
