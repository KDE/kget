/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007-2009 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2008 Urs Wolfer <uwolfer @ kde.org>
   Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "core/kget.h"

#include "mainwindow.h"
#include "core/mostlocalurl.h"
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
#include <iostream>
#include <kinputdialog.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kio/renamedialog.h>
#include <KSharedConfig>
#include <KPluginInfo>
#include <KComboBox>
#include <KConfigDialog>
#include <KSaveFile>
#include <KWindowSystem>

#include <QTextStream>
#include <QDomElement>
#include <QApplication>
#include <QClipboard>
#include <QAbstractItemView>
#include <QTimer>

#ifdef HAVE_NEPOMUK
    #include <Nepomuk/ResourceManager>
    #include "nepomukcontroller.h"
#endif

#ifdef HAVE_KWORKSPACE
    #include <QDBusConnection>
    #include <QDBusInterface>
    #include <QDBusPendingCall>
    #include <kworkspace/kworkspace.h>
    #include <solid/powermanagement.h>
#endif


KGet::TransferData::TransferData(const KUrl &source, const KUrl &destination, const QString& group, bool doStart, const QDomElement *element)
  : src(source),
    dest(destination),
    groupName(group),
    start(doStart),
    e(element)
{
}

/**
 * This is our KGet class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

KGet* KGet::self( MainWindow * mainWindow )
{
    if(mainWindow)
    {
        m_mainWindow = mainWindow;
        m_jobManager = new KUiServerJobs(m_mainWindow);
    }

    static KGet *m = new KGet();

    return m;
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

void KGet::delGroup(TransferGroupHandler *group, bool askUser)
{
    TransferGroup *g = group->m_group;

    if (askUser) {
        QWidget *configDialog = KConfigDialog::exists("preferences");
        if (KMessageBox::warningYesNo(configDialog ? configDialog : m_mainWindow,
                        i18n("Are you sure that you want to remove the group named %1?", g->name()),
                        i18n("Remove Group"),
                        KStandardGuiItem::remove(), KStandardGuiItem::cancel()) != KMessageBox::Yes)
            return;
    }

    m_transferTreeModel->delGroup(g);
    g->deleteLater();
}

void KGet::delGroups(QList<TransferGroupHandler*> groups, bool askUser)
{
    if (groups.isEmpty())
        return;
    if (groups.count() == 1) {
        KGet::delGroup(groups.first(), askUser);
        return;
    }
    bool del = !askUser;
    if (askUser) {
        QStringList names;
        foreach (TransferGroupHandler * handler, groups)
            names << handler->name();
        QWidget * configDialog = KConfigDialog::exists("preferences");
        del = KMessageBox::warningYesNoList(configDialog ? configDialog : m_mainWindow,
              i18n("Are you sure that you want to remove the following groups?"),
              names,
              i18n("Remove groups"),
              KStandardGuiItem::remove(), KStandardGuiItem::cancel()) == KMessageBox::Yes;
    }
    if (del) {
        foreach (TransferGroupHandler * handler, groups)
            KGet::delGroup(handler, false);
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
                                    QString groupName, bool start)
{
    srcUrl = mostLocalUrl(srcUrl);
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
        if (!list.isEmpty()) {
            destDir = list.first()->defaultFolder();
            groupName = list.first()->name();
        }
        
    } else {
        // check whether destDir is actually already the path to a file
        KUrl targetUrl = KUrl(destDir);
        QString directory = targetUrl.directory(KUrl::ObeyTrailingSlash);
        QString fileName = targetUrl.fileName(KUrl::ObeyTrailingSlash);
        if (QFileInfo(directory).isDir() && !fileName.isEmpty()) {
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
    destUrl = getValidDestUrl(destUrl, srcUrl);

    if (destUrl == KUrl())
        return 0;

    TransferHandler *transfer = createTransfer(srcUrl, destUrl, groupName, start);
    if (transfer) {
        KGet::showNotification(m_mainWindow, "added",
                                i18n("<p>The following transfer has been added to the download list:</p><p style=\"font-size: small;\">%1</p>", transfer->source().pathOrUrl()),
                                "kget", i18n("Download added"));
    }

    return transfer;
}

QList<TransferHandler*> KGet::addTransfers(const QList<QDomElement> &elements, const QString &groupName)
{
    QList<TransferData> data;

    foreach(const QDomElement &e, elements) {
        //We need to read these attributes now in order to know which transfer
        //plugin to use.
        KUrl srcUrl = KUrl(e.attribute("Source"));
        KUrl destUrl = KUrl(e.attribute("Dest"));
        data << TransferData(srcUrl, destUrl, groupName, false, &e);

        kDebug(5001) << "src=" << srcUrl << " dest=" << destUrl << " group=" << groupName;
    }

    return createTransfers(data);
}

const QList<TransferHandler *> KGet::addTransfer(KUrl::List srcUrls, QString destDir, QString groupName, bool start)
{
    KUrl::List urlsToDownload;

    KUrl::List::Iterator it = srcUrls.begin();
    KUrl::List::Iterator itEnd = srcUrls.end();
    
    QList<TransferHandler *> addedTransfers;

    for(; it!=itEnd ; ++it)
    {
        *it = mostLocalUrl(*it);
        if ( isValidSource( *it ) )
            urlsToDownload.append( *it );
    }

    if ( urlsToDownload.count() == 0 )
        return addedTransfers;

    if ( urlsToDownload.count() == 1 )
    {
        // just one file -> ask for filename
        TransferHandler * newTransfer = addTransfer(srcUrls.first(), destDir, srcUrls.first().fileName(), groupName, start);

        if (newTransfer) {
            addedTransfers.append(newTransfer);
            return addedTransfers;
        }
    }

    KUrl destUrl;

    // multiple files -> ask for directory, not for every single filename
    if (!isValidDestDirectory(destDir))//TODO: Move that after the for-loop
        destDir = destDirInputDialog();

    it = urlsToDownload.begin();
    itEnd = urlsToDownload.end();

    QList<TransferData> data;
    for ( ; it != itEnd; ++it )
    {
        if (destDir.isEmpty())
        {
            //TODO only use groupsFromExceptions if that is allowed in the settings
            QList<TransferGroupHandler*> list = groupsFromExceptions(*it);
            if (!list.isEmpty()) {
                destDir = list.first()->defaultFolder();
                groupName = list.first()->name();
            }
        }
        destUrl = getValidDestUrl(KUrl(destDir), *it);

        if (destUrl == KUrl())
            continue;

        data << TransferData(*it, destUrl, groupName, start);
    }

    QList<TransferHandler*> transfers = createTransfers(data);
    if (!transfers.isEmpty()) {
        QString urls = transfers[0]->source().pathOrUrl();
        for (int i = 1; i < transfers.count(); ++i) {
            urls += '\n' + transfers[i]->source().pathOrUrl();
        }

        QString message;
        if (transfers.count() == 1) {
            message = i18n("<p>The following transfer has been added to the download list:</p>");
        } else {
            message = i18n("<p>The following transfers have been added to the download list:</p>");
        }
        const QString content = QString("<p style=\"font-size: small;\">%1</p>").arg(urls);
        KGet::showNotification(m_mainWindow, "added", message + content, "kget", i18n("Download added"));
    }

    return transfers;
}


bool KGet::delTransfer(TransferHandler * transfer, DeleteMode mode)
{
    return delTransfers(QList<TransferHandler*>() << transfer, mode);
}

bool KGet::delTransfers(const QList<TransferHandler*> &handlers, DeleteMode mode)
{
    if (!m_store) {
        m_store = TransferHistoryStore::getStore();
    }
    QList<Transfer*> transfers;
    QList<TransferHistoryItem> historyItems;
    foreach (TransferHandler *handler, handlers) {
        Transfer *transfer = handler->m_transfer;
        transfers << transfer;
        historyItems << TransferHistoryItem(*transfer);

        // TransferHandler deinitializations
        handler->destroy();
        // Transfer deinitializations (the deinit function is called by the destroy() function)
        if (mode == AutoDelete) {
            Transfer::DeleteOptions o = Transfer::DeleteTemporaryFiles;
            if (transfer->status() != Job::Finished && transfer->status() != Job::FinishedKeepAlive)
                o |= Transfer::DeleteFiles;
            transfer->destroy(o);
        } else {
            transfer->destroy((Transfer::DeleteTemporaryFiles | Transfer::DeleteFiles));
        }
    }
    m_store->saveItems(historyItems);

    m_transferTreeModel->delTransfers(transfers);
    qDeleteAll(transfers);
    return true;
}


void KGet::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
  Q_UNUSED(transfer)
  Q_UNUSED(groupName)
}

void KGet::redownloadTransfer(TransferHandler * transfer)
{
     QString group = transfer->group()->name();
     QString src = transfer->source().url();
     QString dest = transfer->dest().url();
     QString destFile = transfer->dest().fileName();

     KGet::delTransfer(transfer);
     KGet::addTransfer(src, dest, destFile, group, true);
}

QList<TransferHandler *> KGet::selectedTransfers()
{
//     kDebug(5001) << "KGet::selectedTransfers";

    QList<TransferHandler *> selectedTransfers;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();
    //sort the indexes as this can speed up operations like deleting etc.
    qSort(selectedIndexes.begin(), selectedIndexes.end());

    foreach(const QModelIndex &currentIndex, selectedIndexes)
    {
        ModelItem * item = m_transferTreeModel->itemFromIndex(currentIndex);
        if (!item->isGroup())
            selectedTransfers.append(item->asTransfer()->transferHandler());
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

QList<TransferGroupHandler *> KGet::selectedTransferGroups(bool *mainSelected)
{
    if (mainSelected) {
        *mainSelected = false;
    }
    QList<TransferGroupHandler *> selectedTransferGroups;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();

    foreach(const QModelIndex &currentIndex, selectedIndexes)
    {
        ModelItem * item = m_transferTreeModel->itemFromIndex(currentIndex);
        if (item->isGroup()) {
            TransferGroupHandler *group = item->asGroup()->groupHandler();
            if (mainSelected && group->name() == i18n("My Downloads")) {
                *mainSelected = true;
            }
            selectedTransferGroups.append(group);
        }
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

#ifdef HAVE_NEPOMUK
NepomukController *KGet::nepomukController()
{
    if (!m_nepomukController) {
        m_nepomukController = new NepomukController;
    }

    return m_nepomukController;
}
#endif

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

    KSaveFile file(filename);
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        //kWarning(5001)<<"Unable to open output file when saving";
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Unable to save to: %1", filename));
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

        foreach (TransferGroup * group, m_transferTreeModel->transferGroups())
        {
            QDomElement e = doc.createElement("TransferGroup");
            root.appendChild(e);
            group->save(e);
            //KGet::delGroup((*it)->name());
        }

        QTextStream stream( &file );
        doc.save( stream, 2 );
    }
    file.finalize();
}

QList<TransferFactory*> KGet::factories()
{
    return m_transferFactories;
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
    return (m_scheduler->hasRunningJobs());
}

void KGet::setSuspendScheduler(bool isSuspended)
{
    m_scheduler->setIsSuspended(isSuspended);
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
        kDebug() << group->name();
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

TransferGroupHandler * KGet::findGroup(const QString &name)
{
    TransferGroup *group = KGet::m_transferTreeModel->findGroup(name);
    if (group)
    {
        return group->handler();
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
        {
            running = true;
            break;
        }
    }

    m_mainWindow->setSystemTrayDownloading(running);
}

void KGet::settingsChanged()
{
    kDebug(5001);

    foreach (TransferFactory *factory, m_transferFactories)
    {
        factory->settingsChanged();
    }
    
    m_jobManager->settingsChanged();
    m_scheduler->settingsChanged();
}

QList<TransferGroupHandler*> KGet::groupsFromExceptions(const KUrl &filename)
{
    QList<TransferGroupHandler*> handlers;
    foreach (TransferGroupHandler * handler, allTransferGroups()) {
        const QStringList patterns = handler->regExp().pattern().split(',');//FIXME 4.5 add a tooltip: "Enter a list of foo separated by ," and then do split(i18nc("used as separator in a list, translate to the same thing you translated \"Enter a list of foo separated by ,\"", ","))
        if (matchesExceptions(filename, patterns)) {
            handlers.append(handler);
        }
    }

    return handlers;
}

bool KGet::matchesExceptions(const KUrl &sourceUrl, const QStringList &patterns)
{
    foreach (const QString &pattern, patterns) {
        const QString trimmedPattern = pattern.trimmed();
        if (trimmedPattern.isEmpty()) {
            continue;
        }
        QRegExp regExp = QRegExp(trimmedPattern);

        //try with Regular Expression first
        regExp.setPatternSyntax(QRegExp::RegExp2);
        regExp.setCaseSensitivity(Qt::CaseInsensitive);
        if (regExp.exactMatch(sourceUrl.url())) {
            return true;
        }

        //now try with wildcards
        if (!regExp.pattern().isEmpty() && !regExp.pattern().contains('*')) {
            regExp.setPattern('*' + regExp.pattern());
        }

        regExp.setPatternSyntax(QRegExp::Wildcard);
        regExp.setCaseSensitivity(Qt::CaseInsensitive);

        if (regExp.exactMatch(sourceUrl.url())) {
            return true;
        }
    }

    return false;
}

void KGet::setGlobalDownloadLimit(int limit)
{
    m_scheduler->setDownloadLimit(limit);
}

void KGet::setGlobalUploadLimit(int limit)
{
    m_scheduler->setUploadLimit(limit);
}

void KGet::calculateGlobalSpeedLimits()
{
    //if (m_scheduler->downloadLimit())//TODO: Remove this and the both other hacks in the 2 upper functions with a better replacement
        m_scheduler->calculateDownloadLimit();
    //if (m_scheduler->uploadLimit())
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
TransferGroupScheduler * KGet::m_scheduler = 0;
MainWindow * KGet::m_mainWindow = 0;
KUiServerJobs * KGet::m_jobManager = 0;
TransferHistoryStore * KGet::m_store = 0;
bool KGet::m_hasConnection = true;
#ifdef HAVE_NEPOMUK
    NepomukController *KGet::m_nepomukController = 0;
#endif

// ------ PRIVATE FUNCTIONS ------
KGet::KGet()
{
#ifdef HAVE_NEPOMUK
    Nepomuk::ResourceManager::instance()->init();
#endif

    m_scheduler = new TransferGroupScheduler();
    m_transferTreeModel = new TransferTreeModel(m_scheduler);
    m_selectionModel = new TransferTreeSelectionModel(m_transferTreeModel);

    QObject::connect(m_transferTreeModel, SIGNAL(transfersAddedEvent(QList<TransferHandler*>)),
                     m_jobManager,        SLOT(slotTransfersAdded(QList<TransferHandler*>)));
    QObject::connect(m_transferTreeModel, SIGNAL(transfersAboutToBeRemovedEvent(QList<TransferHandler*>)),
                     m_jobManager,        SLOT(slotTransfersAboutToBeRemoved(QList<TransferHandler*>)));
    QObject::connect(m_transferTreeModel, SIGNAL(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)),
                     m_jobManager,        SLOT(slotTransfersChanged(QMap<TransferHandler*,Transfer::ChangesFlags>)));

    //check if there is a connection
    const Solid::Networking::Status status = Solid::Networking::status();
    KGet::setHasNetworkConnection((status == Solid::Networking::Connected) || (status == Solid::Networking::Unknown));
            
    //Load all the available plugins
    loadPlugins();

    //Create the default group
    addGroup(i18n("My Downloads"));
}

KGet::~KGet()
{
    kDebug();
    delete m_transferTreeModel;
    delete m_jobManager;  //This one must always be before the scheduler otherwise the job manager can't remove the notifications when deleting.
    delete m_scheduler;
    delete m_store;

#ifdef HAVE_NEPOMUK
    delete m_nepomukController;
#endif
}

TransferHandler * KGet::createTransfer(const KUrl &src, const KUrl &dest, const QString& groupName, 
                          bool start, const QDomElement * e)
{
    QList<TransferHandler*> transfer = createTransfers(QList<TransferData>() << TransferData(src, dest, groupName, start, e));
    return (transfer.isEmpty() ? 0 : transfer.first());
}

QList<TransferHandler*> KGet::createTransfers(const QList<TransferData> &dataItems)
{
    QList<TransferHandler*> handlers;
    if (dataItems.isEmpty()) {
        return handlers;
    }

    QList<bool> start;
    QHash<TransferGroup*, QList<Transfer*> > groups;

    QStringList urlsFailed;
    foreach (const TransferData &data, dataItems) {
        kDebug(5001) << "srcUrl=" << data.src << " destUrl=" << data.dest << " group=" << data.groupName;

        TransferGroup *group = m_transferTreeModel->findGroup(data.groupName);
        if (!group) {
            kDebug(5001) << "KGet::createTransfer  -> group not found";
            group = m_transferTreeModel->transferGroups().first();
        }

        Transfer *newTransfer = 0;
        foreach (TransferFactory *factory, m_transferFactories) {
            kDebug(5001) << "Trying plugin   n.plugins=" << m_transferFactories.size();
            if ((newTransfer = factory->createTransfer(data.src, data.dest, group, m_scheduler, data.e))) {
    //             kDebug(5001) << "KGet::createTransfer   ->   CREATING NEW TRANSFER ON GROUP: _" << group->name() << "_";
                newTransfer->create();
                newTransfer->load(data.e);
                handlers << newTransfer->handler();
                groups[group] << newTransfer;
                start << data.start;
                break;
            }
        }
        if (!newTransfer) {
            urlsFailed << data.src.url();
            kWarning(5001) << "Warning! No plugin found to handle" << data.src;
        }
    }

    //show urls that failed
    if (!urlsFailed.isEmpty()) {
        QString message = i18np("<p>The following URL cannot be downloaded, its protocol is not supported by KGet:</p>",              
                                "<p>The following URLs cannot be downloaded, their protocols are not supported by KGet:</p>",
                                urlsFailed.count());

        QString content = urlsFailed.takeFirst();
        foreach (const QString &url, urlsFailed) {
            content += '\n' + url;
        }
        content = QString("<p style=\"font-size: small;\">%1</p>").arg(content);

        KGet::showNotification(m_mainWindow, "error", message + content, "dialog-error", i18n("Protocol unsupported"));
    }

    //add the created transfers to the model and start them if specified
    QHash<TransferGroup*, QList<Transfer*> >::const_iterator it;
    QHash<TransferGroup*, QList<Transfer*> >::const_iterator itEnd = groups.constEnd();
    for (it = groups.constBegin(); it != itEnd; ++it) {
        KGet::model()->addTransfers(it.value(), it.key());
    }
    for (int i = 0; i < handlers.count(); ++i) {
        if (start[i]) {
            handlers[i]->start();
        }
    }

    return handlers;//TODO implement error message if it is 0, or should the addTransfers stuff do that, in case if the numer of returned items does not match the number of sent data?
}

TransferDataSource * KGet::createTransferDataSource(const KUrl &src, const QDomElement &type, QObject *parent)
{
    kDebug(5001);

    TransferDataSource *dataSource;
    foreach (TransferFactory *factory, m_transferFactories)
    {
        dataSource = factory->createTransferDataSource(src, type, parent);
        if(dataSource)
            return dataSource;
    }
    return 0;
}

QString KGet::generalDestDir(bool preferXDGDownloadDir)
{
    QString dir = Settings::lastDirectory();

    if (preferXDGDownloadDir) {
        dir = KGlobalSettings::downloadPath();
    }

    return dir;
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
        newtransfer = newtransfer.trimmed();    //Remove any unnecessary space at the beginning and/or end
        
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
    QString destDir = KFileDialog::getExistingDirectory(generalDestDir());
    Settings::setLastDirectory(destDir);

    return destDir;
}

KUrl KGet::destFileInputDialog(QString destDir, const QString& suggestedFileName) // krazy:exclude=passbyvalue
{
    if (destDir.isEmpty())
        destDir = generalDestDir();

    // Use the destination name if not empty...
    KUrl startLocation(destDir);
    if (!suggestedFileName.isEmpty()) {
        startLocation.addPath(suggestedFileName);
    }

    KUrl destUrl = KFileDialog::getSaveUrl(startLocation, QString(), m_mainWindow, i18n("Save As"));
    if (!destUrl.isEmpty()) {
        Settings::setLastDirectory(destUrl.directory(KUrl::ObeyTrailingSlash));
    }

    return destUrl;
}

bool KGet::isValidSource(const KUrl &source)
{
    // Check if the URL is well formed
    if (!source.isValid()) {
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Malformed URL:\n%1", source.prettyUrl()));

        return false;
    }
    // Check if the URL contains the protocol
    if (source.protocol().isEmpty()){
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Malformed URL, protocol missing:\n%1", source.prettyUrl()));

        return false;
    }
    // Check if a transfer with the same url already exists
    Transfer * transfer = m_transferTreeModel->findTransfer( source );
    if (transfer)
    {
        if (transfer->status() == Job::Finished) {
            // transfer is finished, ask if we want to download again
            if (KMessageBox::questionYesNoCancel(0,
                    i18n("You have already completed a download from the location: \n\n%1\n\nDownload it again?", source.prettyUrl()),
                    i18n("Download it again?"), KStandardGuiItem::yes(),
                    KStandardGuiItem::no(), KStandardGuiItem::cancel())
                    == KMessageBox::Yes) {
                transfer->stop();
                KGet::delTransfer(transfer->handler());
                return true;
            }
            else
                return false;
        }
        else {
            if (KMessageBox::warningYesNoCancel(0,
                    i18n("You have a download in progress from the location: \n\n%1\n\nDelete it and download again?", source.prettyUrl()),
                    i18n("Delete it and download again?"), KStandardGuiItem::yes(),
                    KStandardGuiItem::no(), KStandardGuiItem::cancel())
                    == KMessageBox::Yes) {
                transfer->stop();
                KGet::delTransfer(transfer->handler());
                return true;
            }
            else
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

KUrl KGet::getValidDestUrl(const KUrl& destDir, const KUrl &srcUrl)
{
    kDebug() << "Source Url" << srcUrl << "Destination" << destDir;
    if ( !isValidDestDirectory(destDir.toLocalFile()) )
        return KUrl();

    KUrl destUrl = destDir;

    if (QFileInfo(destUrl.toLocalFile()).isDir())
    {
        QString filename = srcUrl.fileName();
        if (filename.isEmpty())
            filename = KUrl::toPercentEncoding( srcUrl.prettyUrl(), "/" );
        destUrl.adjustPath( KUrl::AddTrailingSlash );
        destUrl.setFileName( filename );
    }
    
    Transfer * existingTransferDest = m_transferTreeModel->findTransferByDestination(destUrl);
    QPointer<KIO::RenameDialog> dlg = 0;

    if (existingTransferDest) {
        if (existingTransferDest->status() == Job::Finished) {
            if (KMessageBox::questionYesNoCancel(0,
                    i18n("You have already downloaded that file from another location.\n\nDownload and delete the previous one?"),
                    i18n("File already downloaded. Download anyway?"), KStandardGuiItem::yes(),
                    KStandardGuiItem::no(), KStandardGuiItem::cancel())
                    == KMessageBox::Yes) {
                existingTransferDest->stop();
                KGet::delTransfer(existingTransferDest->handler());
                //start = true;
            } else 
                return KUrl();
        } else {
            dlg = new KIO::RenameDialog( m_mainWindow, i18n("You are already downloading the same file"/*, destUrl.prettyUrl()*/), srcUrl,
                                     destUrl, KIO::M_MULTI );
        }
    } else if (srcUrl == destUrl) {
        dlg = new KIO::RenameDialog(m_mainWindow, i18n("File already exists"), srcUrl,
                                    destUrl, KIO::M_MULTI);
    } else if (destUrl.isLocalFile() && QFile::exists(destUrl.toLocalFile())) {
        dlg = new KIO::RenameDialog( m_mainWindow, i18n("File already exists"), srcUrl,
                                     destUrl, KIO::M_OVERWRITE );          
    }

    if (dlg) {
        int result = dlg->exec();

        if (result == KIO::R_RENAME || result == KIO::R_OVERWRITE)
            destUrl = dlg->newDestUrl();
        else {
            delete(dlg);
            return KUrl();
        }

        delete(dlg);
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

    foreach (KService::Ptr service, services)
    {
        KPluginInfo info(service);
        info.load(plugins);

        if( !info.isPluginEnabled() ) {
            kDebug(5001) << "TransferFactory plugin (" << service->library()
                             << ") found, but not enabled";
            continue;
        }

        KGetPlugin * plugin;
        if( (plugin = createPluginFromService(service)) != 0 )
        {
            const QString pluginName = info.name();

            pluginList.prepend(plugin);
            kDebug(5001) << "TransferFactory plugin (" << (service)->library()
                         << ") found and added to the list of available plugins";
        }
        else
        {
            kDebug(5001) << "Error loading TransferFactory plugin ("
                         << service->library() << ")";
        }
    }

    foreach (KGetPlugin *plugin, pluginList)
    {
        m_transferFactories.append(qobject_cast<TransferFactory *>(plugin));
    }

    kDebug(5001) << "Number of factories = " << m_transferFactories.size();
}


void KGet::setHasNetworkConnection(bool hasConnection)
{
    kDebug(5001) << "Existing internet connection:" << hasConnection << "old:" << m_hasConnection;
    if (hasConnection == m_hasConnection) {
        return;
    }
    m_hasConnection = hasConnection;
    const bool initialState = m_scheduler->hasRunningJobs();
    m_scheduler->setHasNetworkConnection(hasConnection);
    const bool finalState = m_scheduler->hasRunningJobs();

    if (initialState != finalState) {
        if (hasConnection) {
            KGet::showNotification(m_mainWindow, "notification",
                                   i18n("Internet connection established, resuming transfers."),
                                   "dialog-info");

        } else {
            KGet::showNotification(m_mainWindow, "notification",
                                   i18n("No internet connection, stopping transfers."),
                                   "dialog-info");
        }
    }
}

KGetPlugin * KGet::createPluginFromService( const KService::Ptr &service )
{
    //try to load the specified library
    KPluginFactory *factory = KPluginLoader(service->library()).factory();

    if (!factory)
    {
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Plugin loader could not load the plugin: %1.", service->library()),
                               "dialog-info");
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
        QFileInfo info( url.toLocalFile() );
        if ( info.isDir() )
        {
            KGet::showNotification(m_mainWindow, "notification",
                                   i18n("Not deleting\n%1\nas it is a directory.", url.prettyUrl()),
                                   "dialog-info");
            return false;
        }
        KIO::NetAccess::del( url, 0L );
        return true;
    }

    else
        KGet::showNotification(m_mainWindow, "notification",
                               i18n("Not deleting\n%1\nas it is not a local file.", url.prettyUrl()),
                               "dialog-info");
    return false;
}

KNotification *KGet::showNotification(QWidget *parent, const QString &eventType,
                            const QString &text, const QString &icon, const QString &title, const KNotification::NotificationFlags &flags)
{
    return KNotification::event(eventType, title, text, KIcon(icon).pixmap(KIconLoader::SizeMedium), parent, flags);
}

GenericObserver::GenericObserver(QObject *parent)
  : QObject(parent),
    m_save(0),
    m_finishAction(0)
{
    connect(KGet::model(), SIGNAL(groupRemovedEvent(TransferGroupHandler*)), SLOT(groupRemovedEvent(TransferGroupHandler*)));
    connect(KGet::model(), SIGNAL(transfersAddedEvent(QList<TransferHandler*>)),
                           SLOT(transfersAddedEvent(QList<TransferHandler*>)));
    connect(KGet::model(), SIGNAL(groupAddedEvent(TransferGroupHandler*)), SLOT(groupAddedEvent(TransferGroupHandler*)));
    connect(KGet::model(), SIGNAL(transfersRemovedEvent(QList<TransferHandler*>)),
                           SLOT(transfersRemovedEvent(QList<TransferHandler*>)));
    connect(KGet::model(), SIGNAL(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)), 
                           SLOT(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)));
    connect(KGet::model(), SIGNAL(groupsChangedEvent(QMap<TransferGroupHandler*,TransferGroup::ChangesFlags>)), 
                           SLOT(groupsChangedEvent(QMap<TransferGroupHandler*,TransferGroup::ChangesFlags>)));
    connect(KGet::model(), SIGNAL(transferMovedEvent(TransferHandler*,TransferGroupHandler*)),
                           SLOT(transferMovedEvent(TransferHandler*,TransferGroupHandler*)));
    connect(Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
                         this, SLOT(slotNetworkStatusChanged(Solid::Networking::Status)));

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

void GenericObserver::transfersAddedEvent(const QList<TransferHandler*> &handlers)
{
    Q_UNUSED(handlers)
    requestSave();
    KGet::calculateGlobalSpeedLimits();
    KGet::checkSystemTray();
}

void GenericObserver::transfersRemovedEvent(const QList<TransferHandler*> &handlers)
{
    Q_UNUSED(handlers)
    requestSave();
    KGet::calculateGlobalSpeedLimits();
    KGet::checkSystemTray();
}

void GenericObserver::transferMovedEvent(TransferHandler *transfer, TransferGroupHandler *group)
{
    Q_UNUSED(transfer)
    Q_UNUSED(group)
    requestSave();
    KGet::calculateGlobalSpeedLimits();
}

void GenericObserver::requestSave()
{
    if (!m_save) {
        m_save = new QTimer(this);
        m_save->setInterval(5000);
        connect(m_save, SIGNAL(timeout()), this, SLOT(slotSave()));
    }

    //save regularly if there are running jobs
    m_save->setSingleShot(!KGet::m_scheduler->hasRunningJobs());

    if (!m_save->isActive()) {
        m_save->start();
    }
}

void GenericObserver::slotSave()
{
    KGet::save();
}

void GenericObserver::transfersChangedEvent(QMap<TransferHandler*, Transfer::ChangesFlags> transfers)
{
    bool checkSysTray = false;
    bool allFinished = true;
    QMap<TransferHandler*, Transfer::ChangesFlags>::const_iterator it;
    QMap<TransferHandler*, Transfer::ChangesFlags>::const_iterator itEnd = transfers.constEnd();
    for (it = transfers.constBegin(); it != itEnd; ++it)
    {
        TransferHandler::ChangesFlags transferFlags = *it;
        TransferHandler *transfer = it.key();
        
        if (transferFlags & Transfer::Tc_Status) {
            if ((transfer->status() == Job::Finished)   && (transfer->startStatus() != Job::Finished)) {
                KGet::showNotification(KGet::m_mainWindow, "finished",
                                       i18n("<p>The following file has finished downloading:</p><p style=\"font-size: small;\">%1</p>", transfer->dest().fileName()),
                                       "kget", i18n("Download completed"));
            } else if (transfer->status() == Job::Running) {
                KGet::showNotification(KGet::m_mainWindow, "started",
                                       i18n("<p>The following transfer has been started:</p><p style=\"font-size: small;\">%1</p>", transfer->source().pathOrUrl()),
                                       "kget", i18n("Download started"));
            } else if (transfer->status() == Job::Aborted && transfer->error().type != Job::AutomaticRetry) {
                KNotification * notification = KNotification::event("error", i18n("Error"), i18n("<p>There has been an error in the following transfer:</p><p style=\"font-size: small;\">%1</p>"
                                            "<p>The error message is:</p><p style=\"font-size: small;\">%2</p>", transfer->source().pathOrUrl(), transfer->error().text), 
                                             transfer->error().pixmap, KGet::m_mainWindow, KNotification::CloseOnTimeout);
                if (transfer->error().type == Job::ManualSolve) {
                    m_notifications.insert(notification, transfer);
                    notification->setActions(QStringList() << i18n("Resolve"));
                    connect(notification, SIGNAL(action1Activated()), SLOT(slotResolveTransferError()));
                    connect(notification, SIGNAL(closed()), SLOT(slotNotificationClosed()));
                }
            }
        }

        if (transferFlags & Transfer::Tc_Status) {
            checkSysTray = true;
            requestSave();
        }

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

        if ((transfer->status() == Job::Finished) || (transfer->status() == Job::FinishedKeepAlive)) {
            requestSave();
        } else {
            allFinished = false;
        }
    }
    allFinished = allFinished && allTransfersFinished();

    if (checkSysTray)
        KGet::checkSystemTray();

    //only perform after finished actions if actually the status changed (that is the
    //case if checkSysTray is set to true)
    if (checkSysTray && Settings::afterFinishActionEnabled() && allFinished)
    {
        kDebug(5001) << "All finished";
        KNotification *notification = 0;

        if (!m_finishAction) {
            m_finishAction = new QTimer(this);
            m_finishAction->setSingleShot(true);
            m_finishAction->setInterval(10000);
            connect(m_finishAction, SIGNAL(timeout()), this, SLOT(slotAfterFinishAction()));
        }

        switch (Settings::afterFinishAction()) {
            case KGet::Quit:
                notification = KGet::showNotification(KGet::m_mainWindow, "notification", i18n("KGet is now closing, as all downloads have completed."), "kget", "KGet", KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
                break;
#ifdef HAVE_KWORKSPACE
            case KGet::Shutdown:
                notification = KGet::showNotification(KGet::m_mainWindow, "notification", i18n("The computer will now turn off, as all downloads have completed."), "system-shutdown", i18nc("Shutting down computer", "Shutdown"), KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
                break;
            case KGet::Hibernate:
                notification = KGet::showNotification(KGet::m_mainWindow, "notification", i18n("The computer will now suspend to disk, as all downloads have completed."), "system-suspend-hibernate", i18nc("Hibernating computer", "Hibernating"), KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
                break;
            case KGet::Suspend:
                notification = KGet::showNotification(KGet::m_mainWindow, "notification", i18n("The computer will now suspend to RAM, as all downloads have completed."), "system-suspend", i18nc("Suspending computer", "Suspending"), KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
                break;
#endif
            default:
                break;
        }

        if (notification) {
            notification->setActions(QStringList() << i18nc("abort the proposed action", "Abort"));
            connect(notification, SIGNAL(action1Activated()), this, SLOT(slotAbortAfterFinishAction()));
            connect(m_finishAction, SIGNAL(timeout()), notification, SLOT(close()));

            if (!m_finishAction->isActive()) {
                m_finishAction->start();
            }
        }
    } else if (allFinished) {
        KGet::showNotification(KGet::m_mainWindow, "finishedall",
                               i18n("<p>All transfers have been finished.</p>"),
                               "kget", i18n("Downloads completed"));
    }
}

void GenericObserver::slotResolveTransferError()
{
    KNotification * notification = static_cast<KNotification*>(QObject::sender());
    if (notification) {
        TransferHandler * handler = m_notifications[notification];
        kDebug() << "Resolve error for" << handler->source().pathOrUrl() << "with id" << handler->error().id;
        handler->resolveError(handler->error().id);
        m_notifications.remove(notification);
    }
}

void GenericObserver::slotNotificationClosed()
{
    kDebug() << "Remove notification";
    KNotification * notification = static_cast<KNotification*>(QObject::sender());
    if (notification)
        m_notifications.remove(notification);
}

void GenericObserver::slotNetworkStatusChanged(const Solid::Networking::Status &status)
{
    KGet::setHasNetworkConnection((status == Solid::Networking::Connected) || (status == Solid::Networking::Unknown));
}

void GenericObserver::groupsChangedEvent(QMap<TransferGroupHandler*, TransferGroup::ChangesFlags> groups)
{
    bool recalculate = false;
    foreach (const TransferGroup::ChangesFlags &flags, groups)
    {
        if (flags & TransferGroup::Gc_Percent || flags & TransferGroup::Gc_Status) {
            recalculate = true;
            break;
        }
    }
    kDebug() << "Recalculate limits?" << recalculate;
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
            if ((transfer->status() != Job::Finished) && (transfer->status() != Job::FinishedKeepAlive)) {
                quitFlag = false;
            }
            if ((transfer->status() == Job::Finished || transfer->status() == Job::FinishedKeepAlive) &&
                (transfer->startStatus() != Job::Finished && transfer->startStatus() != Job::FinishedKeepAlive)) {
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

void GenericObserver::slotAfterFinishAction()
{
    kDebug(5001);

    switch (Settings::afterFinishAction()) {
        case KGet::Quit:
            kDebug(5001) << "Quit Kget.";
            QTimer::singleShot(0, KGet::m_mainWindow, SLOT(slotQuit()));
            break;
    #ifdef HAVE_KWORKSPACE
        case KGet::Shutdown:
            QTimer::singleShot(0, KGet::m_mainWindow, SLOT(slotQuit()));
            KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmNo,
                        KWorkSpace::ShutdownTypeHalt,
                        KWorkSpace::ShutdownModeForceNow);
            break;
        case KGet::Hibernate: {
           QDBusMessage call;
           call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                 "/org/kde/Solid/PowerManagement",
                                                 "org.kde.Solid.PowerManagement",
                                                 "suspendToRam");
           QDBusConnection::sessionBus().asyncCall(call);
            break;
        }
        case KGet::Suspend: {
           QDBusMessage call;
           call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                 "/org/kde/Solid/PowerManagement",
                                                 "org.kde.Solid.PowerManagement",
                                                 "suspendToDisk");
           QDBusConnection::sessionBus().asyncCall(call);
            break;
        }
    #endif
        default:
            break;
    }
}

void GenericObserver::slotAbortAfterFinishAction()
{
    kDebug(5001);

    m_finishAction->stop();
}

#include "kget.moc"
