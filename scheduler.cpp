#include "scheduler.h"

//temporary: fixes some of missing includes
#include "kmainwidget.h"
#include <kfiledialog.h>

Scheduler::Scheduler()
{

}

Scheduler::~Scheduler()
{

}

void Scheduler::slotNewURLs(const KURL::List & src, const QString& destDir)
{
    KURL::List urlsToDownload;

    for ( KURL::List::ConstIterator it = src.begin(); it != src.end(); ++it )
    {
        KURL url = *it;
        if ( url.fileName().endsWith( ".kgt" ) )
            readTransfers(url);
        else
            urlsToDownload.append( url );
    }

    if ( urlsToDownload.isEmpty() )
        return;

    if ( urlsToDownload.count() == 1 ) // just one file -> ask for filename
    {
        KURL destFile;

        if ( !destDir.isEmpty() )
        {
            // create a proper destination file from destDir
            KURL destURL = KURL::fromPathOrURL( destDir );
            QString fileName = urlsToDownload.first().fileName();

            // in case the fileName is empty, we simply ask for a filename in
            // addTransferEx. Do NOT attempt to use an empty filename, that
            // would be a directory (and we don't want to overwrite that!)
            if ( !fileName.isEmpty() )
            {
                destURL.adjustPath( +1 );
                destURL.setFileName( fileName );
                if(KIO::NetAccess::exists(destURL, false, this))
                {
                    if (KMessageBox::warningYesNo(this,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()) )
                        == KMessageBox::Yes)
                    {
                        SafeDelete::deleteFile( destURL );
                        destFile = destURL;
                    }
                }
            }
        }

        addTransferEx( urlsToDownload.first(), destFile );
        return;
    }

    // multiple files -> ask for directory, not for every single filename
    KURL dest;
    if ( destDir.isEmpty() || !QFileInfo( destDir ).isDir() )
    {
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

    // create new transfer items
    KURL::List::ConstIterator it = urlsToDownload.begin();
    for ( ; it != urlsToDownload.end(); ++it )
    {
        KURL srcURL = *it;

        if ( !sanityChecksSuccessful( *it ) )
            continue;

        KURL destURL = dest;
        QString fileName = (*it).fileName();
        if ( fileName.isEmpty() ) // simply use the full url as filename
            fileName = KURL::encode_string_no_slash( (*it).prettyURL() );

        destURL.setFileName( fileName );

        if(KIO::NetAccess::exists(destURL, false, this))
        {
            if (KMessageBox::warningYesNo(this,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL() ) )
                                          == KMessageBox::Yes)
            {
                SafeDelete::deleteFile( destURL );
            }
        }

        Transfer *item = myTransferList->addTransfer(*it, destURL);
        item->updateAll(); // update the remaining fields
    }

    myTransferList->clearSelection();

    if (ksettings.b_useSound) {
        KAudioPlayer::play(ksettings.audioAdded);
    }
    checkQueue();

}

void Scheduler::slotNewURL(const KURL & src, const QString& destDir)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if ( src.isEmpty() )
        return;

    addTransferEx( KURL::fromPathOrURL( src ) );

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void Scheduler::slotSetOperation(QValueList<Transfer *>, enum Operation)
{
    
}

/*
void Scheduler::slotPasteTransfer()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    QString newtransfer;

    newtransfer = QApplication::clipboard()->text();
    newtransfer = newtransfer.stripWhiteSpace();

    if (!ksettings.b_expertMode) {
        bool ok = false;
        newtransfer = KInputDialog::getText(i18n("Open Transfer"), i18n("Open transfer:"), newtransfer, &ok, this);

        if (!ok) {
            // cancelled
#ifdef _DEBUG
            sDebugOut << endl;
#endif
            return;
        }

    }

    if (!newtransfer.isEmpty())
        scheduler->addTransfer(newtransfer);


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}
*/

/*

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

    readTransfers(url);

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void Scheduler::readTransfers(const KURL & file)
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
    myTransferList->readTransfers(file);
    checkQueue();
    slotTransferTimeout();
    myTransferList->clearSelection();


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

void Scheduler::slotExportTransfers(bool ask_for_name)
{
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
}


void Scheduler::writeTransfers(const QString & file)
{

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
    myTransferList->writeTransfers(file);

}

// destFile must be a filename, not a directory! And it will be deleted, if
// it exists already, without further notice.
void Scheduler::addTransferEx(const KURL& url, const KURL& destFile)
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    if ( !sanityChecksSuccessful( url ) )
        return;

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
                    return;
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
                                               ( destURL.isEmpty() 
                                                   url.fileName() : destURL.url() ));
            }

            //check if destination already exists
            if(KIO::NetAccess::exists(destURL, false, this))
            {
                if (KMessageBox::warningYesNo(this,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()) )
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
        if(KIO::NetAccess::exists(destURL, false, this))
            SafeDelete::deleteFile( destURL );
    }

    // create a new transfer item
    Transfer *item = myTransferList->addTransfer(url, destURL);
    item->updateAll(); // update the remaining fields

    if (ksettings.b_showIndividual)
        item->showIndividual();

    myTransferList->clearSelection();

    if (ksettings.b_useSound) {
        KAudioPlayer::play(ksettings.audioAdded);
    }
    checkQueue();
#ifdef _DEBUG
    sDebugOut << endl;
#endif
}

bool Scheduler::isValidUrl( const KURL& url )
{
    if (!url.isValid() || !KProtocolInfo::supportsReading( url ) )
    {
        if (!ksettings.b_expertMode)
            KMessageBox::error(this, i18n("Malformed URL:\n%1").arg(url.prettyURL()), i18n("Error"));

        return false;
    }
    // if we find this URL in the list
    Transfer *transfer = myTransferList->find( url );
    if ( transfer )
    {
        if ( transfer->getStatus() != Transfer::ST_FINISHED )
        {
            if ( !ksettings.b_expertMode )
            {
                KMessageBox::error(this, i18n("Already saving URL\n%1").arg(url.prettyURL()), i18n("Error"));
            }

            transfer->showIndividual();
            return false;
        }

        else // transfer is finished, ask if we want to download again
        {
            if ( ksettings.b_expertMode ||
                 (KMessageBox::questionYesNo(this, i18n("Already saved URL\n%1\nDownload again?").arg(url.prettyURL()), i18n("Question"))
                     == KMessageBox::Yes) )
            {
                transfer->slotRequestRemove();
                checkQueue();
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

*/

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

        TransferIterator it(myTransferList);

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

    myTransferList->moveToBegin((Transfer *) myTransferList->currentItem());


#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotMoveToEnd()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    myTransferList->moveToEnd((Transfer *) myTransferList->currentItem());

#ifdef _DEBUG
    sDebugOut << endl;
#endif
}


void Scheduler::slotOpenIndividual()
{
#ifdef _DEBUG
    sDebugIn << endl;
#endif

    Transfer *item = (Transfer *) myTransferList->currentItem();
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

    TransferIterator it(myTransferList);

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

    TransferIterator it(myTransferList);

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

    TransferIterator it(myTransferList);

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
    TransferIterator it(myTransferList);
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

    TransferIterator it(myTransferList);
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

    TransferIterator it(myTransferList);

    for (; it.current(); ++it) {
        if (it.current()->isSelected()) {
            it.current()->slotQueue();
        }
    }

    //    myTransferList->clearSelection();
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


    TransferIterator it(myTransferList);

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

    TransferIterator it(myTransferList);

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
    TransferIterator it(myTransferList);

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

    if (ksettings.b_autoDisconnect && ksettings.b_timedDisconnect && ksettings.disconnectTime <= QTime::currentTime() && ksettings.disconnectDate == QDate::currentDate()) {
        onlineDisconnect();
    }

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}
*/


/*
void Scheduler::customEvent(QCustomEvent * _e)
{
#ifdef _DEBUG
    //sDebugIn << endl;
#endif


    SlaveEvent *e = (SlaveEvent *) _e;
    unsigned int result = e->getEvent();

    switch (result) {

        // running cases..
    case Slave::SLV_PROGRESS_SIZE:
        e->getItem()->slotProcessedSize(e->getData());
        break;
    case Slave::SLV_PROGRESS_SPEED:
        e->getItem()->slotSpeed(e->getData());
        break;

    case Slave::SLV_RESUMED:
        e->getItem()->slotExecResume();
        break;

        // stopping cases
    case Slave::SLV_FINISHED:
        e->getItem()->slotFinished();
        break;
    case Slave::SLV_PAUSED:
        e->getItem()->slotExecPause();
        break;
    case Slave::SLV_SCHEDULED:
        e->getItem()->slotExecSchedule();
        break;

    case Slave::SLV_DELAYED:
        e->getItem()->slotExecDelay();
        break;
    case Slave::SLV_CONNECTED:
        e->getItem()->slotExecConnected();
        break;

    case Slave::SLV_CAN_RESUME:
        e->getItem()->slotCanResume((bool) e->getData());
        break;

    case Slave::SLV_TOTAL_SIZE:
        e->getItem()->slotTotalSize(e->getData());
        break;

    case Slave::SLV_ERROR:
        e->getItem()->slotExecError();
        break;

    case Slave::SLV_BROKEN:
        e->getItem()->slotExecBroken();
        break;

    case Slave::SLV_REMOVED:
        e->getItem()->slotExecRemove();
        break;

    case Slave::SLV_INFO:
        e->getItem()->logMessage(e->getMsg());
        break;
    default:
#ifdef _DEBUG
        sDebug << "Unknown Result..die" << result << endl;
#endif
        assert(0);
    }
}
*/

#ifdef _DEBUG
    //sDebugOut << endl;
#endif
}

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

        if (myTransferList->isQueueEmpty()) {
            // no items in the TransferList or we have donwload all items
            if (ksettings.b_autoDisconnect)
                onlineDisconnect();

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
