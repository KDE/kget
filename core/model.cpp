/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qfileinfo.h>
#include <qvaluevector.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <ktrader.h>
#include <klibloader.h>

#include "core/model.h"
#include "core/transfergroup.h"
#include "core/plugin/plugin.h"
#include "core/plugin/transferfactory.h"
#include "conf/settings.h"
#include "transfers/kio/transferKio.h" //temporary!!

/**
 * This is our Model class. This is where the user's transfers and searches are
 * stored and organized.
 * Use this class from the views to add or remove transfers or searches 
 * In order to organize the transfers inside categories we have a TransferGroup
 * class. By definition, a transfer must always belong to a TransferGroup. If we 
 * don't want it to be displayed by the gui inside a specific group, we will put 
 * it in the group named "Not grouped" (better name?).
 **/

void Model::addObserver(ModelObserver *)
{
    
}

void Model::addGroup(const QString& groupName)
{
    
}

void Model::delGroup(const QString& groupName)
{
    
}

void Model::addTransfer( KURL srcURL, QString destDir,
                         const QString& groupName )
{
    kdDebug() << " addTransfer:  " << srcURL.url() << endl;

    KURL destURL;

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

void Model::addTransfer(KURL::List srcURLs, QString destDir,
                        const QString& groupName)
{
    KURL::List urlsToDownload;

    KURL::List::ConstIterator it = srcURLs.begin();
    KURL::List::ConstIterator itEnd = srcURLs.end();

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

    KURL destURL;

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
    
}

void Model::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
    
}

// ------ STATIC MEMBERS INITIALIZATION ------
QValueList<TransferGroup *> Model::m_transferGroups = QValueList<TransferGroup *>();
QValueList<ModelObserver *> Model::m_observers = QValueList<ModelObserver *>();
QValueList<TransferFactory *> Model::m_transferFactories = QValueList<TransferFactory *>();
QValueList<KLibrary *> Model::m_pluginKLibraries = QValueList<KLibrary *>();
Scheduler Model::m_scheduler = Scheduler();

// ------ PRIVATE FUNCTIONS ------
Model::Model()
{
    //Create the default group with empty name
    m_transferGroups.append(new TransferGroup(""));

    //Load all the available plugins
    loadPlugins();
}

Model::~Model()
{
    unloadPlugins();
}

void Model::createTransfer(KURL src, KURL dest,
                                 const QString& groupName)
{
    kdDebug() << "createTransfer: srcURL=" << src.url() << "  " << "destURL=" << dest.url() << endl;

    TransferGroup * group = findGroup(groupName);
    if (group==0)
        group = m_transferGroups.first();

    //TODO write properly this method
    //group->append(new TransferKio(group, &m_scheduler, src, dest));
}

KURL Model::urlInputDialog()
{
    QString newtransfer;
    bool ok = false;

    while (!ok) 
    {
        newtransfer = KInputDialog::getText(i18n("New Download"), i18n("Enter URL:"), newtransfer, &ok, 0);

        if (!ok)
        {
            //user pressed cancel
            return KURL();
        }

        KURL src = KURL::fromPathOrURL(newtransfer);
        if( isValidSource(src) )
            return src;
        else
            ok = false;
    }
}

QString Model::destInputDialog()
{
    //TODO Somehow, using KFIleDialog::getExistingDirectory() makes kget crash
    //when we close the application. Why?
    QString destDir = KFileDialog::getExistingDirectory( Settings::lastDirectory() );
    Settings::setLastDirectory( destDir );
    return destDir;
}

bool Model::isValidSource(KURL source)
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
        if ( transfer->jobStatus() == Job::Finished )
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

bool Model::isValidDestURL(KURL destURL)
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
        return false;
    }
    return true;
   /*
    KIO::open_RenameDlg(i18n("File already exists"), 
    (*it).url(), destURL.url(),
    KIO::M_MULTI);
   */
}

KURL Model::getValidDestURL(const QString& destDir, KURL srcURL)
{
    if ( !isValidDestDirectory(destDir) )
        return KURL();

    // create a proper destination file from destDir
    KURL destURL = KURL::fromPathOrURL( destDir );
    QString filename = srcURL.filename();

    if ( filename.isEmpty() )
    {
        // I really don't understand why we should care about the source URLs
        // with empty filename.
        // ------ In the previous scheduler.cpp class it was: -------
        // in case the fileName is empty, we simply ask for a filename in
        // addTransferEx. Do NOT attempt to use an empty filename, that
        // would be a directory (and we don't want to overwrite that!)
        // simply use the full url as filename

        // ATM I use the same code I used in the next addTransfer function
        filename = KURL::encode_string_no_slash( srcURL.prettyURL() );
        kdDebug() << " Filename is empty. Setting to  " << filename << endl;
        kdDebug() << "   srcURL = " << srcURL.url() << endl;
        kdDebug() << "   prettyURL = " << srcURL.prettyURL() << endl;
    }
    else
    {
        kdDebug() << " Filename is not empty" << endl;
        destURL.adjustPath( +1 );
        destURL.setFileName( filename );
        if (!isValidDestURL(destURL))
        {
            kdDebug() << "   destURL " << destURL.path() << " is not valid" << endl;
            return KURL();
        }
    }
    return destURL;
}

TransferGroup * Model::findGroup(const QString & groupName)
{
    QValueList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QValueList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    for(; it!=itEnd ; ++it)
    {
        if( (*it)->name() == groupName )
            return *it;
    }
    return 0;
}

Transfer * Model::findTransfer(KURL src)
{
    QValueList<TransferGroup *>::iterator it = m_transferGroups.begin();
    QValueList<TransferGroup *>::iterator itEnd = m_transferGroups.end();

    Transfer * t;

    for(; it!=itEnd ; ++it)
    {
        if( t = (*it)->findTransfer(src) )
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
    //TODO here we want only the TransferFactory items. so re-enable below
    offers = KTrader::self()->query( "KGet/Plugin"/*, str + "TransferFactory"*/ );

    QMap<int, KService::Ptr> services;
    QMap<int, KService::Ptr>::iterator it;

    for ( uint i = 0; i < offers.count(); ++i )
    {
        services[ offers[i]->property( "X-KDE-KGet-rank" ).toInt() ] = offers[i];
        kdDebug() << " TRANSFER FACTORY ITEM" << endl <<
         "  rank = " << offers[i]->property( "X-KDE-KGet-rank" ).toInt() << endl <<
         "  plugintype = " << offers[i]->property( "X-KDE-KGet-plugintype" ) << endl;
    }

    for( it = services.begin(); it != services.end(); ++it )
    {
        KGetPlugin * plugin;
        if( (plugin = createPluginFromService(*it)) != 0 )
        {
            kdDebug() << "plugin found and added to the list" << endl;
            m_transferFactories.prepend( static_cast<TransferFactory *>(plugin) );
        }
        else
            kdDebug() << "error" << endl;
    }
}

void Model::unloadPlugins()
{
    QValueList<KLibrary *>::iterator it = m_pluginKLibraries.begin();
    QValueList<KLibrary *>::iterator itEnd = m_pluginKLibraries.end();

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
    KLibrary *lib = loader->globalLibrary( QFile::encodeName( service->library() ) );

    kdDebug() << service->library() << endl;

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
        kdDebug() << "create_plugin == NULL" << endl;
        return 0;
    }

    m_pluginKLibraries.append(lib);

    return create_plugin();
}

bool Model::safeDeleteFile( const KURL& url )
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
