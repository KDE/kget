/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qfileinfo.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "transfers/kio/transferKio.h"
#include "conf/settings.h"
#include "model.h"
#include "transfergroup.h"

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

void Model::addTransfer( KURL src, const QString& destDir,
                         const QString& groupName )
{
    bool ok = false;

    while (!ok) 
    {
        QString newtransfer = KInputDialog::getText(i18n("New Download"), i18n("Enter URL:"), newtransfer, &ok, 0/*mainWidget*/);

        // user presses cancel
        if (!ok)
            return;

        src = KURL::fromPathOrURL(newtransfer);

        if (!src.isValid())
        {
            KMessageBox::error(0/*mainWidget*/, i18n("Malformed URL:\n%1").arg(newtransfer));
            ok = false;
        }
    }

    KURL destFile;
    QString fileName;

    /**
     * destDir not empty: if the file exists the function
     * asks the user to confirm the overwriting action
     */
    if ( !destDir.isEmpty() )
    {
        // create a proper destination file from destDir
        KURL destURL = KURL::fromPathOrURL( destDir );
        fileName = src.fileName();

        // in case the fileName is empty, we simply ask for a filename in
        // addTransferEx. Do NOT attempt to use an empty filename, that
        // would be a directory (and we don't want to overwrite that!)
        if ( !fileName.isEmpty() )
        {
            destURL.adjustPath( +1 );
            destURL.setFileName( fileName );
            if(KIO::NetAccess::exists(destURL, false, 0/*mainWidget*/))
            {
                if (KMessageBox::warningYesNo(0/*mainWidget*/,i18n("Destination file \n%1\nalready exists.\nDo you want to overwrite it?").arg( destURL.prettyURL()) )
                    == KMessageBox::Yes)
                {
                    //TODO remove the safedelete.cpp file (maybe importing that code here)
                    safeDeleteFile( destURL );
                    destFile = destURL;
                }
            }
        }
    }


    //WARNING! HARDCODED DESTINATION DIRECTORY
    KURL destURL(QString("file:///tmp/") + fileName);

    // simply delete it, the calling process should have asked if you
    // really want to delete (at least khtml does)
    if(KIO::NetAccess::exists(destURL, false, 0/*mainWidget*/))
        safeDeleteFile( destURL );

    QMap<QString, TransferGroup *>::iterator it = m_transferGroups.find(groupName);

    if( it != m_transferGroups.end() )
    {
        //The group with name "groupName" has been found
        //WARNING! HARDCODED TRANSFER CREATION. Here we should use the
        //Transfer Factory
        (*it)->append( new TransferKio( (*it), &m_scheduler, src, destURL) );
    }
    else
    {
        //Group with name "groupName" not found
        kdDebug() << "Error:    group not found: " << groupName << endl;
        return;
    }

    //TODO URGENT!! RE-ENABLE THESE LINES
    /*    TransferList list(item);

    emit addedItems(list);

    queueEvaluateItems(list);
    queueUpdate();    */
}

void Model::delTransfer(TransferHandler * transfer)
{
    
}

void Model::moveTransfer(TransferHandler * transfer, const QString& groupName)
{
    
}

// ------ STATIC MEMBERS INITIALIZATION ------
QMap<QString, TransferGroup *> Model::m_transferGroups = QMap<QString, TransferGroup *>();
QValueList<ModelObserver *> Model::m_observers = QValueList<ModelObserver *>();
Scheduler Model::m_scheduler = Scheduler();

// ------ PRIVATE FUNCTIONS ------
Model::Model()
{
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
