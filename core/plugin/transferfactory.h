/* This file is part of the KDE project

   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

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

#include <kaction.h>

#include "core/plugin/plugin.h"
#include "core/kget.h"
#include "core/transfer.h"
#include "core/transferhandler.h"
#include "kget_export.h"

class TransferGroup;
class Scheduler;

/**
 * @short TransferFactory
 *
 * desc to come...
 */
class KGET_EXPORT TransferFactory : public KGetPlugin
{
    public:
        TransferFactory();

        virtual Transfer * createTransfer( KUrl srcURL, KUrl destURL,
                                           TransferGroup * parent,
                                           Scheduler * scheduler,
                                           const QDomElement * n = 0 )=0;

        virtual TransferHandler * createTransferHandler(Transfer * transfer,
                                                        Scheduler * scheduler)=0;

        virtual QWidget * createDetailsWidget( TransferHandler * transfer )=0;

        virtual const QList<KAction *> actions()=0;

        KMenu * createPopupMenu(QList<TransferHandler *> transfers);
};

#endif
