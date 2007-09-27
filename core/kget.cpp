/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "core/kget.h"

#include "mainwindow.h"
#include "core/transfer.h"
#include "core/transfergroup.h"
#include "core/transfergrouphandler.h"
#include "core/transfertreemodel.h"
#include "core/transfertreeselectionmodel.h"
#include "core/plugin/plugin.h"
#include "core/plugin/transferfactory.h"
#include "core/observer.h"
#include "settings.h"

#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kservicetypetrader.h>
#include <klibloader.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kio/renamedialog.h>

#include <QDirModel>
#include <QTextStream>
#include <QDomElement>
#include <QApplication>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QAbstractItemView>

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
    }

    static KGet m;
    return m;
}

void KGet::addObserver(ModelObserver * observer)
{
    kDebug(5001) << "KGet::addObserver";

    m_observers.append(observer);

    //Update the new observer with the TransferGroups objects of the model
    QList<TransferGroup *>::const_iterator it = m_transferTreeModel->transferGroups().begin();
    QList<TransferGroup *>::const_iterator itEnd = m_transferTreeModel->transferGroups().end();

    for( ; it!=itEnd ; ++it )
    {
        postAddedTransferGroupEvent(*it, observer);
    }

    kDebug(5001) << "KGet::addObserver   >>> EXITING";
}

void KGet::delObserver(ModelObserver * observer)
{
    m_observers.removeAll(observer);
}

bool KGet::addGroup(const QString& groupName)
{
    kDebug(5001) << "KGet::addGroup";

    // Check if a group with that name already exists
    if(m_transferTreeModel->findGroup(groupName))
        return false;

    TransferGroup * group = new TransferGroup(m_transferTreeModel, m_scheduler, groupName);
    m_transferTreeModel->addGroup(group);

    //post notifications
    postAddedTransferGroupEvent(group);

    return true;
}

void KGet::delGroup(const QString& groupName)
{
    TransferGroup * group = m_transferTreeModel->findGroup(groupName);

    if(group)
    {
        m_transferTreeModel->delGroup(group);
        postRemovedTransferGroupEvent(group);
        delete(group);
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

void KGet::addTransfer(KUrl srcUrl, QString destDir, // krazy:exclude=passbyvalue
                       const QString& groupName, bool start)
{
    kDebug(5001) << " addTransfer:  " << srcUrl.url();

    KUrl destUrl;

    if ( srcUrl.isEmpty() )
    {
        //No src location: we let the user insert it manually
        srcUrl = urlInputDialog();
        if( srcUrl.isEmpty() )
            return;
    }

    if ( !isValidSource( srcUrl ) )
        return;

    if (Settings::useDefaultDirectory())
#ifdef Q_OS_WIN //krazy:exclude=cpp
        destDir = Settings::defaultDirectory().remove("file:///");
#else
        destDir = Settings::defaultDirectory().remove("file://");
#endif

    QString checkExceptions = getSaveDirectoryFromExceptions(srcUrl);
    if (Settings::enableExceptions() && !checkExceptions.isEmpty())
        destDir = checkExceptions;

    if (!isValidDestDirectory(destDir))
        destDir = destInputDialog();

    if( (destUrl = getValidDestUrl( destDir, srcUrl )).isEmpty() )
        return;

    if(m_transferTreeModel->findTransferByDestination(destUrl) != 0 || (destUrl.isLocalFile() && QFile::exists(destUrl.path()))) {
        QString newDestination;
        KIO::RenameDialog dlg( m_mainWindow, i18n("Rename transfer"), srcUrl,
                               destUrl, KIO::M_MULTI);
        if (dlg.exec() == KIO::R_RENAME)
            destUrl = dlg.newDestUrl();
    }
    createTransfer(srcUrl, destUrl, groupName, start);
}

void KGet::addTransfer(const QDomElement& e, const QString& groupName)
{
    //We need to read these attributes now in order to know which transfer
    //plugin to use.
    KUrl srcUrl = KUrl( e.attribute("Source") );
    KUrl destUrl = KUrl( e.attribute("Dest") );

    kDebug(5001) << "KGet::addTransfer  src= " << srcUrl.url()
              << " dest= " << destUrl.url() 
              << " group= "<< groupName << endl;

    if ( srcUrl.isEmpty() || !isValidSource(srcUrl) 
         || !isValidDestDirectory(destUrl.directory()) )
        return;

    createTransfer(srcUrl, destUrl, groupName, false, &e);
}

void KGet::addTransfer(KUrl::List srcUrls, QString destDir, // krazy:exclude=passbyvalue
                       const QString& groupName, bool start)
{
    KUrl::List urlsToDownload;

    KUrl::List::ConstIterator it = srcUrls.begin();
    KUrl::List::ConstIterator itEnd = srcUrls.end();

    for(; it!=itEnd ; ++it)
    {
        if ( isValidSource( *it ) )
            urlsToDownload.append( *it );
    }

    if ( urlsToDownload.count() == 0 )
        return;

    if ( urlsToDownload.count() == 1 )
    {
        // just one file -> ask for filename
        addTransfer(srcUrls.first(), destDir, groupName, start);
        return;
    }

    KUrl destUrl;

    // multiple files -> ask for directory, not for every single filename
    if (!isValidDestDirectory(destDir) && !Settings::useDefaultDirectory())
        destDir = destInputDialog();

    it = urlsToDownload.begin();
    itEnd = urlsToDownload.end();

    for ( ; it != itEnd; ++it )
    {
        if (Settings::useDefaultDirectory())
#ifdef Q_OS_WIN //krazy:exclude=cpp
            destDir = Settings::defaultDirectory().remove("file:///");
#else
            destDir = Settings::defaultDirectory().remove("file://");
#endif
        destDir = destDir.remove("file://");

        QString checkExceptions = getSaveDirectoryFromExceptions(*it);
        if (Settings::enableExceptions() && !checkExceptions.isEmpty())
            destDir = checkExceptions;

        destUrl = getValidDestUrl(destDir, *it);

        if(!isValidDestUrl(destUrl))
            continue;

        createTransfer(*it, destUrl, groupName, start);
    }
}


void KGet::delTransfer(TransferHandler * transfer)
{
    Transfer * t = transfer->m_transfer;

    m_transferTreeModel->delTransfer(t);

    //Here I delete the Transfer. The other possibility is to move it to a list
    //and to delete all these transfers when kget gets closed. Obviously, after
    //the notification to the views that the transfer has been removed, all the
    //pointers to it are invalid.
    transfer->postDeleteEvent();
// TODO: why does it crash if a download is going to be deleted which is not the last in the list?
// there are always no problems with the last download.
//     delete( t );
}

void KGet::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
  Q_UNUSED(transfer);
  Q_UNUSED(groupName);
}

QList<TransferHandler *> KGet::selectedTransfers()
{
//     kDebug(5001) << "KGet::selectedTransfers";

    QList<TransferHandler *> selectedTransfers;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();

    foreach(QModelIndex currentIndex, selectedIndexes)
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

QList<TransferGroupHandler *> KGet::selectedTransferGroups()
{
    QList<TransferGroupHandler *> selectedTransferGroups;

    QModelIndexList selectedIndexes = m_selectionModel->selectedRows();

    foreach(QModelIndex currentIndex, selectedIndexes)
    {
        if(m_transferTreeModel->isTransferGroup(currentIndex))
            selectedTransferGroups.append(static_cast<TransferGroupHandler *> (currentIndex.internalPointer()));
    }

    return selectedTransferGroups;
}

TransferTreeSelectionModel * KGet::selectionModel()
{
    return m_selectionModel;
}

void KGet::addTransferView(QAbstractItemView * view)
{
    view->setModel(m_transferTreeModel);
}

void KGet::load( QString filename ) // krazy:exclude=passbyvalue
{
    kDebug(5001) << "KGet::load(" << filename << ")";

    if(filename.isEmpty())
        filename = KStandardDirs::locateLocal("appdata", "transfers.kgt");

    QString tmpFile;

    //Try to save the transferlist to a temporary location
    if(!KIO::NetAccess::download(KUrl(filename), tmpFile, 0))
        return;

    QFile file(tmpFile);
    QDomDocument doc;

    kDebug(5001) << "KGet::load file" << filename;

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

                //Post notifications
                postAddedTransferGroupEvent(newGroup);
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
}

void KGet::save( QString filename ) // krazy:exclude=passbyvalue
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
    }
    QFile file(filename);
    if ( !file.open( QIODevice::WriteOnly ) )
    {
        //kWarning(5001)<<"Unable to open output file when saving";
        KMessageBox::error(0,
                           i18n("Unable to save to: %1", filename),
                           i18n("Error"));
        return;
    }

    QTextStream stream( &file );
    doc.save( stream, 0 );
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
	m_scheduler->start();
    else
	m_scheduler->stop();
}

bool KGet::schedulerRunning()
{
    return (m_scheduler->countRunningJobs() > 0);
}

void KGet::setPluginsSettingsWidget(KTabWidget * widget)
{
    QList<TransferFactory *>::iterator it = m_transferFactories.begin();
    QList<TransferFactory *>::iterator itEnd = m_transferFactories.end();

    QWidget * settingsWidget;
    for( ; it!=itEnd ; ++it)
    {
        settingsWidget = (*it)->createSettingsWidget();
        if(settingsWidget)
            widget->addTab( settingsWidget, (*it)->displayName() );
    }
}

// ------ STATIC MEMBERS INITIALIZATION ------
QList<ModelObserver *> KGet::m_observers;
TransferTreeModel * KGet::m_transferTreeModel;
TransferTreeSelectionModel * KGet::m_selectionModel;
QList<TransferFactory *> KGet::m_transferFactories;
QList<KLibrary *> KGet::m_pluginKLibraries;
Scheduler * KGet::m_scheduler = new Scheduler();
MainWindow * KGet::m_mainWindow = 0;

// ------ PRIVATE FUNCTIONS ------
KGet::KGet()
{
    m_transferTreeModel = new TransferTreeModel(m_scheduler);
    m_selectionModel = new TransferTreeSelectionModel(m_transferTreeModel);

    //Load all the available plugins
    loadPlugins();

    //Create the default group
    addGroup(i18n("My Downloads"));
}

KGet::~KGet()
{
    unloadPlugins();
    delete(m_scheduler);
}

void KGet::createTransfer(const KUrl &src, const KUrl &dest, const QString& groupName, 
                          bool start, const QDomElement * e)
{
    kDebug(5001) << "createTransfer: srcUrl= " << src.url() << "  " 
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
            m_transferTreeModel->addTransfer(newTransfer, group);

            if(start)
                newTransfer->handler()->start();

            return;
        }
    }
    kDebug(5001) << "createTransfer: Warning! No plugin found to handle the given url";
}

void KGet::postAddedTransferGroupEvent(TransferGroup * group, ModelObserver * observer)
{
    kDebug(5001) << "KGet::postAddedTransferGroupEvent";
    if(observer)
    {
        observer->addedTransferGroupEvent(group->handler());
        return;
    }

    QList<ModelObserver *>::iterator it = m_observers.begin();
    QList<ModelObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        kDebug(5001) << "message posted";

        (*it)->addedTransferGroupEvent(group->handler());
    }
}

void KGet::postRemovedTransferGroupEvent(TransferGroup * group, ModelObserver * observer)
{
    if(observer)
    {
        observer->removedTransferGroupEvent(group->handler());
        return;
    }

    QList<ModelObserver *>::iterator it = m_observers.begin();
    QList<ModelObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->removedTransferGroupEvent(group->handler());
    }
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

QString KGet::destInputDialog()
{
    QString destDir = KFileDialog::getExistingDirectory(Settings::lastDirectory());

    Settings::setLastDirectory( destDir );
    return destDir;
}

QString KGet::getSaveDirectoryFromExceptions(const KUrl &filename)
{
    QString destDir;

    QStringList list = Settings::extensionsFolderList();
    QStringList::Iterator it = list.begin();
    QStringList::Iterator end = list.end();
    while (it != end) {
        // odd list items are regular expressions for extensions
        QString ext = *it;
        ++it;
        QString path = *it;
        ++it;

        if (!ext.startsWith('*'))
            ext = '*' + ext;

        QRegExp rexp(ext);
        rexp.setPatternSyntax(QRegExp::Wildcard);

        if (rexp.exactMatch(filename.url())) {
            destDir = path;
            break;
        }
    }

#ifdef Q_OS_WIN //krazy:exclude=cpp
    destDir = destDir.remove("file:///");
#endif
    return destDir.remove("file://");
}

bool KGet::isValidSource(const KUrl &source)
{
    if (!source.isValid())
    {
        KMessageBox::error(0,
                           i18n("Malformed URL:\n%1", source.prettyUrl()),
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
                //TODO reimplement this
                //transfer->slotRemove();
                //checkQueue();
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
    return (!destDir.isEmpty() && QFileInfo( destDir ).isDir());
}

bool KGet::isValidDestUrl(const KUrl &destUrl)
{
    if(KIO::NetAccess::exists(destUrl, KIO::NetAccess::DestinationSide, 0))
    {
        if (KMessageBox::warningYesNoCancel(0,
            i18n("Destination file \n%1\nalready exists.\n"
                 "Do you want to overwrite it?", destUrl.prettyUrl()))
            == KMessageBox::Yes)
        {
            safeDeleteFile( destUrl );
            return true;
        }
        else
            return false;
    }
    return true;
   /*
    KIO::open_RenameDlg(i18n("File already exists"), 
    (*it).url(), destUrl.url(),
    KIO::M_MULTI);
   */
}

KUrl KGet::getValidDestUrl(const QString& destDir, const KUrl &srcUrl)
{
    if ( !isValidDestDirectory(destDir) )
        return KUrl();

    // create a proper destination file from destDir
    KUrl destUrl = KUrl( destDir );
    QString filename = srcUrl.fileName();

    if ( filename.isEmpty() )
    {
        // simply use the full url as filename
        filename = KUrl::toPercentEncoding( srcUrl.prettyUrl(), "/" );
        kDebug(5001) << " Filename is empty. Setting to  " << filename;
        kDebug(5001) << "   srcUrl = " << srcUrl.url();
        kDebug(5001) << "   prettyUrl = " << srcUrl.prettyUrl();
    }
    else
    {
        kDebug(5001) << " Filename is not empty";
        destUrl.adjustPath( KUrl::AddTrailingSlash );
        destUrl.setFileName( filename );
        if (!isValidDestUrl(destUrl))
        {
            kDebug(5001) << "   destUrl " << destUrl.path() << " is not valid";
            return KUrl();
        }
    }
    return destUrl;
}

void KGet::loadPlugins()
{
    // Add versioning constraint
    QString
    str  = "[X-KDE-KGet-framework-version] == ";
    str += QString::number( FrameworkVersion );
    str += " and ";
    str += "[X-KDE-KGet-rank] > 0";
    str += " and ";
    str += "[X-KDE-KGet-plugintype] == ";

    KService::List offers;

    //TransferFactory plugins
    offers = KServiceTypeTrader::self()->query( "KGet/Plugin", str + "'TransferFactory'" );

    //Here we use a QMap only to easily sort the plugins by rank
    QMap<int, KService::Ptr> services;
    QMap<int, KService::Ptr>::iterator it;

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

    for( it = services.begin(); it != services.end(); ++it )
    {
        KGetPlugin * plugin;
        if( (plugin = createPluginFromService(*it)) != 0 )
        {
            pluginList.prepend(plugin);
            kDebug(5001) << "TransferFactory plugin (" << (*it)->library() 
                      << ") found and added to the list of available plugins" << endl;
        }
        else
            kDebug(5001) << "Error loading TransferFactory plugin (" 
                      << (*it)->library() << ")" << endl;
    }

    QList<KGetPlugin *>::iterator it2 = pluginList.begin();
    QList<KGetPlugin *>::iterator it2End = pluginList.end();

    for( ; it2!=it2End ; ++it2 )
        m_transferFactories.append( static_cast<TransferFactory *>(*it2) );

    kDebug(5001) << "Number of factories = " << m_transferFactories.size();
}

void KGet::unloadPlugins()
{
    QList<KLibrary *>::iterator it = m_pluginKLibraries.begin();
    QList<KLibrary *>::iterator itEnd = m_pluginKLibraries.end();

    for(;it!=itEnd;++it)
    {
        (*it)->unload();
    }
    m_transferFactories.clear();
}

KGetPlugin * KGet::createPluginFromService( const KService::Ptr service )
{
    //get the library loader instance
    KLibLoader *loader = KLibLoader::self();

    //try to load the specified library
    //Warning! This line seems to erase my m_transferFactories list!!
    KLibrary *lib = loader->library( QFile::encodeName( service->library() ) );

    if ( !lib ) 
    {
        KMessageBox::error( 0, i18n( "<p>KLibLoader could not load the plugin:<br/><i>%1</i></p>"
                "<p>Error message:<br/><i>%2</i></p>", service->library(), loader->lastErrorMessage() ) );
        return 0;
    }

    KGetPlugin* (*create_plugin)() = ( KGetPlugin* (*)() ) lib->resolveFunction( "create_plugin" );

    if ( !create_plugin ) 
    {
        kDebug(5001) << "create_plugin == NULL";
        return 0;
    }

    m_pluginKLibraries.append(lib);

    return create_plugin();
}

bool KGet::safeDeleteFile( const KUrl& url )
{
    if ( url.isLocalFile() )
    {
        QFileInfo info( url.path() );
        if ( info.isDir() )
        {
            KMessageBox::information(0L,i18n("Not deleting\n%1\nas it is a "
                                     "directory.", url.prettyUrl()),
                                     i18n("Not Deleted"));
            return false;
        }
        KIO::NetAccess::del( url, 0L );
        return true;
    }

    else
        KMessageBox::information( 0L,
                                  i18n("Not deleting\n%1\nas it is not a local"
                                          " file.", url.prettyUrl()),
                                  i18n("Not Deleted") );
    return false;
}
