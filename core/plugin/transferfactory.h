/* This file is part of the KDE project

   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
   Copyright (C) 2009 by Lukas Appelhans <l.appelhans@gmx.de>

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
#include <QStringList>

#include "core/plugin/plugin.h"
#include "core/kget.h"
#include "core/transfer.h"
#include "core/transferhandler.h"
#include "core/transferdatasource.h"
#include "kget_export.h"

class TransferGroup;
class Scheduler;
class KDialog;

/**
 * @short TransferFactory
 *
 * desc to come...
 */
class KGET_EXPORT TransferFactory : public KGetPlugin
{
    Q_OBJECT
    public:
        TransferFactory(QObject *parent, const QVariantList &args);

        virtual Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                           TransferGroup * parent,
                                           Scheduler * scheduler,
                                           const QDomElement * n = 0 );

        virtual TransferHandler * createTransferHandler(Transfer * transfer,
                                                        Scheduler * scheduler);

        virtual QWidget * createDetailsWidget(TransferHandler * transfer);

        virtual KDialog * createNewTransferDialog(const KUrl &srcUrl, const QString &suggestedFileName = QString(), TransferGroupHandler * defaultGroup = 0);

        virtual const QList<KAction *> actions(TransferHandler *handler = 0);

        virtual void settingsChanged() {}
        
        virtual bool isSupported(const KUrl &url) const;

        /**
         * Returns a list of protocols for which the TransferFactory adds support.
         * An empty list simply means that the TransferFactory does not add support
         * for the urls and might internally resort on other TransferFactories
         */
        virtual QStringList addsProtocols() const;

        /**
         * Returns a Data Source. needed for Transfers Containers if any.
         * default implementation returns 0
         */
        virtual TransferDataSource * createTransferDataSource(const KUrl &srcUrl, const QDomElement &type, QObject *parent);
};

#endif
