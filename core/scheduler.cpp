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

#include <qregexp.h>
#include <qtimer.h>

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
#include "group.h"
#include "connection.h"
#include "safedelete.h"

#include "conf/settings.h"
#include "transfers/kio/transferKio.h"
#include "kget.h"

Scheduler::Scheduler(KMainWidget * _mainWidget)
    : QObject(), mainWidget(_mainWidget),
    running(false)
{
    transfers = new TransferList();
    runningTransfers = new TransferList();
    removedTransfers = new TransferList();
    groups = new GroupList();
    
    connect(this,   SIGNAL(addedItems(const TransferList&)),
            groups, SLOT(slotAddedTransfers(const TransferList&)));
    connect(this,   SIGNAL(removedItems(const TransferList&)),
            groups, SLOT(slotRemovedTransfers(const TransferList&)));
    connect(this,   SIGNAL(changedItems(const TransferList&)),
            groups, SLOT(slotChangedTransfers(const TransferList&)));
    connect(groups, SIGNAL(changedGroups(const GroupList&)),
            this,   SIGNAL(changedGroups(const GroupList&)));
    
    connections.append( new Connection(this) );
}

Scheduler::~Scheduler()
{
    sDebugIn << endl;
    slotExportTransfers();
    sDebugOut << endl;
}


void Scheduler::run()
{
    running = true;
    queueUpdate();
}

void Scheduler::stop()
{
    running = false;
    
    TransferList::iterator it;
    //TransferList::iterator it = runningTransfers->begin();
    //TransferList::iterator endList = runningTransfers->end();
    
    //This line can sound strange.. But since the slotStop call triggers
    //the deletion of the transfer from the runningTransfers list we can't
    //use the iterator in the usual way
    while( (it=runningTransfers->begin()) != runningTransfers->end())
    {
        (*it)->slotStop();
        ++it;
    }
}

void Scheduler::slotNewURLs(const KURL::List & src, const QString& destDir)
{
    sDebugIn << endl;
    
    KURL::List urlsToDownload;

    KURL::List::ConstIterator it = src.begin();
    KURL::List::ConstIterator itEnd = src.end();
    
    for ( ; it != itEnd; ++it )
    {
        ///sDebug << "AAA" << endl;

        KURL url = *it;
        if ( url.fileName().endsWith( ".kgt", false ) )
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
            //FIXME files could be divided using extension, can't take the
            //heading one only
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
    it = urlsToDownload.begin();
    itEnd = urlsToDownload.end();
    
    for ( ; it != itEnd; ++it )
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

        Transfer *item = new TransferKio(this, *it, destURL);
        list.addTransfer(item);
    }

    transfers->addTransfers(list);
    
    emit addedItems(list);
    
    queueEvaluateItems(list);
    queueUpdate();
    
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
                KMessageBox::error(mainWidget, i18n("Malformed URL:\n%1").arg(newtransfer));
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

    queueEvaluateItems(list);
    queueUpdate();    
    
    sDebugOut << endl;
}

void Scheduler::slotRemoveItems(TransferList list)
{
    transfers->removeTransfers(list);
    removedTransfers->addTransfers(list);

    queueRemovedItems(list);
        
    emit removedItems(list);
}

void Scheduler::slotSetPriority(TransferList list, int priority)
{
    sDebugIn << endl;
    
    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();
    
    for(;it != endList; it++)
    {
        (*it)->setPriority(priority);
        
        // Here we remove and insert the items to keep the list sorted
        transfers->removeTransfer(*it);
        transfers->addTransfer(*it, true);
    }
    
    emit changedItems(list);
    
    queueEvaluateItems(list);
    
    sDebugOut << endl;
}

void Scheduler::slotSetCommand(TransferList list, TransferCommand op)
{
    sDebugIn << endl;

    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();
    
    //for(; it!=endList && setTransferCommand(*it, op); ++it);
    
    switch (op)
        {
        case CmdResume:
            sDebug << "      RESUME        " << endl;
            slotSetPriority(list, 1);
            queueEvaluateItems(list, true);
            queueUpdate();
            break;   
        case CmdPause:
            sDebug << "      PAUSE        " << endl;
            setTransferCommand(list, CmdPause);
            queueUpdate();
        case CmdRestart:
            ;
                        
    }
    sDebugOut << endl;
}


void Scheduler::slotSetGroup(TransferList list, const QString& g)
{
    TransferList::iterator it;
    TransferList::iterator endList = list.end();
    
    for(it = list.begin(); it != endList; ++it)
    {
        //FIXME(*it)->setGroup(groupNumber);
    }
    emit changedItems(list);
}

void Scheduler::slotAddGroup(GroupList l)
{
    l.about();
    
    groups->addGroups(l);
    
    kdDebug() << "Scheduler: added group!!!!" << endl;
    
    groups->about();
    
    emit addedGroups(l);
}

void Scheduler::slotDelGroup(GroupList l)
{
    groups->delGroups(l);
    
    kdDebug() << "Scheduler: removed group!!!!" << endl;
    
    emit removedGroups(l);
}

void Scheduler::slotModifyGroup(const QString& n, Group g)
{
    groups->modifyGroup(n, g);
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
            slotImportTransfers(true);
            break;

        case OpExportTransfers:
            slotExportTransfers(true);
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

void Scheduler::slotReqOperation(SchedulerDebugOp operation)
{
    switch (operation)
        {
        case OpPrintTransferList:
            kdDebug() << "  ***  PRINTING LIST  ***  " << endl;
            transfers->about();
            break;
    }
}

void Scheduler::slotTransferChanged(Transfer * item)
{
//    sDebugIn << endl;    
    TransferList list(item);
    
    Transfer::TransferChanges transferFlags = item->changesFlags(this);
    item->resetChangesFlags(this);
     
    if(transferFlags & Transfer::Tc_Status)
    {
        switch (item->info().status)
        {
//            case Transfer::St_Delayed:
            case Transfer::St_Aborted:
                item->slotSetDelay(5);
            case Transfer::St_Finished:
            case Transfer::St_Stopped:
                sDebug << "TRANSFER REMOVAL: " << item << endl;
                runningTransfers->removeTransfer(item);
                queueUpdate();
                break;
            /*case MSG_DELAY_FINISHED:
                queueUpdate();
                break;
            */
            case Transfer::St_Running:
                break;
        }
    }
    
    //TODO: OPTIMIZE IF POSSIBLE
    if( transfers->contains(item) )
    {
        emit changedItems(list);
    }
//    sDebugOut << endl;    
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
    
    GroupList newGroups;
    TransferList newTransfers;
    
    newTransfers.readTransfers(file.url(), this, &newGroups);

    transfers->addTransfers(newTransfers);
    groups->addGroups(newGroups);
    
//     kdDebug() << "SIZE:" << groups->size() << endl;
    
    queueUpdate();
    
    emit addedGroups(newGroups);
    emit addedItems(newTransfers);

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
    if (!file.endsWith(".kgt", false))
        file += ".kgt";

    transfers->writeTransfers(file, this);

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
    Transfer * t = new TransferKio(this, url, destURL);
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
        if ( transfer->info().status != Transfer::St_Finished )
        {
            if ( !Settings::expertMode() )
            {
                KMessageBox::error(mainWidget, i18n("Already saving URL\n%1").arg(url.prettyURL()), i18n("Error"));
            }

            return false;
        }

        else // transfer is finished, ask if we want to download again
        {
            if (KMessageBox::questionYesNo(mainWidget,
                                           i18n("URL already saved:\n%1\nDownload again?").arg(url.prettyURL()),
                                           i18n("Download URL Again?"), KStdGuiItem::yes(),
                                           KStdGuiItem::no(), "QuestionURLAlreadySaved" )
                == KMessageBox::Yes)
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

    
    KURL destURL( dest );
    
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
     
//    for ( uint i = 0; i < Settings::mimeDirList().count(); i++ )
//        sDebug << ">>> " << Settings::mimeDirList()[i] << endl;
     
    if (Settings::useDefaultDirectory()) {
        // check wildcards for default directory

// damn iterators! they are causing - al least to me - some problems,
// so i'm disabling them...
//        QStringList::Iterator it = Settings::mimeDirList().begin();
//        QStringList::Iterator end = Settings::mimeDirList().end();
//        for (; it != end; ++it)
        for ( uint i = 0; i < Settings::mimeDirList().count(); i+=2 )
        {
            sDebug << ">>> " << Settings::mimeDirList()[i] << endl;
            // odd list items are regular expressions
//            QRegExp rexp( ( *it ) );
            QRegExp rexp(Settings::mimeDirList()[i]);
            rexp.setWildcard(true);
            // even list items are directory
//            ++it;

            if ((rexp.search( filename )) != -1) {
//                destDir = *it;
//                break;
                sDebug << ">>>>>> " << Settings::mimeDirList()[i+1] << endl;
                return Settings::mimeDirList()[i+1];
            }
        }

        // no specific directory found for this file, using the default one
        sDebug << ">>>>>> Using default dir" << endl;
        return Settings::defaultDirectory();
    }

    // we're in last directory mode
    sDebug << ">>>>>> Using last dir" << endl;
    return Settings::lastDirectory();
}

/*
These could be new functions that should be much simpler and will(?) replace
the existing ones.

typedef struct {
    KURL fromURL;
    QString toDir;
} dlPaths;

// connect (via the ViewInterfaceConnector) the slotNewURL() to this function
void Scheduler::addNewEmptyURL()
{
    // here kget should:
    // a) prompt the user, using the input dialog, a new download
    bool ok;
    QString newtransfer = KInputDialog::getText(i18n("New Download"),
                                                i18n("Enter URL:"), "", &ok,
                                                mainWidget);
    if (!ok)
        return;

    // b) checking if the URL is valid
    KURL from( newtransfer );
    if ( !isValidURL( from ) )
        return;

    // c) get the right dest folder for that
    QString dest = getDestFolderFor( from );

    // d) scheduling the new tansfer
    dlPaths newdl;
    newdl.fromURL = from;
    newdl.toDir = 
    if ( !schedNewTransfer( newdl ) )
        return;

}


bool Scheduler::schedNewTransfer( dlPaths newdl )
{
    // - schedule the download represented by the scruct newdl
    // - returns true if we added succesfully the new dl, false otherwise

}


*/

//BEGIN Private low level queue commands implementation

bool Scheduler::setTransferCommand(TransferList list, TransferCommand op)
{
    TransferList::iterator it = list.begin();
    TransferList::iterator endList = list.end();

    for(; it!=endList; ++it)
        setTransferCommand(*it, op);

    return true;
}

bool Scheduler::setTransferCommand(Transfer * item, TransferCommand op)
{
    switch (op)
    {
        case CmdResume:
            sDebug << "Scheduler::setTransferCommand() -> CmdResume" << endl;
            if(  (item->info().status != Transfer::St_Running)
                && (item->info().status != Transfer::St_Finished)
                && (item->info().status != Transfer::St_Delayed)
                && (item->info().priority != 6)
                && (Settings::maxConnections() > runningTransfers->size()) )
                {
                    if(item->slotResume())
                    {
                        runningTransfers->addTransfer(item);
                        return true;
                    } 
                }
                return false;
        case CmdRestart:
                sDebug << "Scheduler::setTransferCommand() -> CmdRestart" << endl;
                item->slotRetransfer();
                return false; //temporary
        case CmdPause:
                sDebug << "Scheduler::setTransferCommand() -> CmdPause" << endl;
                runningTransfers->removeTransfer(item);
                item->slotStop();
                return true;
    }
    return false;
}

void Scheduler::queueEvaluateItems(TransferList list, bool force)
{
    sDebugIn << endl;
    
    if( (!running) || list.empty() || runningTransfers->empty() )
        return;
        
    TransferList toRemove;
    TransferList toAdd;
        
    TransferList::iterator it1 = runningTransfers->fromLast();
    TransferList::iterator beginList1 = runningTransfers->begin();
    
    TransferList::iterator it2 = list.begin();
    TransferList::iterator endList2 = list.end();
    
    bool isPrior;
    
    do
        {
        sDebug << "(1)" << endl;
        if(force)
            isPrior = **it2 <= **it1;
        else
            isPrior = **it2 < **it1;
        
        if (isPrior)
        {
            sDebug << "(2)" << endl;
            if(!runningTransfers->contains(*it2))
            {
                toRemove.addTransfer(*it1);
                toAdd.addTransfer(*it2);
                it1--;
                it2++;
            }
            else
            {
                it1--;
            }
        }
    }
    while  ( (isPrior)
          && (it1 != beginList1 ) 
          && (it2 != endList2) );

    sDebug << "(3)" << endl;

    toAdd.about();
    toRemove.about();

/*    //Here we must remove the items that are in both the toRemove and 
    //the toAdd list.
    TransferList intersection = toAdd.intersect(toRemove);
    toAdd.removeTransfers(intersection);
    toRemove.removeTransfers(intersection);*/

    setTransferCommand(toRemove, CmdPause);
    setTransferCommand(toAdd, CmdResume);

    sDebugOut << endl;
}

void Scheduler::queueRemovedItems(TransferList list)
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
    if(!running)
        return;

    int newTransfers = Settings::maxConnections() - runningTransfers->size();

    kdDebug() << "Scheduler::queueUpdate() ->  ( " << runningTransfers->size()
           << " : " << Settings::maxConnections() << " )" << endl;

    if( newTransfers <= 0 )
        return;

    //search for the next transfer in the list to be downloaded
    TransferList::iterator it = transfers->begin();
    TransferList::iterator endList = transfers->end();

    while( (newTransfers > 0)  && (it != endList) )
    {
        sDebug << "Scheduler::queueUpdate() -> Evaluating item..." << endl;
        if( ((*it)->info().status == Transfer::St_Stopped)
            && setTransferCommand(*it, CmdResume) ) 
        {
            sDebug << "Scheduler::queueUpdate() -> Item added to the queue" << endl;
            --newTransfers;
        }
        else
            sDebug << "Scheduler::queueUpdate() -> Item not added to the queue" << endl;
        ++it;
    }
}

//END

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
