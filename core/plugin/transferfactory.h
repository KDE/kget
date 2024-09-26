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
 * Common fields in the JSON file:
{
    "KPlugin": {
        "Name": "MyPlugin"

    },
    "X-KDE-ConfigModule": "kget_kcms/kcm",
    "X-KDE-KGet-framework-version": "2",
    "X-KDE-KGet-rank": "70"
}
 *
 * @see kget_plugin.desktop - for "KGet/Plugin" definition
 * @see transfers/kio/kget_kiotransfer.desktop - desktop entry example
 */

#include <QAction>
#include <QStringList>

#include "core/kget.h"
#include "core/plugin/plugin.h"
#include "core/transfer.h"
#include "core/transferdatasource.h"
#include "core/transferhandler.h"
#include "kget_export.h"
#include "kget_macro.h"

class TransferGroup;
class Scheduler;
class QDialog;

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

    virtual Transfer *createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup *parent, Scheduler *scheduler, const QDomElement *n = nullptr);

    virtual TransferHandler *createTransferHandler(Transfer *transfer, Scheduler *scheduler);

    virtual QWidget *createDetailsWidget(TransferHandler *transfer);

    virtual QDialog *createNewTransferDialog(const QUrl &srcUrl, const QString &suggestedFileName = QString(), TransferGroupHandler *defaultGroup = nullptr);

    virtual const QList<QAction *> actions(TransferHandler *handler = nullptr);

    virtual void settingsChanged()
    {
    }

    virtual bool isSupported(const QUrl &url) const;

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
    virtual TransferDataSource *createTransferDataSource(const QUrl &srcUrl, const QDomElement &type, QObject *parent);

    virtual QString displayName() const;
};

#endif
