#include <qregexp.h>

#include <kurl.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <kinputdialog.h>
#include <knotifyclient.h>

#include "scheduler.h"
#include "connection.h"
#include "safedelete.h"
#include "settings.h"
#include "kmainwidget.h"

Scheduler::Scheduler(KMainWidget * _mainWidget)
    : QObject(), mainWidget(_mainWidget),
    running(false)
{
    transfers = new TransferList();
    runningTransfers = new TransferList();
    removedTransfers = new TransferList();
    connections.append( new Connection(this) );
}

Scheduler::~Scheduler()
{

}

void Scheduler::run()
{
    running = true;
    queueUpdate();
}

void Scheduler::stop()
{
    running = false;
    
    TransferList::iterator it = runningTransfers->begin();
    TransferList::iterator endList = runningTransfers->end();
    
    while(it != endList)
        {
        (*it)->slotStop();
        ++it;
    }
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
            slotImportTransfers(url);
        else
            urlsToDownload.append( url );
    }

    //sDebug << "BBB" << endl;
    
    if ( urlsToDownload.isEmpty() )
        return;

    //sDebug << "CCC" << endl;
    KURL dest;
    
    if ( urlsToDownload.count() == 1 )
        {
        // just one file -> ask for filename
        slotNewURL(src.first(), destDir);
        return;
    }
        
    // multiple files -> ask for directory, not for every single filename
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
            Settings::setLastDirectory( dir );
        }
    }

    // dest is now finally the real destination directory for all the files
    dest.adjustPath(+1);

    TransferList list;
    
    // create new transfer items
    KURL::List::ConstIterator it = urlsToDownload.begin();
    for ( ; it != urlsToDownload.end(); ++it )
    {
        KURL srcURL = *it;
        sDebug << ">>>>>>>>>>>>>>>  " << srcURL.url() << endl;
        if ( !isValidURL( srcURL ) )
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
            /*
            KIO::open_RenameDlg(i18n("File already exists"), 
                (*it).url(), destURL.url(),
                KIO::M_MULTI);
            */
        }

        Transfer *item = new Transfer(this, *it, destURL);
        list.addTransfer(item);
    }

    transfers->addTransfers(list);
    
    emit addedItems(list);
    
    queueAddedItems(list);

    KNotifyClient::event( mainWidget->winId(), "added" );
        
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
            
            newtransfer = KInputDialog::getText(i18n("New Download"), i18n("Enter URL:"), newtransfer, &ok, mainWidget);
    
            // user presses cancel
            if (!ok) 
                {
                return;
            }
    
            src = KURL::fromPathOrURL(newtransfer);
    
            if (!src.isValid())
                {
                sDebug << "hhh" << endl;
                KMessageBox::error(mainWidget, i18n("Malformed URL:\n%1").arg(newtransfer), i18n("Error"));
                ok = false;
                sDebug << "ggg" << endl;
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
    
    TransferList list(item);
       
    emit addedItems(list);

    queueAddedItems(list);
        
    sDebugOut << endl;
}

void Scheduler::slotRemoveItems(TransferList & list)
{
    transfers->removeTransfers(list);
    removedTransfers->addTransfers(list);
    
    emit removedItems(list);
    
    queueRemovedItems(list);
}

void Scheduler::slotRemoveItems(Transfer * item)
{
    transfers->removeTransfer(item);
    removedTransfers->addTransfer(item);
    
    TransferList list(item);
    
    emit removedItems(list);

    queueRemovedItems(list);
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
    
    TransferList list(item);
    
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

bool Scheduler::slotSetCommand(Transfer * item, TransferCommand op)
{
    sDebugIn << endl;

    switch (op)
        {
        case CmdResume:
                sDebug << "111 ->" << runningTransfers->size() << endl;
                if(  (item->getStatus() == ST_STOPPED) 
                  && (item->getPriority()!=6)
                  && (1/*Settings::maxSimConnections()*/ > runningTransfers->size()))
                    {
                    sDebug << "222" << endl;
                    if(item->slotResume())
                        {
                        runningTransfers->addTransfer(item);
                        return true;
                    } 
                }
                sDebug << "333" << endl;
                return false;
        case CmdRestart:
                item->slotRetransfer();
                return false; //temporary
        case CmdPause:
                runningTransfers->removeTransfer(item);
                item->slotStop();
                return true;
    }
    sDebugOut << endl;
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

    TransferList list(item);
    
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
//     if (!Settings::expertMode()) {
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
//     KURL filename = KFileDialog::getOpenURL(Settings::lastDirectory());
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
    TransferList list(item);
    
    switch (msg)
        {
        case MSG_FINISHED:
        case MSG_REMOVED:
        case MSG_PAUSED:
        case MSG_ABORTED:
        case MSG_DELAYED:
            sDebug << "TRANSFER REMOVAL: " << item << endl;
            runningTransfers->removeTransfer(item);
            removedTransfers->addTransfer(item);
            queueUpdate();
            //TODO Here we should set some properties to the transfers
            //that we can't download. In this way we don't continue to try 
            //to download files that get a error
            
            break;
        case MSG_DELAY_FINISHED:
            queueUpdate();
            break;
        case MSG_RESUMED:
            
            break;
            
    }
       
    emit changedItems(list);
    sDebugOut << endl;    
}


void Scheduler::slotImportTransfers(bool ask_for_name)
{
    sDebugIn << endl;

    KURL url;

    if (ask_for_name)
        url = KFileDialog::getOpenURL(Settings::lastDirectory(), i18n("*.kgt|*.kgt\n*|All Files"));
    else
        url.setPath( locateLocal("appdata", "transfers.kgt") );

    slotImportTransfers(url);

    sDebugOut << endl;
}

void Scheduler::slotImportTransfers(const KURL & file)
{
  sDebugIn << endl;

    if (!file.isValid()) {

        sDebugOut<< " string empty" << endl;
        return;
    }
    sDebug << "Read from file: " << file << endl;
    transfers->readTransfers(file, this);
    //checkQueue(); <--- TO BE ENABLED
    
    
    //slotTransferTimeout();
    //transfers->clearSelection();

    emit addedItems(*transfers);

    sDebugOut << endl;
}

void Scheduler::slotExportTransfers(bool ask_for_name)
{
    sDebugIn << endl;

    QString str;
    QString txt;

    if (ask_for_name)
        txt = KFileDialog::getSaveFileName(Settings::lastDirectory(), i18n("*.kgt|*.kgt\n*|All Files"));
    else
        txt = locateLocal("appdata", "transfers.kgt");


    slotExportTransfers(txt);

    sDebugOut << endl;
}

void Scheduler::slotExportTransfers(QString & file)
{
    sDebugIn << endl;
    
    if (file.isEmpty())
    {
        sDebugOut<< " because Destination File name isEmpty"<< endl;
        return;
    }
    if (!file.endsWith(".kgt"))
        file += ".kgt";

    transfers->writeTransfers(file);

    sDebugOut << endl;
}

// destFile must be a filename, not a directory! And it will be deleted, if
// it exists already, without further notice.
Transfer * Scheduler::addTransferEx(const KURL& url, const KURL& destFile)
{
    sDebugIn << endl;

    if ( !isValidURL( url ) )
        return 0;

    sDebug << "aaa" << endl;
        
    KURL destURL = getValidDest(url.fileName(), destFile);
    if(destURL.isEmpty())
        return 0;
    
    sDebug << "bbb" << endl;

    // simply delete it, the calling process should have asked if you
    // really want to delete (at least khtml does)
    if(KIO::NetAccess::exists(destURL, false, mainWidget))
        SafeDelete::deleteFile( destURL );

    sDebug << "ccc" << endl;

    // create a new transfer item
    Transfer * t = new Transfer(this, url, destURL);
    transfers->addTransfer(t);
    
    return t;

    sDebugOut << endl;
}

bool Scheduler::isValidURL( KURL url )
{
    sDebugIn << endl;

    if (!url.isValid())
    {
        if (!Settings::expertMode())
            KMessageBox::error(mainWidget, i18n("Malformed URL:\n%1").arg(url.prettyURL()), i18n("Error"));
        sDebug << "##########" << endl;
        return false;
    }
    // if we find this URL in the list
    Transfer *transfer = transfers->find( url );
    if ( transfer )
    {
        if ( transfer->getStatus() != ST_FINISHED )
        {
            if ( !Settings::expertMode() )
            {
                KMessageBox::error(mainWidget, i18n("Already saving URL\n%1").arg(url.prettyURL()), i18n("Error"));
            }

            return false;
        }

        else // transfer is finished, ask if we want to download again
        {
            if ( Settings::expertMode() ||
                 (KMessageBox::questionYesNo(mainWidget, i18n("Already saved URL\n%1\nDownload again?").arg(url.prettyURL()), i18n("Question"))
                     == KMessageBox::Yes) )
            {
                transfer->slotRemove();
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

    sDebugOut << endl;
    
    return true;
}

KURL Scheduler::getValidDest( const QString& filename, const KURL& dest)
{
    // Malformed destination url means one of two things.
    // 1) The URL is empty.
    // 2) The URL is only a filename, like a default (suggested) filename.

    
    KURL destURL = dest;
    
    if ( !destURL.isValid() )
    {
        // Setup destination
        QString destDir = getSaveDirectoryFor( filename );
        bool b_expertMode = Settings::expertMode();
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
                    dlg.setSelection(filename);
                else
                    dlg.setSelection(destURL.url());

                if ( dlg.exec() == QDialog::Rejected )
                {
                    sDebugOut << endl;
                    return KURL(); //return 0
                }
                else
                {
                    destURL = dlg.selectedURL();
                    Settings::setLastDirectory( destURL.directory() );
                }
            }
            else {
                // in expert mode don't open the filedialog
                // if destURL is not empty, it's the suggested filename
                destURL = KURL::fromPathOrURL( destDir + "/" +
                                               ( destURL.isEmpty() ?
                                                   filename : destURL.url() ));
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
                    Settings::setLastDirectory( destURL.directory() );
                }
            }
            else
                bDestisMalformed=false;
        }
    }
    return destURL;
}

QString Scheduler::getSaveDirectoryFor( const QString& filename ) const
{
    /**
     * first set destination directory to current directory 
     * ( which is also last used )
     */
     
    QString destDir = Settings::lastDirectory();

    if (!Settings::useLastDirectory()) {
        // check wildcards for default directory

        QStringList::Iterator it = Settings::mimeDirList().begin();
        QStringList::Iterator end = Settings::mimeDirList().end();
        for (; it != end; ++it)
        {
            // odd list items are regular expressions
            QRegExp rexp(*it);
            rexp.setWildcard(true);
            // even list items are directory
            ++it;

            if ((rexp.search( filename )) != -1) {
                destDir = *it;
                break;
            }
        }
    }

    return destDir;
}

void Scheduler::queueAddedItems(TransferList & list)
{
    if(!running)
        return;
        
    TransferList toRemove;
    TransferList toAdd;
        
    TransferList::iterator it1 = runningTransfers->begin();
    TransferList::iterator endList1 = runningTransfers->end();
    
    TransferList::iterator it2;
    TransferList::iterator endList2 = list.end();
    
    for( ; it1 != endList1; ++it1)
        {
        for(it2=list.begin(); it2!=endList2; ++it2 )
            {
            if ( ((Transfer *) *it2) < ((Transfer *) *it1) 
               &&((Transfer *) *it1)->getCanResume() )
                {
                toRemove.addTransfer(*it1);
                toAdd.addTransfer(*it2);
            }
        }
    }
    slotSetCommand(toRemove, CmdPause);
    slotSetCommand(toAdd, CmdResume);
}

void Scheduler::queueRemovedItems(TransferList & list)
{
    if(!running)
        return;

    TransferList toRemove;
        
    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();
    
    for(; it!=endList; ++it )
        {
        if (runningTransfers->contains(*it))
            {
            toRemove.addTransfer(*it);
        }
    }
    slotSetCommand(toRemove, CmdPause);
}

void Scheduler::queueUpdate()
{
    sDebugIn << endl;

    if(!running)
        return;
    
    sDebug << "(1)" << endl;
                
    int newTransfers = 1/*Settings::maxSimConnections()*/ - runningTransfers->size();

    sDebug << "(2) -> " << 1/*Settings::maxSimConnections()*/ << " : " << runningTransfers->size() << endl;
        
    if(newTransfers <= 0 )
        return;
                   
    //search for the next transfer in the list to be downloaded
    TransferList::iterator it = transfers->begin();
    TransferList::iterator endList = transfers->end();
    
    while(newTransfers > 0 && it != endList)
        {
        sDebug << "AAA" << endl;
        if(slotSetCommand(*it, CmdResume))
            {
            sDebug << "BBB" << endl;
            --newTransfers;
        }
        ++it;
    }
    
    sDebugOut << endl;
}

/*

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

    if (!Settings::expertMode())
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

    if ( !Settings::expertMode() && transferFinishedMeanwhile > 0 )
        KMessageBox::information(this, i18n("The transfer you wanted to delete completed before it could be deleted.",
                                            "%n transfers you wanted to delete completed before they could be deleted.",
                                            transferFinishedMeanwhile ),
                                 QString::null, "completedBeforeDeletion" );

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}
*/


#include "scheduler.moc"
