/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007-2009 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/kget.h"

#include "mainwindow.h"
#include "core/transfer.h"
#include "core/transferdatasource.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "core/transfertreeselectionmodel.h"
#include "core/plugin/plugin.h"
#include "core/plugin/transferfactory.h"
#include "core/kuiserverjobs.h"
#include "core/transfergroupscheduler.h"
#include "settings.h"
#include "core/transferhistorystore.h"

#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kio/renamedialog.h>
#ifndef HAVE_KNOTIFICATIONITEM
#include <KSystemTrayIcon>
#endif
#include <KSharedConfig>
#include <KPluginInfo>
#include <KComboBox>
#include <KPassivePopup>

#include <QTextStream>
#include <QDomElement>
#include <QApplication>
#include <QClipboard>
#include <QAbstractItemView>

#ifdef HAVE_NEPOMUK
    #include <Nepomuk/ResourceManager>
#endif

#ifdef HAVE_KWORKSPACE
    #include <kworkspace/kworkspace.h>
#endif

/**
 * This is our KGet class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

KGet& KGet::self( MainWindow * mainWindow )
{
    if(mainWindow)
    {
        m_mainWindow = mainWindow;
        m_jobManager = new KUiServerJobs(m_mainWindow);
    }

    static KGet m;
    return m;
}

void KGet::deleteSelf()
{
}

bool KGet::addGroup(const QString& groupName)
{
    kDebug(5001);

    // Check if a group with that name already exists
    if(m_transferTreeModel->findGroup(groupName))
        return false;

    TransferGroup * group = new TransferGroup(m_transferTreeModel, m_scheduler, groupName);
    m_transferTreeModel->addGroup(group);

    return true;
}

void KGet::delGroup(const QString& groupName)
{
    TransferGroup * group = m_transferTreeModel->findGroup(groupName);

    if(group)
    {
        JobQueue::iterator begin = group->begin();
        JobQueue::iterator end = group->end();
        for (; begin != end; begin++) {
            Transfer *transfer = static_cast<Transfer*>(*begin);
            m_transferTreeModel->delTransfer(transfer);//Unregister transfers from model first, they will get deleted by the group when destructing
        }
        m_transferTreeModel->delGroup(group);
        group->deleteLater();
    }
}

void KGet::renameGroup(const QString& oldName, const QString& newName)
{
    TransferGroup *group = m_transferTreeModel->findGroup(oldName);

    if(group)
    {
        group->handler()->setName(newName);
    }
}

QStringList KGet::transferGroupNames()
{
    QStringList names;

    foreach(TransferGroup *group, m_transferTreeModel->transferGroups()) {
        names << group->name();
    }

    return names;
}

TransferHandler * KGet::addTransfer(KUrl srcUrl, QString destDir, QString suggestedFileName, // krazy:exclude=passbyvalue
                       const QString& groupName, bool start)
{
    // Note: destDir may actually be a full path to a file :-(
    kDebug(5001) << "Source:" << srcUrl.url() << ", dest: " << destDir << ", sugg file: " << suggestedFileName << endl;

    KUrl destUrl; // the final destination, including filename

    if ( srcUrl.isEmpty() )
    {
        //No src location: we let the user insert it manually
        srcUrl = urlInputDialog();
        if( srcUrl.isEmpty() )
            return 0;
    }

    if ( !isValidSource( srcUrl ) )
        return 0;

    // when we get a destination directory and suggested filename, we don't
    // need to ask for it again
    bool confirmDestination = false;
    if (destDir.isEmpty())
    {
        confirmDestination = true;
        QList<TransferGroupHandler*> list = groupsFromExceptions(srcUrl);
        if (!list.isEmpty())
            destDir = list.first()->defaultFolder();
    } else {
        // check whether destDir is actually already the path to a file
        KUrl targetUrl = KUrl(destDir);
        QString directory = targetUrl.directory(KUrl::ObeyTrailingSlash);
        QString fileName = targetUrl.fileName(KUrl::ObeyTrailingSlash);
        if (QFileInfo(directory).isDir() && !fileName.isEmpty())
        {
            destDir = directory;
            suggestedFileName = fileName;
        }
    }

    if (suggestedFileName.isEmpty())
    {
	confirmDestination = true;
        suggestedFileName = srcUrl.fileName(KUrl::ObeyTrailingSlash);
        if (suggestedFileName.isEmpty())
        {
            // simply use the full url as filename
            suggestedFileName = KUrl::toPercentEncoding( srcUrl.prettyUrl(), "/" );
        }
    }

    // now ask for confirmation of the entire destination url (dir + filename)
    if (confirmDestination || !isValidDestDirectory(destDir))
    {
        do 
        {
            destUrl = destFileInputDialog(destDir, suggestedFileName);
            if (destUrl.isEmpty()) 
        return 0;

            destDir = destUrl.directory(KUrl::ObeyTrailingSlash);
        } while (!isValidDestDirectory(destDir));
    } else {
        destUrl = KUrl();
        destUrl.setDirectory(destDir); 
        destUrl.addPath(suggestedFileName);
    }

    if(m_transferTreeModel->findTransferByDestination(destUrl) != 0 || (destUrl.isLocalFile() && QFile::exists(destUrl.path()))) {
        KIO::RenameDialog dlg( m_mainWindow, i18n("Rename transfer"), srcUrl,
                               destUrl, KIO::M_MULTI);
        if (dlg.exec() == KIO::R_RENAME)
            destUrl = dlg.newDestUrl();
        else
            return 0;
    }
    return createTransfer(srcUrl, destUrl, groupName, start);
}

TransferHandler * KGet::addTransfer(const QDomElement& e, const QString& groupName)
{
    //We need to read these attributes now in order to know which transfer
    //plugin to use.
    KUrl srcUrl = KUrl( e.attribute("Source") );
    KUrl destUrl = KUrl( e.attribute("Dest") );

    kDebug(5001) << " src= " << srcUrl.url()
              << " dest= " << destUrl.url() 
              << " group= "<< groupName << endl;

    if ( srcUrl.isEmpty() || !isValidSource(srcUrl) 
         || !isValidDestDirectory(destUrl.directory()) )
        return 0;

    return createTransfer(srcUrl, destUrl, groupName, false, &e);
}

const QList<TransferHandler *> KGet::addTransfer(KUrl::List srcUrls, QString destDir, const QString& groupName, bool start)
{
    KUrl::List urlsToDownload;

    KUrl::List::Iterator it = srcUrls.begin();
    KUrl::List::Iterator itEnd = srcUrls.end();
    
    QList<TransferHandler *> addedTransfers;

    for(; it!=itEnd ; ++it)
    {
        if ( isValidSource( *it ) )
            urlsToDownload.append( *it );
    }

    if ( urlsToDownload.count() == 0 )
        return addedTransfers;

    if ( urlsToDownload.count() == 1 )
    {
        // just one file -> ask for filename
        TransferHandler * newTransfer = addTransfer(srcUrls.first(), destDir, srcUrls.first().fileName(), groupName, start);
        
        if(newTransfer)
            addedTransfers.append(newTransfer);
    }

    KUrl destUrl;

    // multiple files -> ask for directory, not for every single filename
    if (!isValidDestDirectory(destDir))//TODO: Move that after the for-loop
        destDir = destDirInputDialog();

    it = urlsToDownload.begin();
    itEnd = urlsToDownload.end();

    for ( ; it != itEnd; ++it )
    {
        if (destDir.isEmpty())
        {
            QList<TransferGroupHandler*> list = groupsFromExceptions(*it);
            if (!list.isEmpty())
                destDir = list.first()->defaultFolder();
        }
        destUrl = getValidDestUrl(destDir, *it);

        if(!isValidDestUrl(destUrl))
            continue;

        TransferHandler * newTransfer = createTransfer(*it, destUrl, groupName, start);
        
        if(newTransfer)
            addedTransfers.append(newTransfer);
    }
    
    return addedTransfers;
}


bool KGet::delTransfer(TransferHandler * transfer)
{
    Transfer * t = transfer->m_transfer;
    t->stop();
    TransferHistoryStore::getStore()->saveItem(TransferHistoryItem(*t));

    //Here I delete the Transfer. The other possibility is to move it to a list
    //and to delete all these transfers when kget gets closed. Obviously, after
    //the notification to the views that the transfer has been removed, all the
    //pointers to it are invalid.
    transfer->postDeleteEvent();
    m_transferTreeModel->delTransfer(t);
    t->deleteLater();
    return true;
}

void KGet::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
  Q_UNUSED(transfer);
  Q_UNUSED(groupName);
}

void KGet::redownloadTransfer(TransferHandler * transfer)
{
     QString group = transfer->group()->name();
     QString src = transfer->source().url();
     QString dest = transfer->dest().url();
     QString destFile = transfer->dest().fileName();
     bool running = false;
     if (transfer->status() == Job::Running)
         running = true;

     KGet::delTransfer(transfer);
     KGet::addTransfer(src, dest, destFile, group, running);
}

QList<TransferHandler *> KGet::selectedTransfers()
{
//     kDebug(5001) << "KGet::selectedTransfers";

    QList<TransferHandler *> selectedTransfers;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();

    foreach(const QModelIndex &currentIndex, selectedIndexes)
    {
        if(!m_transferTreeModel->isTransferGroup(currentIndex))
            selectedTransfers.append(static_cast<TransferHandler *> (currentIndex.internalPointer()));
    }

    return selectedTransfers;


// This is the code that was used in the old selectedTransfers function
/*    QList<TransferGroup *>::const_iterator it = m_transferTreeModel->transferGroups().begin();
    QList<TransferGroup *>::const_iterator itEnd = m_transferTreeModel->transferGroups().end();

    for( ; it!=itEnd ; ++it )
    {
        TransferGroup::iterator it2 = (*it)->begin();
        TransferGroup::iterator it2End = (*it)->end();

        for( ; it2!=it2End ; ++it2 )
        {
            Transfer * transfer = (Transfer*) *it2;

            if( transfer->isSelected() )
                selectedTransfers.append( transfer->handler() );
        }
    }
    return selectedTransfers;*/
}

QList<TransferHandler *> KGet::finishedTransfers()
{
    QList<TransferHandler *> finishedTransfers;

    foreach(TransferHandler *transfer, allTransfers())
    {
        if (transfer->status() == Job::Finished) {
            finishedTransfers << transfer;
        }
    }
    return finishedTransfers;
}

QList<TransferGroupHandler *> KGet::selectedTransferGroups()
{
    QList<TransferGroupHandler *> selectedTransferGroups;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();

    foreach(const QModelIndex &currentIndex, selectedIndexes)
    {
        if(m_transferTreeModel->isTransferGroup(currentIndex))
            selectedTransferGroups.append(static_cast<TransferGroupHandler *> (currentIndex.internalPointer()));
    }

    return selectedTransferGroups;
}

TransferTreeModel * KGet::model()
{
    return m_transferTreeModel;
}

TransferTreeSelectionModel * KGet::selectionModel()
{
    return m_selectionModel;
}

void KGet::addTransferView(QAbstractItemView * view)
{
    view->setModel(m_transferTreeModel);
}

void KGet::addTransferView(KComboBox * view)
{
    view->setModel(m_transferTreeModel);
}

void KGet::load( QString filename ) // krazy:exclude=passbyvalue
{
    kDebug(5001) << "(" << filename << ")";

    if(filename.isEmpty())
        filename = KStandardDirs::locateLocal("appdata", "transfers.kgt");

    QString tmpFile;

    //Try to save the transferlist to a temporary location
    if(!KIO::NetAccess::download(KUrl(filename), tmpFile, 0))
        return;

    QFile file(tmpFile);
    QDomDocument doc;

    kDebug(5001) << "file:" << filename;

    if(doc.setContent(&file))
    {
        QDomElement root = doc.documentElement();

        QDomNodeList nodeList = root.elementsByTagName("TransferGroup");
        int nItems = nodeList.length();

        for( int i = 0 ; i < nItems ; i++ )
        {
            TransferGroup * foundGroup = m_transferTreeModel->findGroup( nodeList.item(i).toElement().attribute("Name") );

            kDebug(5001) << "KGet::load  -> group = " << nodeList.item(i).toElement().attribute("Name");

            if( !foundGroup )
            {
                kDebug(5001) << "KGet::load  -> group not found";

                TransferGroup * newGroup = new TransferGroup(m_transferTreeModel, m_scheduler);

                m_transferTreeModel->addGroup(newGroup);

                newGroup->load(nodeList.item(i).toElement());
            }
            else
            {
                kDebug(5001) << "KGet::load  -> group found";

                //A group with this name already exists.
                //Integrate the group's transfers with the ones read from file
                foundGroup->load(nodeList.item(i).toElement());
            }
        }
    }
    else
    {
        kWarning(5001) << "Error reading the transfers file";
    }

    new GenericObserver(m_mainWindow);
}

void KGet::save( QString filename, bool plain ) // krazy:exclude=passbyvalue
{
    if ( !filename.isEmpty()
        && QFile::exists( filename )
        && (KMessageBox::questionYesNoCancel(0,
                i18n("The file %1 already exists.\nOverwrite?", filename),
                i18n("Overwrite existing file?"), KStandardGuiItem::yes(),
                KStandardGuiItem::no(), KStandardGuiItem::cancel(), "QuestionFilenameExists" )
                != KMessageBox::Yes) )
        return;

    if(filename.isEmpty())
        filename = KStandardDirs::locateLocal("appdata", "transfers.kgt");

    QFile file(filename);
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        //kWarning(5001)<<"Unable to open output file when saving";
        KMessageBox::error(0,
                        i18n("Unable to save to: %1", filename),
                        i18n("Error"));

        return;
    }

    if (plain) {
        QTextStream out(&file);
        foreach(TransferHandler *handler, allTransfers()) {
            out << handler->source().prettyUrl() << endl;
        }
    }
    else {
        QDomDocument doc(QString("KGetTransfers"));
        QDomElement root = doc.createElement("Transfers");
        doc.appendChild(root);

        QList<TransferGroup *>::const_iterator it = m_transferTreeModel->transferGroups().begin();
        QList<TransferGroup *>::const_iterator itEnd = m_transferTreeModel->transferGroups().end();

        for ( ; it!=itEnd ; ++it )
        {
            QDomElement e = doc.createElement("TransferGroup");
            root.appendChild(e);
            (*it)->save(e);
            //KGet::delGroup((*it)->name());
        }

        QTextStream stream( &file );
        doc.save( stream, 0 );
    }
    file.close();
}

TransferFactory * KGet::factory(TransferHandler * transfer)
{
    return transfer->m_transfer->factory();
}

KActionCollection * KGet::actionCollection()
{
    return m_mainWindow->actionCollection();
}

void KGet::setSchedulerRunning(bool running)
{
    if(running)
    {
        m_scheduler->stop(); //stopall first, to have a clean startingpoint
	    m_scheduler->start();
    }
    else
	    m_scheduler->stop();
}

bool KGet::schedulerRunning()
{
    return (m_scheduler->countRunningJobs() > 0);
}

QList<TransferHandler*> KGet::allTransfers()
{
    QList<TransferHandler*> transfers;

    foreach (TransferGroup *group, KGet::m_transferTreeModel->transferGroups())
    {
        transfers << group->handler()->transfers();
    }
    return transfers;
}

QList<TransferGroupHandler*> KGet::allTransferGroups()
{
    QList<TransferGroupHandler*> transfergroups;

    foreach (TransferGroup *group, KGet::m_transferTreeModel->transferGroups())
    {
        transfergroups << group->handler();
    }
    return transfergroups;
}

TransferHandler * KGet::findTransfer(const KUrl &src)
{
    Transfer *transfer = KGet::m_transferTreeModel->findTransfer(src);
    if (transfer)
    {
        return transfer->handler();
    }
    return 0;
}

void KGet::checkSystemTray()
{
    kDebug(5001);
    bool running = false;

    foreach (TransferHandler *handler, KGet::allTransfers())
    {
        if (handler->status() == Job::Running)
            running = true;

        if (running)
            continue;
    }

    m_mainWindow->setSystemTrayDownloading(running);
}

void KGet::settingsChanged()
{
    kDebug(5001);

    QList<TransferFactory*>::const_iterator it = m_transferFactories.constBegin();
    QList<TransferFactory*>::const_iterator itEnd = m_transferFactories.constEnd();

    for( ; it!=itEnd ; ++it )
    {
        (*it)->settingsChanged();
    }
}

void KGet::registerKJob(KJob *job)
{
    m_jobManager->registerJob(job);
}

void KGet::unregisterKJob(KJob *job)
{
    m_jobManager->unregisterJob(job);
}

void KGet::reloadKJobs()
{
    m_jobManager->reload();
}

QList<TransferGroupHandler*> KGet::groupsFromExceptions(const KUrl &filename)
{
    QList<TransferGroupHandler*> handlers;
    foreach (TransferGroupHandler * handler, allTransferGroups()) {
        QRegExp regExp = handler->regExp();
        if (!regExp.pattern().isEmpty() && !regExp.pattern().startsWith('*'))
            regExp.setPattern('*' + regExp.pattern());

        regExp.setPatternSyntax(QRegExp::Wildcard);

        if (regExp.exactMatch(filename.url())) {
            handlers.append(handler);
        }
    }

    return handlers;
}

void KGet::setGlobalDownloadLimit(int limit)
{
    m_scheduler->setDownloadLimit(limit);
    if (!limit)
        m_scheduler->calculateDownloadLimit();
}

void KGet::setGlobalUploadLimit(int limit)
{
    m_scheduler->setUploadLimit(limit);
    if (!limit)
        m_scheduler->calculateUploadLimit();
}

void KGet::calculateGlobalSpeedLimits()
{
    if (m_scheduler->downloadLimit())//TODO: Remove this and the both other hacks in the 2 upper functions with a better replacement
        m_scheduler->calculateDownloadLimit();
    if (m_scheduler->uploadLimit())
        m_scheduler->calculateUploadLimit();
}

void KGet::calculateGlobalDownloadLimit()
{
    m_scheduler->calculateDownloadLimit();
}

void KGet::calculateGlobalUploadLimit()
{
    m_scheduler->calculateUploadLimit();
}

// ------ STATIC MEMBERS INITIALIZATION ------
TransferTreeModel * KGet::m_transferTreeModel;
TransferTreeSelectionModel * KGet::m_selectionModel;
QList<TransferFactory *> KGet::m_transferFactories;
TransferGroupScheduler * KGet::m_scheduler = new TransferGroupScheduler();
MainWindow * KGet::m_mainWindow = 0;
KUiServerJobs * KGet::m_jobManager = 0;

// ------ PRIVATE FUNCTIONS ------
KGet::KGet()
{
#ifdef HAVE_NEPOMUK
    Nepomuk::ResourceManager::instance()->init();
#endif

    m_transferTreeModel = new TransferTreeModel(m_scheduler);
    m_selectionModel = new TransferTreeSelectionModel(m_transferTreeModel);

    //Load all the available plugins
    loadPlugins();

    //Create the default group
    addGroup(i18n("My Downloads"));
}

KGet::~KGet()
{
    delete m_transferTreeModel;
    delete m_scheduler;
}

TransferHandler * KGet::createTransfer(const KUrl &src, const KUrl &dest, const QString& groupName, 
                          bool start, const QDomElement * e)
{
    kDebug(5001) << "srcUrl= " << src.url() << "  " 
                             << "destUrl= " << dest.url() 
                             << "group= _" << groupName << "_" << endl;

    TransferGroup * group = m_transferTreeModel->findGroup(groupName);
    if (group==0)
    {
        kDebug(5001) << "KGet::createTransfer  -> group not found";
        group = m_transferTreeModel->transferGroups().first();
    }
    Transfer * newTransfer;

    QList<TransferFactory *>::iterator it = m_transferFactories.begin();
    QList<TransferFactory *>::iterator itEnd = m_transferFactories.end();

    for( ; it!=itEnd ; ++it)
    {
        kDebug(5001) << "Trying plugin   n.plugins=" << m_transferFactories.size();
        if((newTransfer = (*it)->createTransfer(src, dest, group, m_scheduler, e)))
        {
//             kDebug(5001) << "KGet::createTransfer   ->   CREATING NEW TRANSFER ON GROUP: _" << group->name() << "_";
            newTransfer->init();
            newTransfer->load(e);
            m_transferTreeModel->addTransfer(newTransfer, group);

            if(start)
                newTransfer->handler()->start();

            return newTransfer->handler();
        }
    }
    kDebug(5001) << "Warning! No plugin found to handle the given url";
    return 0;
}

TransferDataSource * KGet::createTransferDataSource(const KUrl &src)
{
    kDebug(5001);
    QList<TransferFactory *>::iterator it = m_transferFactories.begin();
    QList<TransferFactory *>::iterator itEnd = m_transferFactories.end();

    TransferDataSource *dataSource;
    for( ; it!=itEnd ; ++it)
    {
        dataSource = (*it)->createTransferDataSource(src);
        if(dataSource)
            return dataSource;
    }
    return 0;
}

KUrl KGet::urlInputDialog()
{
    QString newtransfer;
    bool ok = false;

    KUrl clipboardUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
    if (clipboardUrl.isValid())
        newtransfer = clipboardUrl.url();

    while (!ok)
    {
        newtransfer = KInputDialog::getText(i18n("New Download"), i18n("Enter URL:"), newtransfer, &ok, 0);

        if (!ok)
        {
            //user pressed cancel
            return KUrl();
        }

        KUrl src = KUrl(newtransfer);
        if(src.isValid())
            return src;
        else
            ok = false;
    }
    return KUrl();
}

QString KGet::destDirInputDialog()
{
    QString destDir = KFileDialog::getExistingDirectory(Settings::lastDirectory());

    Settings::setLastDirectory( destDir );
    return destDir;
}

KUrl KGet::destFileInputDialog(QString destDir, const QString& suggestedFileName) // krazy:exclude=passbyvalue
{
    if (destDir.isEmpty())
        destDir = Settings::lastDirectory();

    // open the filedialog for confirmation
    KFileDialog dlg(destDir, QString(),
                    0L, 0L);
    dlg.setCaption(i18n("Save As"));
    dlg.setOperationMode(KFileDialog::Saving);
    dlg.setMode(KFile::File | KFile::LocalOnly);

    // Use the destination name if not empty...
    if (!suggestedFileName.isEmpty())
        dlg.setSelection(suggestedFileName);

    if ( dlg.exec() == QDialog::Rejected )
    {
        return KUrl();
    }
    else
    {
        KUrl destUrl = dlg.selectedUrl();
        Settings::setLastDirectory( destUrl.directory(KUrl::ObeyTrailingSlash) );
        return destUrl;
    }
}

bool KGet::isValidSource(const KUrl &source)
{
    // Check if the URL is well formed
    if (!source.isValid())
    {
        KMessageBox::error(0,
                           i18n("Malformed URL:\n%1", source.prettyUrl()),
                           i18n("Error"));

        return false;
    }
    // Check if the URL contains the protocol
    if ( source.protocol().isEmpty() ){

        KMessageBox::error(0, i18n("Malformed URL, protocol missing:\n%1", source.prettyUrl()),
                           i18n("Error"));

        return false;
    }
    // Check if a transfer with the same url already exists
    Transfer * transfer = m_transferTreeModel->findTransfer( source );
    if ( transfer )
    {
        if ( transfer->status() == Job::Finished )
        {
            // transfer is finished, ask if we want to download again
            if (KMessageBox::questionYesNoCancel(0,
                i18n("URL already saved:\n%1\nDownload again?", source.prettyUrl()),
                i18n("Download URL again?"), KStandardGuiItem::yes(),
                KStandardGuiItem::no(), KStandardGuiItem::cancel(), "QuestionUrlAlreadySaved" )
                == KMessageBox::Yes)
            {
                transfer->stop();
                TransferHandler * th = transfer->handler();
                KGet::delTransfer(th);
                return true;
            }
        }
        else
        {
            //Transfer is already in list and not finished, ...
            return false;
        }
        return false;
    }
    return true;
}

bool KGet::isValidDestDirectory(const QString & destDir)
{
    kDebug(5001) << destDir;
    if (!QFileInfo(destDir).isDir())
    {
        if (QFileInfo(KUrl(destDir).directory()).isWritable())
            return (!destDir.isEmpty());
        if (!QFileInfo(KUrl(destDir).directory()).isWritable() && !destDir.isEmpty())
            KMessageBox::error(0, i18n("Directory is not writable"));
    }
    else
    {
        if (QFileInfo(destDir).isWritable())
            return (!destDir.isEmpty());
        if (!QFileInfo(destDir).isWritable() && !destDir.isEmpty())
            KMessageBox::error(0, i18n("Directory is not writable"));
    }
    return false;
}

bool KGet::isValidDestUrl(const KUrl &destUrl)
{
    if(KIO::NetAccess::exists(destUrl, KIO::NetAccess::DestinationSide, 0))
    {
        if (KMessageBox::warningYesNo(0,
            i18n("Destination file \n%1\nalready exists.\n"
                 "Do you want to overwrite it?", destUrl.prettyUrl()), i18n("Overwrite destination"),
            KStandardGuiItem::overwrite(), KStandardGuiItem::cancel()) == KMessageBox::Yes)
        {
            safeDeleteFile( destUrl );
            return true;
        }
        else
            return false;
    }
    if (m_transferTreeModel->findTransferByDestination(destUrl) || destUrl.isEmpty())
        return false;

    return true;
   /*
    KIO::open_RenameDlg(i18n("File already exists"), 
    (*it).url(), destUrl.url(),
    KIO::M_MULTI);
   */
}

KUrl KGet::getValidDestUrl(const QString& destDir, const KUrl &srcUrl, const QString& destFileName)
{
    if ( !isValidDestDirectory(destDir) )
        return KUrl();

    // create a proper destination file from destDir
    KUrl destUrl = KUrl( destDir );
    QString filename = destFileName.isEmpty() ? srcUrl.fileName() : destFileName;

    if ( filename.isEmpty() )
    {
        // simply use the full url as filename
        filename = KUrl::toPercentEncoding( srcUrl.prettyUrl(), "/" );
        kDebug(5001) << " Filename is empty. Setting to  " << filename;
        kDebug(5001) << "   srcUrl = " << srcUrl.url();
        kDebug(5001) << "   prettyUrl = " << srcUrl.prettyUrl();
    }
    if (QFileInfo(destUrl.path()).isDir())
    {
        kDebug(5001) << " Filename is not empty";
        destUrl.adjustPath( KUrl::AddTrailingSlash );
        destUrl.setFileName( filename );
    }
    if (!isValidDestUrl(destUrl))
    {
        kDebug(5001) << "   destUrl " << destUrl.path() << " is not valid";
        return KUrl();
    }
    return destUrl;
}

void KGet::loadPlugins()
{
    m_transferFactories.clear();
    // Add versioning constraint
    QString
    str  = "[X-KDE-KGet-framework-version] == ";
    str += QString::number( FrameworkVersion );
    str += " and ";
    str += "[X-KDE-KGet-rank] > 0";
    str += " and ";
    str += "[X-KDE-KGet-plugintype] == ";


    //TransferFactory plugins
    KService::List offers = KServiceTypeTrader::self()->query( "KGet/Plugin", str + "'TransferFactory'" );

    //Here we use a QMap only to easily sort the plugins by rank
    QMap<int, KService::Ptr> services;
    QMap<int, KService::Ptr>::ConstIterator it;

    for ( int i = 0; i < offers.count(); ++i )
    {
        services[ offers[i]->property( "X-KDE-KGet-rank" ).toInt() ] = offers[i];
        kDebug(5001) << " TransferFactory plugin found:" << endl <<
         "  rank = " << offers[i]->property( "X-KDE-KGet-rank" ).toInt() << endl <<
         "  plugintype = " << offers[i]->property( "X-KDE-KGet-plugintype" ) << endl;
    }

    //I must fill this pluginList before and my m_transferFactories list after.
    //This because calling the KLibLoader::globalLibrary() erases the static
    //members of this class (why?), such as the m_transferFactories list.
    QList<KGetPlugin *> pluginList;

    const KConfigGroup plugins = KConfigGroup(KGlobal::config(), "Plugins");
   
    for( it = services.constBegin(); it != services.constEnd(); ++it )
    {
        KPluginInfo info(*it);
        info.load(plugins);

        if( !info.isPluginEnabled() ) {
            kDebug(5001) << "TransferFactory plugin (" << (*it)->library()
                             << ") found, but not enabled";
            continue;
        }

        KGetPlugin * plugin;
        if( (plugin = createPluginFromService(*it)) != 0 )
        {
            const QString pluginName = info.name();
            
            pluginList.prepend(plugin);
            kDebug(5001) << "TransferFactory plugin (" << (*it)->library() 
                         << ") found and added to the list of available plugins";
                
        }
        else
            kDebug(5001) << "Error loading TransferFactory plugin (" 
                      << (*it)->library() << ")";
    }

    QList<KGetPlugin *>::ConstIterator it2 = pluginList.constBegin();
    QList<KGetPlugin *>::ConstIterator it2End = pluginList.constEnd();

    for( ; it2!=it2End ; ++it2 )
        m_transferFactories.append( qobject_cast<TransferFactory *>(*it2) );

    kDebug(5001) << "Number of factories = " << m_transferFactories.size();
}

KGetPlugin * KGet::createPluginFromService( const KService::Ptr &service )
{
    //try to load the specified library
    KPluginFactory *factory = KPluginLoader(service->library()).factory();

    if (!factory)
    {
        KMessageBox::error(0, i18n("<html><p>Plugin loader could not load the plugin:<br/><i>%1</i></p></html>",
                                   service->library()));
        kError(5001) << "KPluginFactory could not load the plugin:" << service->library();
        return 0;
    }
    KGetPlugin * plugin = factory->create< TransferFactory >(KGet::m_mainWindow);

    return plugin;
}

bool KGet::safeDeleteFile( const KUrl& url )
{
    if ( url.isLocalFile() )
    {
        QFileInfo info( url.path() );
        if ( info.isDir() )
        {
            KGet::showNotification(m_mainWindow, KNotification::Notification,
                                   i18n("Not deleting\n%1\nas it is a directory.", url.prettyUrl()),
                                   "dialog-info");
            return false;
        }
        KIO::NetAccess::del( url, 0L );
        return true;
    }

    else
        KGet::showNotification(m_mainWindow, KNotification::Notification,
                               i18n("Not deleting\n%1\nas it is not a local file.", url.prettyUrl()),
                               "dialog-info");
    return false;
}

void KGet::showNotification(QWidget *parent, KNotification::StandardEvent eventId, 
                            const QString &text, const QString &icon)
{
    KNotification::event(eventId, text, KIcon(icon).pixmap(KIconLoader::Small), parent);
}

GenericObserver::GenericObserver(QObject *parent)
  : QObject(parent)
{
    connect(KGet::model(), SIGNAL(groupRemovedEvent(TransferGroupHandler*)), SLOT(groupRemovedEvent(TransferGroupHandler*)));
    connect(KGet::model(), SIGNAL(transferAddedEvent(TransferHandler*)), SLOT(transferAddedEvent(TransferHandler*)));
    connect(KGet::model(), SIGNAL(groupAddedEvent(TransferGroupHandler*)), SLOT(groupAddedEvent(TransferGroupHandler*)));
    connect(KGet::model(), SIGNAL(transferRemovedEvent(TransferHandler*)), SLOT(transferRemovedEvent(TransferHandler*)));
    connect(KGet::model(), SIGNAL(transfersChangedEvent(QMap<TransferHandler*, Transfer::ChangesFlags>)), 
                           SLOT(transfersChangedEvent(QMap<TransferHandler*, Transfer::ChangesFlags>)));
    connect(KGet::model(), SIGNAL(groupsChangedEvent(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags>)), 
                           SLOT(groupsChangedEvent(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags>)));
    connect(KGet::model(), SIGNAL(transferMovedEvent(TransferHandler *, TransferGroupHandler *)),
                           SLOT(transferMovedEvent(TransferHandler *, TransferGroupHandler *)));
}

GenericObserver::~GenericObserver()
{
}

void GenericObserver::groupAddedEvent(TransferGroupHandler *handler)
{
    Q_UNUSED(handler)
    KGet::save();
}

void GenericObserver::groupRemovedEvent(TransferGroupHandler *handler)
{
    Q_UNUSED(handler)
    KGet::save();
}

void GenericObserver::transferAddedEvent(TransferHandler *handler)
{
    Q_UNUSED(handler)
    KGet::save();
    KGet::calculateGlobalSpeedLimits();
}

void GenericObserver::transferRemovedEvent(TransferHandler *handler)
{
    Q_UNUSED(handler)
    KGet::save();
    KGet::calculateGlobalSpeedLimits();
}

void GenericObserver::transferMovedEvent(TransferHandler *transfer, TransferGroupHandler *group)
{
    Q_UNUSED(transfer)
    Q_UNUSED(group)
    KGet::save();
    KGet::calculateGlobalSpeedLimits();
}

void GenericObserver::transfersChangedEvent(QMap<TransferHandler*, Transfer::ChangesFlags> transfers)
{
    kDebug();
    if (Settings::afterFinishActionEnabled())
    {
        bool allFinished = true;
        foreach (TransferHandler *transfer, transfers.keys())
        {
            if (transfer->status() != Job::Finished)
                allFinished = false;
        }
        if (allFinished)
            kDebug() << "All finished";
        if (allFinished && Settings::afterFinishAction() == KGet::Quit)
            checkAndFinish();

#ifdef HAVE_KWORKSPACE
        if (allFinished && Settings::afterFinishAction() == KGet::Shutdown)
            checkAndShutdown();
#endif
    }

    bool checkSysTray = false;
    foreach (TransferHandler *transfer, transfers.keys())
    {
        TransferHandler::ChangesFlags transferFlags = transfers[transfer];
        
        if (transferFlags & Transfer::Tc_Status)
            checkSysTray = true;

        if (transferFlags & Transfer::Tc_Percent) {
            transfer->group()->setGroupChange(TransferGroup::Gc_Percent, true);
            transfer->checkShareRatio();
        }

        if (transferFlags & Transfer::Tc_DownloadSpeed) {
            transfer->group()->setGroupChange(TransferGroup::Gc_DownloadSpeed, true);
        }

        if (transferFlags & Transfer::Tc_UploadSpeed) {
            transfer->group()->setGroupChange(TransferGroup::Gc_UploadSpeed, true);
        }
    }
    if (checkSysTray)
        KGet::checkSystemTray();
}

void GenericObserver::groupsChangedEvent(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags> groups)
{
    bool recalculate = false;
    foreach (TransferGroup::ChangesFlags flags, groups.values())
    {
        if (flags & TransferGroup::Gc_Percent)
        {
            recalculate = true;
            break;
        }
    }
    if (recalculate)
        KGet::calculateGlobalSpeedLimits();
}

bool GenericObserver::allTransfersFinished()
{
    bool quitFlag = true;

    // if all the downloads had state finished from
    // the beginning
    bool allWereFinished = true;

    foreach(TransferGroup *transferGroup, KGet::model()->transferGroups()) {
        foreach(TransferHandler *transfer, transferGroup->handler()->transfers()) {
            if(transfer->status() != Job::Finished) {
                quitFlag = false;
            }
            if (transfer->status() == Job::Finished &&
               transfer->startStatus() != Job::Finished)
            {
                allWereFinished = false;
            }
        }
    }

    // if the only downloads in the queue
    // are those that are already finished
    // before the current KGet instance
    // we don't want to quit
    if (allWereFinished)
    {
        return false;
    }

    // otherwise, we did some downloads right now, let quitFlag decide
    return quitFlag;
}

KPassivePopup* GenericObserver::popupMessage(const QString &title, const QString &message)
{
    KPassivePopup *popup;
    // we have to call diferent message from kpassivePopup
    // one with parent as QWidget for the mainWindow
    // and another with parent as QSystemTrayIcon if the parent is a systemTray
    // so passing the QSystemTrayIcon as QWidget don't work
    if(Settings::enableSystemTray()) 
    {
#ifdef HAVE_KNOTIFICATIONITEM
        //FIXME: proper solution ...
        popup = KPassivePopup::message(5000, title, message, KGet::m_mainWindow);
#else
        popup = KPassivePopup::message(5000, title, message, KGet::m_mainWindow->systemTray());
#endif
    }
    else 
    {
        popup = KPassivePopup::message(5000, title, message, KGet::m_mainWindow);
    }

    return popup;
}

void GenericObserver::checkAndFinish()
{
    kDebug();
    // check if there is some unfinished transfer in scheduler queues
    if(allTransfersFinished()) {
        KPassivePopup *message = popupMessage(i18n("Quit KGet"),
                                            i18n("KGet is now closing, as all downloads have completed."));
        QObject::connect(message, SIGNAL(destroyed()), KGet::m_mainWindow, SLOT(slotQuit()));
    }
}

#ifdef HAVE_KWORKSPACE
void GenericObserver::checkAndShutdown()
{
    if(allTransfersFinished()) {
        KPassivePopup *message = popupMessage(i18n("Quit KGet"),
                                            i18n("The computer will now turn off, as all downloads have completed."));
        QObject::connect(message, SIGNAL(destroyed()), SLOT(slotShutdown()));
        QObject::connect(message, SIGNAL(destroyed()),  KGet::m_mainWindow, SLOT(slotQuit()));
    }
}

void GenericObserver::slotShutdown()
{
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmNo,
                                KWorkSpace::ShutdownTypeHalt,
                                KWorkSpace::ShutdownModeForceNow);
}
#endif
