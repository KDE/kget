/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KGET_TRANSFERFACTORY_H
#define KGET_TRANSFERFACTORY_H

/* TransferFactory [KGet/Plugin]
 *
 * Defines a ...XXX...
 *
 * Common fields in the [Desktop Entry]:
 *   Type=Service
 *   ServiceTypes=KGet/Plugin
 *   X-KDE-KGet-plugintype=TransferFactory
 *   X-KDE-KGet-framework-version=1
 * Custom fields in the [Desktop Entry]:
 *   Name=%YOURTRANSFERFACTORY%
 *   X-KDE-Library=lib%YOURLIBRARY%
 *   X-KDE-KGet-rank=%PLUGINRANK%
 *
 * @see kget_plugin.desktop - for "KGet/Plugin" definition
 * @see transfers/kio/kget_kiotransfer.desktop - desktop entry example
 */

#include <qvaluelist.h>

#include <kapplication.h>

#include "core/plugin/plugin.h"
#include "core/model.h"
#include "core/transfer.h"
#include "core/transferhandler.h"
#include "core/transferaction.h"

class KURL;
class KPopupMenu;

class TransferGroup;
class Scheduler;

/**
 * @short TransferFactory
 *
 * desc to come...
 */
class TransferFactory : public KGetPlugin
{
    public:
        TransferFactory();

        virtual Transfer * createTransfer( KURL srcURL, KURL destURL,
                                           TransferGroup * parent,
                                           Scheduler * scheduler,
                                           const QDomElement * n = 0 )=0;

        const QValueList<TransferAction *> & actions() {return m_actions;}

        KPopupMenu * createPopupMenu(QValueList<TransferHandler *> transfers);

    protected:
        QValueList<TransferAction *> m_actions;
};


/**----------------------- TransferFactory actions -----------------------
 *
 *  Here are the TransferAction objects common to all the Transfer objects.
 */

class ActionStart : public TransferAction
{
    public:
        ActionStart( const QString& text, const QString& pix, 
                     const KShortcut& cut, KActionCollection* parent, 
                     const char* name )
            : TransferAction(text, pix, cut, parent, name)
        {}

        void execute(const QValueList<TransferHandler *> & transfers)
        {
            QValueList<TransferHandler *>::const_iterator it = transfers.begin();
            QValueList<TransferHandler *>::const_iterator itEnd = transfers.end();

            for( ; it!=itEnd ; ++it )
                (*it)->start();
        }
};

class ActionStop : public TransferAction
{
    public:
        ActionStop( const QString& text, const QString& pix, 
                    const KShortcut& cut, KActionCollection* parent, 
                    const char* name )
            : TransferAction(text, pix, cut, parent, name)
        {}

        void execute(const QValueList<TransferHandler *> & transfers)
        {
            QValueList<TransferHandler *>::const_iterator it = transfers.begin();
            QValueList<TransferHandler *>::const_iterator itEnd = transfers.end();

            for( ; it!=itEnd ; ++it )
                (*it)->stop();
        }
};

class ActionDelete : public TransferAction
{
    public:
        ActionDelete( const QString& text, const QString& pix,
                      const KShortcut& cut, KActionCollection* parent,
                      const char* name )
            : TransferAction(text, pix, cut, parent, name)
        {}

        void execute(const QValueList<TransferHandler *> & transfers)
        {
            QValueList<TransferHandler *>::const_iterator it = transfers.begin();
            QValueList<TransferHandler *>::const_iterator itEnd = transfers.end();

            for( ; it!=itEnd ; ++it )
                Model::delTransfer(*it);
        }
};

class ActionOpenDestination : public TransferAction
{
    public:
        ActionOpenDestination( const QString& text, const QString& pix,
                               const KShortcut& cut, KActionCollection* parent,
                               const char* name )
            : TransferAction(text, pix, cut, parent, name)
        {}

        void execute(const QValueList<TransferHandler *> & transfers)
        {
            QStringList openedDirs;

            QValueList<TransferHandler *>::const_iterator it = transfers.begin();
            QValueList<TransferHandler *>::const_iterator itEnd = transfers.end();

            for( ; it!=itEnd ; ++it )
            {
                QString directory = (*it)->dest().directory();
                if( !openedDirs.contains( directory ) )
                {
                    kapp->invokeBrowser( directory );
                    openedDirs.append( directory );
                }
            }
        }
};

#endif
