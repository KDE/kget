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

#include "kget_debug.h"

#include <algorithm>
#include <iostream>

#include <KMessageBox>
#include <KLocalizedString>
#include <KIconLoader>
#include <KActionCollection>
#include <KIO/RenameDialog>
#include <KIO/DeleteJob>
#include <KSharedConfig>
#include <KPluginInfo>
#include <KConfigDialog>
#include <KPluginMetaData>

#include <QAbstractItemView>
#include <QApplication>
#include <QDomElement>
#include <QClipboard>
#include <QFileDialog>
#include <QInputDialog>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QTemporaryFile>
#include <QTextStream>

#ifdef HAVE_KWORKSPACE
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <kworkspace.h>
#endif


KGet::TransferData::TransferData(const QUrl &source, const QUrl &destination, const QString& group, bool doStart, const QDomElement *element)
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
    qCDebug(KGET_DEBUG);

    // Check if a group with that name already exists
    if (m_transferTreeModel->findGroup(groupName))
        return false;

    auto * group = new TransferGroup(m_transferTreeModel, m_scheduler, groupName);
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

TransferHandler * KGet::addTransfer(QUrl srcUrl, QString destDir, QString suggestedFileName, // krazy:exclude=passbyvalue
                                    QString groupName, bool start)
{
    srcUrl = mostLocalUrl(srcUrl);
    // Note: destDir may actually be a full path to a file :-(
    qCDebug(KGET_DEBUG) << "Source:" << srcUrl.url() << ", dest: " << destDir << ", sugg file: " << suggestedFileName;

    QUrl destUrl; // the final destination, including filename

    if ( srcUrl.isEmpty() )
    {
        //No src location: we let the user insert it manually
        srcUrl = urlInputDialog();
        if( srcUrl.isEmpty() )
            return nullptr;
    }
    
    if ( !isValidSource( srcUrl ) )
        return nullptr;

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
        QUrl targetUrl = QUrl::fromLocalFile(destDir);
        QString directory = targetUrl.adjusted(QUrl::RemoveFilename).path();
        QString fileName = targetUrl.fileName(QUrl::PrettyDecoded);
        if (QFileInfo(directory).isDir() && !fileName.isEmpty()) {
            destDir = directory;
            suggestedFileName = fileName;
        }
    }

    if (suggestedFileName.isEmpty())
    {
        confirmDestination = true;
        suggestedFileName = srcUrl.fileName(QUrl::PrettyDecoded);
        if (suggestedFileName.isEmpty())
        {
            // simply use the full url as filename
            suggestedFileName = QUrl::toPercentEncoding( srcUrl.toDisplayString(), "/" );
        }
    }

    // now ask for confirmation of the entire destination url (dir + filename)
    if (confirmDestination || !isValidDestDirectory(destDir))
    {
        do 
        {
            destUrl = destFileInputDialog(destDir, suggestedFileName);
            if (destUrl.isEmpty())
                return nullptr;

            destDir = destUrl.adjusted(QUrl::RemoveFilename).path();
        } while (!isValidDestDirectory(destDir));
    } else {
        destUrl = QUrl::fromLocalFile(destDir + suggestedFileName);
    }
    destUrl = getValidDestUrl(destUrl, srcUrl);

    if (destUrl == QUrl())
        return nullptr;

    TransferHandler *transfer = createTransfer(srcUrl, destUrl, groupName, start);
    if (transfer) {
        KGet::showNotification(m_mainWindow, "added",
                                i18n("<p>The following transfer has been added to the download list:</p><p style=\"font-size: small;\">%1</p>", transfer->source().toString()),
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
        QUrl srcUrl = QUrl(e.attribute("Source"));
        QUrl destUrl = QUrl(e.attribute("Dest"));
        data << TransferData(srcUrl, destUrl, groupName, false, &e);

        qCDebug(KGET_DEBUG) << "src=" << srcUrl << " dest=" << destUrl << " group=" << groupName;
    }

    return createTransfers(data);
}

const QList<TransferHandler *> KGet::addTransfer(QList<QUrl> srcUrls, QString destDir, QString groupName, bool start)
{
    QList<QUrl> urlsToDownload;

    QList<QUrl>::iterator it = srcUrls.begin();
    QList<QUrl>::iterator itEnd = srcUrls.end();
    
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
        }

        return addedTransfers;
    }

    QUrl destUrl;

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
        destUrl = getValidDestUrl(QUrl::fromLocalFile(destDir), *it);

        if (destUrl == QUrl())
            continue;

        data << TransferData(*it, destUrl, groupName, start);
    }

    QList<TransferHandler*> transfers = createTransfers(data);
    if (!transfers.isEmpty()) {
        QString urls = transfers[0]->source().toString();
        for (int i = 1; i < transfers.count(); ++i) {
            urls += '\n' + transfers[i]->source().toString();
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
     QUrl src = transfer->source();
     QString dest = transfer->dest().toLocalFile();
     QString destFile = transfer->dest().fileName();

     KGet::delTransfer(transfer);
     KGet::addTransfer(src, dest, destFile, group, true);
}

QList<TransferHandler *> KGet::selectedTransfers()
{
//     qCDebug(KGET_DEBUG) << "KGet::selectedTransfers";

    QList<TransferHandler *> selectedTransfers;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();
    //sort the indexes as this can speed up operations like deleting etc.
    std::sort(selectedIndexes.begin(), selectedIndexes.end());

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

QList<TransferGroupHandler *> KGet::selectedTransferGroups()
{
    QList<TransferGroupHandler *> selectedTransferGroups;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();

    foreach(const QModelIndex &currentIndex, selectedIndexes)
    {
        ModelItem * item = m_transferTreeModel->itemFromIndex(currentIndex);
        if (item->isGroup()) {
            TransferGroupHandler *group = item->asGroup()->groupHandler();
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

void KGet::load( QString filename ) // krazy:exclude=passbyvalue
{
    qCDebug(KGET_DEBUG) << "(" << filename << ")";

    if(filename.isEmpty()) {
        filename = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        // make sure that the DataLocation directory exists (earlier this used to be handled by KStandardDirs)
        if (!QFileInfo::exists(filename)) {
            QDir().mkpath(filename);
        }
        filename += QStringLiteral("/transfers.kgt");
    }

    QTemporaryFile tmpFile;

    QUrl url = QUrl(filename);
    if (url.scheme().isEmpty())
        url.setScheme("file");
    KIO::StoredTransferJob * job = KIO::storedGet(url);
    job->exec();
    if (job->data().isEmpty() || !tmpFile.open()) {
        qCDebug(KGET_DEBUG) << "Transferlist empty or cannot open temporary file";
        if (m_transferTreeModel->transferGroups().isEmpty()) //Create the default group
            addGroup(i18n("My Downloads"));
        return;
    }
    tmpFile.write(job->data());
    tmpFile.close();

    QDomDocument doc;

    qCDebug(KGET_DEBUG) << "file:" << tmpFile.fileName();

    if(doc.setContent(&tmpFile))
    {
        QDomElement root = doc.documentElement();

        QDomNodeList nodeList = root.elementsByTagName("TransferGroup");
        int nItems = nodeList.length();

        for( int i = 0 ; i < nItems ; i++ )
        {
            TransferGroup * foundGroup = m_transferTreeModel->findGroup( nodeList.item(i).toElement().attribute("Name") );

            qCDebug(KGET_DEBUG) << "KGet::load  -> group = " << nodeList.item(i).toElement().attribute("Name");

            if( !foundGroup )
            {
                qCDebug(KGET_DEBUG) << "KGet::load  -> group not found";

                auto * newGroup = new TransferGroup(m_transferTreeModel, m_scheduler);

                m_transferTreeModel->addGroup(newGroup);

                newGroup->load(nodeList.item(i).toElement());
            }
            else
            {
                qCDebug(KGET_DEBUG) << "KGet::load  -> group found";

                //A group with this name already exists.
                //Integrate the group's transfers with the ones read from file
                foundGroup->load(nodeList.item(i).toElement());
            }
        }
    }
    else
    {
        qCWarning(KGET_DEBUG) << "Error reading the transfers file";
    }
    
    if (m_transferTreeModel->transferGroups().isEmpty()) //Create the default group
        addGroup(i18n("My Downloads"));

    new GenericObserver(m_mainWindow);
}

void KGet::save( QString filename, bool plain ) // krazy:exclude=passbyvalue
{
    if ( !filename.isEmpty()
        && QFile::exists( filename )
        && (KMessageBox::questionYesNoCancel(nullptr,
                i18n("The file %1 already exists.\nOverwrite?", filename),
                i18n("Overwrite existing file?"), KStandardGuiItem::yes(),
                KStandardGuiItem::no(), KStandardGuiItem::cancel(), "QuestionFilenameExists" )
                != KMessageBox::Yes) )
        return;

    if(filename.isEmpty()) {
        filename = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        // make sure that the DataLocation directory exists (earlier this used to be handled by KStandardDirs)
        if (!QFileInfo::exists(filename)) {
            QDir().mkpath(filename);
        }
        filename += QStringLiteral("/transfers.kgt");
    }
    
    qCDebug(KGET_DEBUG) << "Save transferlist to " << filename;

    QSaveFile file(filename);
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        //qCWarning(KGET_DEBUG)<<"Unable to open output file when saving";
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Unable to save to: %1", filename));
        return;
    }

    if (plain) {
        QTextStream out(&file);
        foreach(TransferHandler *handler, allTransfers()) {
            out << handler->source().toString() << '\n';
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
    file.commit();
}

QList<TransferFactory*> KGet::factories()
{
    return m_transferFactories;
}

KPluginInfo::List KGet::pluginInfos()
{
    return m_pluginInfoList;
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
        qDebug() << group->name();
        transfergroups << group->handler();
    }
    return transfergroups;
}

TransferHandler * KGet::findTransfer(const QUrl &src)
{
    Transfer *transfer = KGet::m_transferTreeModel->findTransfer(src);
    if (transfer)
    {
        return transfer->handler();
    }
    return nullptr;
}

TransferGroupHandler * KGet::findGroup(const QString &name)
{
    TransferGroup *group = KGet::m_transferTreeModel->findGroup(name);
    if (group)
    {
        return group->handler();
    }
    return nullptr;
}

void KGet::checkSystemTray()
{
    qCDebug(KGET_DEBUG);
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
    qCDebug(KGET_DEBUG);

    foreach (TransferFactory *factory, m_transferFactories)
    {
        factory->settingsChanged();
    }
    
    m_jobManager->settingsChanged();
    m_scheduler->settingsChanged();
}

QList<TransferGroupHandler*> KGet::groupsFromExceptions(const QUrl &filename)
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

bool KGet::matchesExceptions(const QUrl &sourceUrl, const QStringList &patterns)
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
KPluginInfo::List KGet::m_pluginInfoList;
TransferGroupScheduler * KGet::m_scheduler = nullptr;
MainWindow * KGet::m_mainWindow = nullptr;
KUiServerJobs * KGet::m_jobManager = nullptr;
TransferHistoryStore * KGet::m_store = nullptr;
bool KGet::m_hasConnection = true;
// ------ PRIVATE FUNCTIONS ------
KGet::KGet()
{

    m_scheduler = new TransferGroupScheduler();
    m_transferTreeModel = new TransferTreeModel(m_scheduler);
    m_selectionModel = new TransferTreeSelectionModel(m_transferTreeModel);

    QObject::connect(m_transferTreeModel, SIGNAL(transfersAddedEvent(QList<TransferHandler*>)),
                     m_jobManager,        SLOT(slotTransfersAdded(QList<TransferHandler*>)));
    QObject::connect(m_transferTreeModel, &TransferTreeModel::transfersAboutToBeRemovedEvent,
                     m_jobManager,        &KUiServerJobs::slotTransfersAboutToBeRemoved);
    QObject::connect(m_transferTreeModel, SIGNAL(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)),
                     m_jobManager,        SLOT(slotTransfersChanged(QMap<TransferHandler*,Transfer::ChangesFlags>)));
            
    //Load all the available plugins
    loadPlugins();
}

KGet::~KGet()
{
    qDebug();
    delete m_transferTreeModel;
    delete m_jobManager;  //This one must always be before the scheduler otherwise the job manager can't remove the notifications when deleting.
    delete m_scheduler;
    delete m_store;

}

TransferHandler * KGet::createTransfer(const QUrl &src, const QUrl &dest, const QString& groupName, 
                          bool start, const QDomElement * e)
{
    QList<TransferHandler*> transfer = createTransfers(QList<TransferData>() << TransferData(src, dest, groupName, start, e));
    return (transfer.isEmpty() ? nullptr : transfer.first());
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
        qCDebug(KGET_DEBUG) << "srcUrl=" << data.src << " destUrl=" << data.dest << " group=" << data.groupName;

        TransferGroup *group = m_transferTreeModel->findGroup(data.groupName);
        if (!group) {
            qCDebug(KGET_DEBUG) << "KGet::createTransfer  -> group not found";
            group = m_transferTreeModel->transferGroups().first();
        }

        Transfer *newTransfer = nullptr;
        foreach (TransferFactory *factory, m_transferFactories) {
            qCDebug(KGET_DEBUG) << "Trying plugin   n.plugins=" << m_transferFactories.size() << factory->displayName();
            if ((newTransfer = factory->createTransfer(data.src, data.dest, group, m_scheduler, data.e))) {
    //             qCDebug(KGET_DEBUG) << "KGet::createTransfer   ->   CREATING NEW TRANSFER ON GROUP: _" << group->name() << "_";
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
            qCWarning(KGET_DEBUG) << "Warning! No plugin found to handle" << data.src;
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

TransferDataSource * KGet::createTransferDataSource(const QUrl &src, const QDomElement &type, QObject *parent)
{
    qCDebug(KGET_DEBUG);

    TransferDataSource *dataSource;
    foreach (TransferFactory *factory, m_transferFactories)
    {
        dataSource = factory->createTransferDataSource(src, type, parent);
        if(dataSource)
            return dataSource;
    }
    return nullptr;
}

QString KGet::generalDestDir(bool preferXDGDownloadDir)
{
    QString dir = Settings::lastDirectory();

    if (preferXDGDownloadDir) {
        dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }

    return dir;
}

QUrl KGet::urlInputDialog()
{
    QString newtransfer;
    bool ok = false;

    QUrl clipboardUrl = QUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
    if (clipboardUrl.isValid())
        newtransfer = clipboardUrl.url();

    while (!ok)
    {
        newtransfer = QInputDialog::getText(nullptr, i18n("New Download"), i18n("Enter URL:"), QLineEdit::Normal, newtransfer, &ok);
        newtransfer = newtransfer.trimmed();    //Remove any unnecessary space at the beginning and/or end
        
        if (!ok)
        {
            //user pressed cancel
            return QUrl();
        }

        QUrl src = QUrl(newtransfer);
        if(src.isValid())
            return src;
        else
            ok = false;
    }
    return QUrl();
}

QString KGet::destDirInputDialog()
{
    QString destDir = QFileDialog::getExistingDirectory(nullptr, i18nc("@title:window", "Choose Directory"), generalDestDir(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    Settings::setLastDirectory(destDir);

    return destDir;
}

QUrl KGet::destFileInputDialog(QString destDir, const QString& suggestedFileName) // krazy:exclude=passbyvalue
{
    if (destDir.isEmpty())
        destDir = generalDestDir();

    // Use the destination name if not empty...
    QUrl startLocation;
    if (!suggestedFileName.isEmpty()) {
        startLocation.setPath(destDir + suggestedFileName);
    } else {
        startLocation.setPath(destDir);
    }

    QUrl destUrl = QFileDialog::getSaveFileUrl(m_mainWindow, i18nc("@title:window", "Save As"), startLocation, QString());
    if (!destUrl.isEmpty()) {
        Settings::setLastDirectory(destUrl.adjusted(QUrl::RemoveFilename).path());
    }

    return destUrl;
}

bool KGet::isValidSource(const QUrl &source)
{
    // Check if the URL is well formed
    if (!source.isValid()) {
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Malformed URL:\n%1", source.toString()));

        return false;
    }
    // Check if the URL contains the protocol
    if (source.scheme().isEmpty()){
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Malformed URL, protocol missing:\n%1", source.toString()));

        return false;
    }
    // Check if a transfer with the same url already exists
    Transfer * transfer = m_transferTreeModel->findTransfer( source );
    if (transfer)
    {
        if (transfer->status() == Job::Finished) {
            // transfer is finished, ask if we want to download again
            if (KMessageBox::questionYesNoCancel(nullptr,
                    i18n("You have already completed a download from the location: \n\n%1\n\nDownload it again?", source.toString()),
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
            if (KMessageBox::warningYesNoCancel(nullptr,
                    i18n("You have a download in progress from the location: \n\n%1\n\nDelete it and download again?", source.toString()),
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
    qCDebug(KGET_DEBUG) << destDir;
    if (!QFileInfo(destDir).isDir())
    {
        if (QFileInfo(QUrl(destDir).adjusted(QUrl::RemoveFilename).toString()).isWritable())
            return (!destDir.isEmpty());
        if (!QFileInfo(QUrl(destDir).adjusted(QUrl::RemoveFilename).toString()).isWritable() && !destDir.isEmpty())
            KMessageBox::error(nullptr, i18n("Directory is not writable"));
    }
    else
    {
        if (QFileInfo(destDir).isWritable())
            return (!destDir.isEmpty());
        if (!QFileInfo(destDir).isWritable() && !destDir.isEmpty())
            KMessageBox::error(nullptr, i18n("Directory is not writable"));
    }
    return false;
}

QUrl KGet::getValidDestUrl(const QUrl& destDir, const QUrl &srcUrl)
{
    qDebug() << "Source Url" << srcUrl << "Destination" << destDir;
    if ( !isValidDestDirectory(destDir.toLocalFile()) )
        return QUrl();

    QUrl destUrl = destDir;

    if (QFileInfo(destUrl.toLocalFile()).isDir())
    {
        QString filename = srcUrl.fileName();
        if (filename.isEmpty())
            filename = QUrl::toPercentEncoding( srcUrl.toString(), "/" );
        destUrl = destUrl.adjusted( QUrl::RemoveFilename );
        destUrl.setPath(destUrl.path() + filename);
    }
    
    Transfer * existingTransferDest = m_transferTreeModel->findTransferByDestination(destUrl);
    QPointer<KIO::RenameDialog> dlg = nullptr;

    if (existingTransferDest) {
        if (existingTransferDest->status() == Job::Finished) {
            if (KMessageBox::questionYesNoCancel(nullptr,
                    i18n("You have already downloaded that file from another location.\n\nDownload and delete the previous one?"),
                    i18n("File already downloaded. Download anyway?"), KStandardGuiItem::yes(),
                    KStandardGuiItem::no(), KStandardGuiItem::cancel())
                    == KMessageBox::Yes) {
                existingTransferDest->stop();
                KGet::delTransfer(existingTransferDest->handler());
                //start = true;
            } else 
                return QUrl();
        } else {
            dlg = new KIO::RenameDialog( m_mainWindow, i18n("You are already downloading the same file"/*, destUrl.prettyUrl()*/), srcUrl,
                                     destUrl, KIO::RenameDialog_MultipleItems );
        }
    } else if (srcUrl == destUrl) {
        dlg = new KIO::RenameDialog(m_mainWindow, i18n("File already exists"), srcUrl,
                                    destUrl, KIO::RenameDialog_MultipleItems);
    } else if (destUrl.isLocalFile() && QFile::exists(destUrl.toLocalFile())) {
        dlg = new KIO::RenameDialog( m_mainWindow, i18n("File already exists"), srcUrl,
                                     destUrl, KIO::RenameDialog_Overwrite );          
    }

    if (dlg) {
        int result = dlg->exec();

        if (result == KIO::Result_Rename || result == KIO::Result_Overwrite)
            destUrl = dlg->newDestUrl();
        else {
            delete(dlg);
            return QUrl();
        }

        delete(dlg);
    }

    return destUrl;
}

void KGet::loadPlugins()
{
    m_transferFactories.clear();
    m_pluginInfoList.clear();

    // TransferFactory plugins
    const QVector<KPluginMetaData> offers = KPluginLoader::findPlugins(QStringLiteral("kget"), [](const KPluginMetaData& md) {
        return md.value(QStringLiteral("X-KDE-KGet-framework-version")) == QString::number(FrameworkVersion) &&
            md.value(QStringLiteral("X-KDE-KGet-rank")).toInt() > 0 &&
            md.value(QStringLiteral("X-KDE-KGet-plugintype")) == QStringLiteral("TransferFactory");
    });

    qCDebug(KGET_DEBUG) << "Found" << offers.size() << "plugins";

    //Here we use a QMap only to easily sort the plugins by rank
    QMap<int, KPluginMetaData> sortedOffers;

    for (const KPluginMetaData& md : offers)
    {
        sortedOffers[md.value("X-KDE-KGet-rank").toInt()] = md;
        qCDebug(KGET_DEBUG) << " TransferFactory plugin found:\n"<<
         "  rank = " << md.value("X-KDE-KGet-rank").toInt() << '\n' <<
         "  plugintype = " << md.value("X-KDE-KGet-plugintype");
    }

    //I must fill this pluginList before and my m_transferFactories list after.
    //This because calling the KLibLoader::globalLibrary() erases the static
    //members of this class (why?), such as the m_transferFactories list.
    QList<KGetPlugin *> pluginList;

    const KConfigGroup plugins = KConfigGroup(KSharedConfig::openConfig(), "Plugins");

    for (const KPluginMetaData& md : qAsConst(sortedOffers))
    {
        KPluginInfo info(md);
        info.load(plugins);
        m_pluginInfoList.prepend(info);

        if (!info.isPluginEnabled())
        {
            qCDebug(KGET_DEBUG) << "TransferFactory plugin (" << md.fileName()
                             << ") found, but not enabled";
            continue;
        }

        KGetPlugin* plugin = loadPlugin(md);
        if (plugin != nullptr)
        {
            const QString pluginName = info.name();

            pluginList.prepend(plugin);
            qCDebug(KGET_DEBUG) << "TransferFactory plugin (" << md.fileName()
                         << ") found and added to the list of available plugins";
        }
        else
        {
            qCDebug(KGET_DEBUG) << "Error loading TransferFactory plugin ("
                         << md.fileName() << ")";
        }
    }

    foreach (KGetPlugin* plugin, pluginList)
    {
        m_transferFactories.append(qobject_cast<TransferFactory*>(plugin));
    }

    qCDebug(KGET_DEBUG) << "Number of factories = " << m_transferFactories.size();
}


void KGet::setHasNetworkConnection(bool hasConnection)
{
    qCDebug(KGET_DEBUG) << "Existing internet connection:" << hasConnection << "old:" << m_hasConnection;
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

KGetPlugin* KGet::loadPlugin(const KPluginMetaData& md)
{
    KPluginFactory* factory = KPluginLoader(md.fileName()).factory();
    if (factory) {
        return factory->create<TransferFactory>(KGet::m_mainWindow);
    } else {
        KGet::showNotification(m_mainWindow, "error",
                               i18n("Plugin loader could not load the plugin: %1.", md.fileName()),
                               "dialog-info");
        qCCritical(KGET_DEBUG) << "KPluginFactory could not load the plugin:" << md.fileName();
        return nullptr;
    }
}

bool KGet::safeDeleteFile( const QUrl& url )
{
    if ( url.isLocalFile() )
    {
        QFileInfo info( url.toLocalFile() );
        if ( info.isDir() )
        {
            KGet::showNotification(m_mainWindow, "notification",
                                   i18n("Not deleting\n%1\nas it is a directory.", url.toString()),
                                   "dialog-info");
            return false;
        }
        KIO::DeleteJob * del = KIO::del(url);
        del->exec();
        return true;
    }

    else
        KGet::showNotification(m_mainWindow, "notification",
                               i18n("Not deleting\n%1\nas it is not a local file.", url.toString()),
                               "dialog-info");
    return false;
}

KNotification *KGet::showNotification(QWidget *parent, const QString &eventType,
                            const QString &text, const QString &icon, const QString &title, const KNotification::NotificationFlags &flags)
{
    return KNotification::event(eventType, title, text, QIcon::fromTheme(icon).pixmap(KIconLoader::SizeMedium), parent, flags);
}

GenericObserver::GenericObserver(QObject *parent)
  : QObject(parent),
    m_save(nullptr),
    m_finishAction(nullptr)
{
    //check if there is a connection
    KGet::setHasNetworkConnection(m_networkConfig.isOnline());
    
    connect(KGet::model(), &TransferTreeModel::groupRemovedEvent, this, &GenericObserver::groupRemovedEvent);
    connect(KGet::model(), SIGNAL(transfersAddedEvent(QList<TransferHandler*>)),
                           SLOT(transfersAddedEvent(QList<TransferHandler*>)));
    connect(KGet::model(), &TransferTreeModel::groupAddedEvent, this, &GenericObserver::groupAddedEvent);
    connect(KGet::model(), &TransferTreeModel::transfersRemovedEvent,
                           this, &GenericObserver::transfersRemovedEvent);
    connect(KGet::model(), SIGNAL(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)), 
                           SLOT(transfersChangedEvent(QMap<TransferHandler*,Transfer::ChangesFlags>)));
    connect(KGet::model(), SIGNAL(groupsChangedEvent(QMap<TransferGroupHandler*,TransferGroup::ChangesFlags>)), 
                           SLOT(groupsChangedEvent(QMap<TransferGroupHandler*,TransferGroup::ChangesFlags>)));
    connect(KGet::model(), &TransferTreeModel::transferMovedEvent,
                           this, &GenericObserver::transferMovedEvent);
    connect(&m_networkConfig, &QNetworkConfigurationManager::onlineStateChanged,
                         this, &GenericObserver::slotNetworkStatusChanged);

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
        connect(m_save, &QTimer::timeout, this, &GenericObserver::slotSave);
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
                                       i18n("<p>The following transfer has been started:</p><p style=\"font-size: small;\">%1</p>", transfer->source().toString()),
                                       "kget", i18n("Download started"));
            } else if (transfer->status() == Job::Aborted && transfer->error().type != Job::AutomaticRetry) {
                KNotification * notification = KNotification::event("error", i18n("Error"), i18n("<p>There has been an error in the following transfer:</p><p style=\"font-size: small;\">%1</p>"
                                            "<p>The error message is:</p><p style=\"font-size: small;\">%2</p>", transfer->source().toString(), transfer->error().text), 
                                             transfer->error().pixmap, KGet::m_mainWindow, KNotification::CloseOnTimeout);
                if (transfer->error().type == Job::ManualSolve) {
                    m_notifications.insert(notification, transfer);
                    notification->setActions(QStringList() << i18n("Resolve"));
                    connect(notification, &KNotification::action1Activated, this, &GenericObserver::slotResolveTransferError);
                    connect(notification, &KNotification::closed, this, &GenericObserver::slotNotificationClosed);
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
        qCDebug(KGET_DEBUG) << "All finished";
        KNotification *notification = nullptr;

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
            connect(notification, &KNotification::action1Activated, this, &GenericObserver::slotAbortAfterFinishAction);
            connect(m_finishAction, &QTimer::timeout, notification, &KNotification::close);

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
    auto * notification = static_cast<KNotification*>(QObject::sender());
    if (notification) {
        TransferHandler * handler = m_notifications[notification];
        qDebug() << "Resolve error for" << handler->source().toString() << "with id" << handler->error().id;
        handler->resolveError(handler->error().id);
        m_notifications.remove(notification);
    }
}

void GenericObserver::slotNotificationClosed()
{
    qDebug() << "Remove notification";
    auto * notification = static_cast<KNotification*>(QObject::sender());
    if (notification)
        m_notifications.remove(notification);
}

void GenericObserver::slotNetworkStatusChanged(bool online)
{
    KGet::setHasNetworkConnection(online);
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
    qDebug() << "Recalculate limits?" << recalculate;
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
    qCDebug(KGET_DEBUG);

    switch (Settings::afterFinishAction()) {
        case KGet::Quit:
            qCDebug(KGET_DEBUG) << "Quit Kget.";
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
            call = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.PowerManagement"),
                                                  QStringLiteral("/org/freedesktop/PowerManagement"),
                                                  QStringLiteral("org.freedesktop.PowerManagement"),
                                                  QStringLiteral("Hibernate"));
            QDBusConnection::sessionBus().asyncCall(call);
            break;
        }
        case KGet::Suspend: {
            QDBusMessage call;
            call = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.PowerManagement"),
                                                  QStringLiteral("/org/freedesktop/PowerManagement"),
                                                  QStringLiteral("org.freedesktop.PowerManagement"),
                                                  QStringLiteral("Suspend"));
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
    qCDebug(KGET_DEBUG);

    m_finishAction->stop();
}


