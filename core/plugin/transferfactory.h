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

#include "plugin.h"
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
                                           TransferGroup * parent, Scheduler * scheduler )=0;

        const QValueList<TransferAction *> & actions() {return m_actions;}

        KPopupMenu * createPopupMenu(QValueList<Transfer *> transfers);

    protected:
        QValueList<TransferAction *> m_actions;
};


/** -------- TransferFactory actions --------
 *
 *  Here are the TransferAction objects common to all the Transfer objects.
 */

class ActionStart : public TransferAction
{
    public:
        ActionStart( const QString& text, const QIconSet& pix, 
                     const KShortcut& cut, KActionCollection* parent, 
                     const char* name )
            : TransferAction(text, pix, cut, parent, name)
        {}

        void execute(Transfer * transfer)
        {
            transfer->handler()->start();
        }
};

class ActionStop : public TransferAction
{
    public:
        ActionStop( const QString& text, const QIconSet& pix, 
                    const KShortcut& cut, KActionCollection* parent, 
                    const char* name )
            : TransferAction(text, pix, cut, parent, name)
        {}

        void execute(Transfer * transfer)
        {
            transfer->handler()->stop();
        }
};

#endif
