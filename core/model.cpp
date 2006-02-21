/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <klibloader.h>
#include <kiconloader.h>

#include "kget.h"
#include "core/model.h"
#include "core/transfer.h"
#include "core/transfergroup.h"
#include "core/plugin/plugin.h"
#include "core/plugin/transferfactory.h"
#include "core/observer.h"
#include "conf/settings.h"

/**
 * This is our Model class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

void Model::addObserver(ModelObserver * observer)
{
    kDebug() << "Model::addObserver" << endl;

    m_observers.append(observer);

    //Update the new observer with the TransferGroups objects of the model
    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    for( ; it!=itEnd ; ++it )
        postAddedTransferGroupEvent(*it, observer);
}

void Model::delObserver(ModelObserver * observer)
{
    m_observers.removeAll(observer);
}

void Model::addGroup(const QString& groupName)
{
    TransferGroup * group = new TransferGroup(m_scheduler, groupName);
    m_transferGroups.append(group);

    kDebug() << "Model::addGroup" << endl;
    postAddedTransferGroupEvent(group);
}

void Model::delGroup(const QString& groupName)
{
    TransferGroup * group = findGroup(groupName);

    if(group)
    {
        m_transferGroups.removeAll(group);
        postRemovedTransferGroupEvent(group);
        delete(group);
    }
}

void Model::addTransfer( KUrl srcURL, QString destDir,
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

void Model::addTransfer(const QDomElement& e, const QString& groupName)
{
    //We need to read these attributes now in order to know which transfer
    //plugin to use.
    KUrl srcURL = KUrl::fromPathOrURL( e.attribute("Source") );
    KUrl destURL = KUrl::fromPathOrURL( e.attribute("Dest") );

    kDebug() << "Model::addTransfer  src= " << srcURL.url()
              << " dest= " << destURL.url() << endl;

    if ( srcURL.isEmpty() || !isValidSource(srcURL) 
         || !isValidDestDirectory(destURL.directory()) )
        return;

    createTransfer(srcURL, destURL, groupName, &e);
}

void Model::addTransfer(KUrl::List srcURLs, QString destDir,
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


void Model::delTransfer(TransferHandler * transfer)
{
    Transfer * t = transfer->m_transfer;
    t->group()->remove( t );

    //Here I delete the Transfer. The other possibility is to move it to a list
    //and to delete all these transfers when kget gets closed. Obviously, after
    //the notification to the views that the transfer has been removed, all the
    //pointers to it are invalid.
    transfer->postDeleteEvent();
    delete( t );
}

void Model::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
  Q_UNUSED(transfer);
  Q_UNUSED(groupName);
}

QList<TransferHandler *> Model::selectedTransfers()
{
    QList<TransferHandler *> selectedTransfers;

    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

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

void Model::load( QString filename )
{
    if(filename.isEmpty())
        filename = locateLocal("appdata", "transfers.kgt");

    QString tmpFile;

    //Try to save the transferlist to a temporary location
    if(!KIO::NetAccess::download(KUrl::fromPathOrURL(filename), tmpFile, 0))
        return;

    QFile file(tmpFile);
    QDomDocument doc;

    kDebug() << "Model::load file" << filename << endl;

    if(doc.setContent(&file))
    {
        QDomElement root = doc.documentElement();

        QDomNodeList nodeList = root.elementsByTagName("TransferGroup");
        int nItems = nodeList.length();

        for( int i = 0 ; i < nItems ; i++ )
        {
            TransferGroup * foundGroup = findGroup( nodeList.item(i).toElement().attribute("Name") );

            if( !foundGroup )
            {
                TransferGroup * newGroup = new TransferGroup(m_scheduler, nodeList.item(i).toElement());
                m_transferGroups.append(newGroup);
                postAddedTransferGroupEvent(newGroup);
            }
            else
            {
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

void Model::save( QString filename )
{
    if ( !filename.isEmpty()
        && QFile::exists( filename )
        && (KMessageBox::questionYesNo(0,
                i18n("The file %1 Already exists\nOverwrite?").arg(filename),
                i18n("Overwrite existing file?"), KStdGuiItem::yes(),
                KStdGuiItem::no(), "QuestionFilenameExists" )
                == KMessageBox::No) )
        return;

    if(filename.isEmpty())
        filename = locateLocal("appdata", "transfers.kgt");

    QDomDocument doc(QString("KGetTransfers"));
    QDomElement root = doc.createElement("Transfers");
    doc.appendChild(root);

    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

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
                           i18n("Unable to save to: %1").arg(filename),
                           i18n("Error"));
        return;
    }

    QTextStream stream( &file );
    doc.save( stream, 0 );
    file.close();
}

TransferFactory * Model::factory(TransferHandler * transfer)
{
    return transfer->m_transfer->factory();
}

KActionCollection * Model::actionCollection()
{
    return m_kget->actionCollection();
}

void Model::setSchedulerRunning(bool running)
{
    if(running)
	m_scheduler->start();
    else
	m_scheduler->stop();
}

// ------ STATIC MEMBERS INITIALIZATION ------
QList<TransferGroup *> Model::m_transferGroups; // = QValueList<TransferGroup *>();
QList<ModelObserver *> Model::m_observers; // = QValueList<ModelObserver *>();
QList<TransferFactory *> Model::m_transferFactories; // = QValueList<TransferFactory *>();
QList<KLibrary *> Model::m_pluginKLibraries;// = QValueList<KLibrary *>();
Scheduler * Model::m_scheduler = new Scheduler();
KGet * Model::m_kget = 0;

// ------ PRIVATE FUNCTIONS ------
Model::Model()
{
    //Load all the available plugins
    loadPlugins();

    //Create the default group with empty name
    addGroup("");
}

Model::~Model()
{
    unloadPlugins();
    delete(m_scheduler);
}

void Model::createTransfer(KUrl src, KUrl dest, const QString& groupName, const QDomElement * e)
{
    kDebug() << "createTransfer: srcURL=" << src.url() << "  " << "destURL=" << dest.url() << endl;

    TransferGroup * group = findGroup(groupName);
    if (group==0)
        group = m_transferGroups.first();
    Transfer * newTransfer;

    QList<TransferFactory *>::iterator it = m_transferFactories.begin();
    QList<TransferFactory *>::iterator itEnd = m_transferFactories.end();

    for( ; it!=itEnd ; ++it)
    {
        kDebug() << "Trying plugin   n.plugins=" << m_transferFactories.size() << endl;
        if((newTransfer = (*it)->createTransfer(src, dest, group, m_scheduler, e)))
        {
            group->append(newTransfer);
            return;
        }
    }
    kDebug() << "createTransfer: Warning! No plugin found to handle the given url" << endl;
}

void Model::postAddedTransferGroupEvent(TransferGroup * group, ModelObserver * observer)
{
    kDebug() << "Model::postAddedTransferGroupEvent" << endl;
    if(observer)
    {
        observer->addedTransferGroupEvent(group->handler());
        return;
    }

    QList<ModelObserver *>::iterator it = m_observers.begin();
    QList<ModelObserver *>::iterator itEnd = m_observers.end();

    for(; it!=itEnd; ++it)
    {
        (*it)->addedTransferGroupEvent(group->handler());
    }
}

void Model::postRemovedTransferGroupEvent(TransferGroup * group, ModelObserver * observer)
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

KUrl Model::urlInputDialog()
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

        KUrl src = KUrl::fromPathOrURL(newtransfer);
        if( isValidSource(src) )
            return src;
        else
            ok = false;
    }
    return KUrl();
}

QString Model::destInputDialog()
{
    //TODO Somehow, using KFIleDialog::getExistingDirectory() makes kget crash
    //when we close the application. Why?
    QString destDir = KFileDialog::getExistingDirectory( Settings::lastDirectory() );
    Settings::setLastDirectory( destDir );
    return destDir;
}

bool Model::isValidSource(KUrl source)
{
    if (!source.isValid())
    {
        KMessageBox::error(0,
                           i18n("Malformed URL:\n%1").arg(source.prettyURL()),
                           i18n("Error"));
        return false;
    }
    // Check if a transfer with the same url already exists
    Transfer * transfer = findTransfer( source );
    if ( transfer )
    {
        if ( transfer->status() == Job::Finished )
        {
            // transfer is finished, ask if we want to download again
            if (KMessageBox::questionYesNo(0,
                i18n("URL already saved:\n%1\nDownload again?").arg(source.prettyURL()),
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
                               i18n("Already saving URL\n%1").arg(source.prettyURL()),
                               i18n("Error"));
            return false;
        }
        return false;
    }
    return true;
}

bool Model::isValidDestDirectory(const QString & destDir)
{
    return (!destDir.isEmpty() && QFileInfo( destDir ).isDir());
}

bool Model::isValidDestURL(KUrl destURL)
{
    if(KIO::NetAccess::exists(destURL, false, 0))
    {
        if (KMessageBox::warningYesNo(0,
            i18n("Destination file \n%1\nalready exists.\n"
                 "Do you want to overwrite it?").arg( destURL.prettyURL()) )
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

KUrl Model::getValidDestURL(const QString& destDir, KUrl srcURL)
{
    if ( !isValidDestDirectory(destDir) )
        return KUrl();

    // create a proper destination file from destDir
    KUrl destURL = KUrl::fromPathOrURL( destDir );
    QString filename = srcURL.fileName();

    if ( filename.isEmpty() )
    {
        // simply use the full url as filename
        filename = QUrl::toPercentEncoding( srcURL.prettyURL(), "/" );
        kDebug() << " Filename is empty. Setting to  " << filename << endl;
        kDebug() << "   srcURL = " << srcURL.url() << endl;
        kDebug() << "   prettyURL = " << srcURL.prettyURL() << endl;
    }
    else
    {
        kDebug() << " Filename is not empty" << endl;
        destURL.adjustPath( +1 );
        destURL.setFileName( filename );
        if (!isValidDestURL(destURL))
        {
            kDebug() << "   destURL " << destURL.path() << " is not valid" << endl;
            return KUrl();
        }
    }
    return destURL;
}

TransferGroup * Model::findGroup(const QString & groupName)
{
    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    for(; it!=itEnd ; ++it)
    {
        if( (*it)->name() == groupName )
            return *it;
    }
    return 0;
}

Transfer * Model::findTransfer(KUrl src)
{
    QList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    Transfer * t;

    for(; it!=itEnd ; ++it)
    {
        if( ( t = (*it)->findTransfer(src) ) )
            return t;
    }
    return 0;
}

void Model::loadPlugins()
{
    // Add versioning constraint
    QString
    str  = "[X-KDE-KGet-framework-version] == ";
    str += QString::number( FrameworkVersion );
    str += " and ";
    str += "[X-KDE-KGet-rank] > 0";
    str += " and ";
    str += "[X-KDE-KGet-plugintype] == ";

    KTrader::OfferList offers;

    //TransferFactory plugins
    offers = KTrader::self()->query( "KGet/Plugin", str + "'TransferFactory'" );

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

void Model::unloadPlugins()
{
    QList<KLibrary *>::iterator it = m_pluginKLibraries.begin();
    QList<KLibrary *>::iterator itEnd = m_pluginKLibraries.end();

    for(;it!=itEnd;++it)
    {
        (*it)->unload();
    }
    m_transferFactories.clear();
}

KGetPlugin * Model::createPluginFromService( const KService::Ptr service )
{
    //get the library loader instance
    KLibLoader *loader = KLibLoader::self();

    //try to load the specified library
    //Warning! This line seems to erase my m_transferFactories list!!
    KLibrary *lib = loader->library( QFile::encodeName( service->library() ) );

    if ( !lib ) 
    {
        KMessageBox::error( 0, i18n( "<p>KLibLoader could not load the plugin:<br/><i>%1</i></p>"
                "<p>Error message:<br/><i>%2</i></p>" )
                        .arg( service->library() )
                        .arg( loader->lastErrorMessage() ) );
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

bool Model::safeDeleteFile( const KUrl& url )
{
    if ( url.isLocalFile() )
    {
        QFileInfo info( url.path() );
        if ( info.isDir() )
        {
            KMessageBox::information(0L,i18n("Not deleting\n%1\nas it is a "
                                     "directory.").arg( url.prettyURL() ),
                                     i18n("Not Deleted"));
            return false;
        }
        KIO::NetAccess::del( url, 0L );
        return true;
    }

    else
        KMessageBox::information( 0L,
                                  i18n("Not deleting\n%1\nas it is not a local"
                                          " file.").arg( url.prettyURL() ),
                                  i18n("Not Deleted") );
    return false;
}
