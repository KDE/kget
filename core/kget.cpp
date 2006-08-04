/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <QDirModel>
#include <QTextStream>
#include <QDomElement>
#include <QFileDialog>  // temporarily replace the bugged kfiledialog

#include <QAbstractItemView>

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

#include "mainwindow.h"
#include "core/kget.h"
#include "core/transfer.h"
#include "core/transfergroup.h"
#include "core/plugin/plugin.h"
#include "core/plugin/transferfactory.h"
#include "core/observer.h"
#include "settings.h"

/**
 * This is our KGet class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

void KGet::addObserver(ModelObserver * observer)
{
    kDebug() << "KGet::addObserver" << endl;

    m_observers.append(observer);

    //Update the new observer with the TransferGroups objects of the model
    QList<TransferGroup *>::const_iterator it = m_transferTreeModel->transferGroups().begin();
    QList<TransferGroup *>::const_iterator itEnd = m_transferTreeModel->transferGroups().end();

    for( ; it!=itEnd ; ++it )
    {
        postAddedTransferGroupEvent(*it, observer);
    }

    kDebug() << "KGet::addObserver   >>> EXITING" << endl;
}

void KGet::delObserver(ModelObserver * observer)
{
    m_observers.removeAll(observer);
}

void KGet::addGroup(const QString& groupName)
{
    kDebug() << "KGet::addGroup" << endl;

    TransferGroup * group = new TransferGroup(m_transferTreeModel, m_scheduler, groupName);

    m_transferTreeModel->addGroup(group);

    //post notifications
    postAddedTransferGroupEvent(group);
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

void KGet::addTransfer( KUrl srcURL, QString destDir,
                         const QString& groupName )
{
    kDebug() << " addTransfer:  " << srcURL.url() << endl;

    KUrl destURL;

    if ( srcURL.isEmpty() )
    {
        //No src location: we let the user insert it manually
        srcURL = urlInputDialog();
        if( srcURL.isEmpty() )
            return;
    }

    if ( !isValidSource( srcURL ) )
        return;

    if ( !isValidDestDirectory( destDir ) )
        destDir = destInputDialog();

    if( (destURL = getValidDestURL( destDir, srcURL )).isEmpty() )
        return;

    createTransfer(srcURL, destURL, groupName);
}

void KGet::addTransfer(const QDomElement& e, const QString& groupName)
{
    //We need to read these attributes now in order to know which transfer
    //plugin to use.
    KUrl srcURL = KUrl( e.attribute("Source") );
    KUrl destURL = KUrl( e.attribute("Dest") );

    kDebug() << "KGet::addTransfer  src= " << srcURL.url()
              << " dest= " << destURL.url() 
              << " group= "<< groupName << endl;

    if ( srcURL.isEmpty() || !isValidSource(srcURL) 
         || !isValidDestDirectory(destURL.directory()) )
        return;

    createTransfer(srcURL, destURL, groupName, &e);
}

void KGet::addTransfer(KUrl::List srcURLs, QString destDir,
                        const QString& groupName)
{
    KUrl::List urlsToDownload;

    KUrl::List::ConstIterator it = srcURLs.begin();
    KUrl::List::ConstIterator itEnd = srcURLs.end();

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
        addTransfer(srcURLs.first(), destDir, groupName);
        return;
    }

    KUrl destURL;

    // multiple files -> ask for directory, not for every single filename
    if ( !isValidDestDirectory(destDir) )
        destDir = destInputDialog();

    it = urlsToDownload.begin();
    itEnd = urlsToDownload.end();

    for ( ; it != itEnd; ++it )
    {
        destURL = getValidDestURL( destDir, *it );

        if(!isValidDestURL(destURL))
            continue;

        createTransfer(*it, destURL, groupName);
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
    delete( t );
}

void KGet::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
  Q_UNUSED(transfer);
  Q_UNUSED(groupName);
}

QList<TransferHandler *> KGet::selectedTransfers()
{
    QList<TransferHandler *> selectedTransfers;

    QList<TransferGroup *>::const_iterator it = m_transferTreeModel->transferGroups().begin();
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
    return selectedTransfers;
}

void KGet::addTransferTreeView(QAbstractItemView * view)
{
//     QDirModel* model = new QDirModel();

    view->setModel(m_transferTreeModel);
}

void KGet::load( QString filename )
{
    kDebug() << "KGet::load(" << filename << ")" << endl;

    if(filename.isEmpty())
        filename = KStandardDirs::locateLocal("appdata", "transfers.kgt");

    QString tmpFile;

    //Try to save the transferlist to a temporary location
    if(!KIO::NetAccess::download(KUrl(filename), tmpFile, 0))
        return;

    QFile file(tmpFile);
    QDomDocument doc;

    kDebug() << "KGet::load file" << filename << endl;

    if(doc.setContent(&file))
    {
        QDomElement root = doc.documentElement();

        QDomNodeList nodeList = root.elementsByTagName("TransferGroup");
        int nItems = nodeList.length();

        for( int i = 0 ; i < nItems ; i++ )
        {
            TransferGroup * foundGroup = m_transferTreeModel->findGroup( nodeList.item(i).toElement().attribute("Name") );

            kDebug() << "KGet::load  -> group = " << nodeList.item(i).toElement().attribute("Name") << endl;

            if( !foundGroup )
            {
                kDebug() << "KGet::load  -> group not found" << endl;

                TransferGroup * newGroup = new TransferGroup(m_transferTreeModel, m_scheduler);
                newGroup->load(nodeList.item(i).toElement());

                m_transferTreeModel->addGroup(newGroup);

                //Post notifications
                postAddedTransferGroupEvent(newGroup);
            }
            else
            {
                kDebug() << "KGet::load  -> group found" << endl;

                //A group with this name already exists.
                //Integrate the group's transfers with the ones read from file
                foundGroup->load(nodeList.item(i).toElement());
            }
        }
    }
    else
    {
        kWarning() << "Error reading the transfers file" << endl;
    }
}

void KGet::save( QString filename )
{
    if ( !filename.isEmpty()
        && QFile::exists( filename )
        && (KMessageBox::questionYesNo(0,
                i18n("The file %1 Already exists\nOverwrite?", filename),
                i18n("Overwrite existing file?"), KStdGuiItem::yes(),
                KStdGuiItem::no(), "QuestionFilenameExists" )
                == KMessageBox::No) )
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
        //kWarning()<<"Unable to open output file when saving"<< endl;
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

// ------ STATIC MEMBERS INITIALIZATION ------
QList<ModelObserver *> KGet::m_observers;
TransferTreeModel * KGet::m_transferTreeModel;
QList<TransferFactory *> KGet::m_transferFactories;
QList<KLibrary *> KGet::m_pluginKLibraries;
Scheduler * KGet::m_scheduler = new Scheduler();
MainWindow * KGet::m_mainWindow = 0;

// ------ PRIVATE FUNCTIONS ------
KGet::KGet()
{
    m_transferTreeModel = new TransferTreeModel(m_scheduler);

    //Load all the available plugins
    loadPlugins();

    //Create the default group with empty name
    addGroup(QString());
}

KGet::~KGet()
{
    unloadPlugins();
    delete(m_scheduler);
}

void KGet::createTransfer(KUrl src, KUrl dest, const QString& groupName, const QDomElement * e)
{
    kDebug() << "createTransfer: srcURL= " << src.url() << "  " 
                             << "destURL= " << dest.url() 
                             << "group= _" << groupName << "_" << endl;

    TransferGroup * group = m_transferTreeModel->findGroup(groupName);
    if (group==0)
    {
        kDebug() << "KGet::createTransfer  -> group not found" << endl;
        group = m_transferTreeModel->transferGroups().first();
    }
    Transfer * newTransfer;

    QList<TransferFactory *>::iterator it = m_transferFactories.begin();
    QList<TransferFactory *>::iterator itEnd = m_transferFactories.end();

    for( ; it!=itEnd ; ++it)
    {
        kDebug() << "Trying plugin   n.plugins=" << m_transferFactories.size() << endl;
        if((newTransfer = (*it)->createTransfer(src, dest, group, m_scheduler, e)))
        {
//             kDebug() << "KGet::createTransfer   ->   CREATING NEW TRANSFER ON GROUP: _" << group->name() << "_" << endl;
            m_transferTreeModel->addTransfer(newTransfer, group);
            return;
        }
    }
    kDebug() << "createTransfer: Warning! No plugin found to handle the given url" << endl;
}

void KGet::postAddedTransferGroupEvent(TransferGroup * group, ModelObserver * observer)
{
    kDebug() << "KGet::postAddedTransferGroupEvent" << endl;
    if(observer)
    {
        observer->addedTransferGroupEvent(group->handler());
        return;
    }

    QList<ModelObserver *>::iterator it = m_observers.begin();
    QList<ModelObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        kDebug() << "message posted" << endl;

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

    while (!ok) 
    {
        newtransfer = KInputDialog::getText(i18n("New Download"), i18n("Enter URL:"), newtransfer, &ok, 0);

        if (!ok)
        {
            //user pressed cancel
            return KUrl();
        }

        KUrl src = KUrl(newtransfer);
        if( isValidSource(src) )
            return src;
        else
            ok = false;
    }
    return KUrl();
}

QString KGet::destInputDialog()
{
    //TODO Somehow, using KFIleDialog::getExistingDirectory() makes kget crash
    //when we close the application. Why?
//     QString destDir = KFileDialog::getExistingDirectory( Settings::lastDirectory() );
    QString destDir = QFileDialog::getExistingDirectory(m_mainWindow,
                                                        i18n("Choose a directory"),
                                                        Settings::lastDirectory());

    Settings::setLastDirectory( destDir );
    return destDir;
}

bool KGet::isValidSource(KUrl source)
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
            if (KMessageBox::questionYesNo(0,
                i18n("URL already saved:\n%1\nDownload again?", source.prettyUrl()),
                i18n("Download URL Again?"), KStdGuiItem::yes(),
                KStdGuiItem::no(), "QuestionURLAlreadySaved" )
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
            //transfer is not finished. Give an error message.
            KMessageBox::error(0,
                               i18n("Already saving URL\n%1", source.prettyUrl()),
                               i18n("Error"));
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

bool KGet::isValidDestURL(KUrl destURL)
{
    if(KIO::NetAccess::exists(destURL, false, 0))
    {
        if (KMessageBox::warningYesNo(0,
            i18n("Destination file \n%1\nalready exists.\n"
                 "Do you want to overwrite it?", destURL.prettyUrl()))
            == KMessageBox::Yes)
        {
            safeDeleteFile( destURL );
            return true;
        }
    }
    return true;
   /*
    KIO::open_RenameDlg(i18n("File already exists"), 
    (*it).url(), destURL.url(),
    KIO::M_MULTI);
   */
}

KUrl KGet::getValidDestURL(const QString& destDir, KUrl srcURL)
{
    if ( !isValidDestDirectory(destDir) )
        return KUrl();

    // create a proper destination file from destDir
    KUrl destURL = KUrl( destDir );
    QString filename = srcURL.fileName();

    if ( filename.isEmpty() )
    {
        // simply use the full url as filename
        filename = QUrl::toPercentEncoding( srcURL.prettyUrl(), "/" );
        kDebug() << " Filename is empty. Setting to  " << filename << endl;
        kDebug() << "   srcURL = " << srcURL.url() << endl;
        kDebug() << "   prettyUrl = " << srcURL.prettyUrl() << endl;
    }
    else
    {
        kDebug() << " Filename is not empty" << endl;
        destURL.adjustPath( KUrl::AddTrailingSlash );
        destURL.setFileName( filename );
        if (!isValidDestURL(destURL))
        {
            kDebug() << "   destURL " << destURL.path() << " is not valid" << endl;
            return KUrl();
        }
    }
    return destURL;
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
        kDebug() << " TransferFactory plugin found:" << endl <<
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
            kDebug() << "TransferFactory plugin (" << (*it)->library() 
                      << ") found and added to the list of available plugins" << endl;
        }
        else
            kDebug() << "Error loading TransferFactory plugin (" 
                      << (*it)->library() << ")" << endl;
    }

    QList<KGetPlugin *>::iterator it2 = pluginList.begin();
    QList<KGetPlugin *>::iterator it2End = pluginList.end();

    for( ; it2!=it2End ; ++it2 )
        m_transferFactories.append( static_cast<TransferFactory *>(*it2) );

    kDebug() << "Number of factories = " << m_transferFactories.size() << endl;
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

    KGetPlugin* (*create_plugin)() = ( KGetPlugin* (*)() ) lib->symbol( "create_plugin" );

    if ( !create_plugin ) 
    {
        kDebug() << "create_plugin == NULL" << endl;
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
