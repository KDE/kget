/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfermultisegkiofactory.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "multisegkiosettings.h"
#include "transfermultisegkio.h"
#include "multisegkiodatasource.h"

#include <QDomElement>

#include "kget_debug.h"
#include <QDebug>
#include "kget_macro.h"

K_PLUGIN_CLASS_WITH_JSON(TransferMultiSegKioFactory, "kget_multisegkiofactory.json")

TransferMultiSegKioFactory::TransferMultiSegKioFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

TransferMultiSegKioFactory::~TransferMultiSegKioFactory()
{
}

Transfer * TransferMultiSegKioFactory::createTransfer( const QUrl &srcUrl, const QUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler,
                                               const QDomElement * e )
{
    qCDebug(KGET_DEBUG);

    if (isSupported(srcUrl) && (!e || !e->firstChildElement("factories").isNull()))
    {
       return new TransferMultiSegKio(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return nullptr;
}

TransferHandler * TransferMultiSegKioFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * TransferMultiSegKioFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer)
    return nullptr;   //Temporary!!
}

const QList<QAction *> TransferMultiSegKioFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler)
    return QList<QAction *>();
}

 TransferDataSource * TransferMultiSegKioFactory::createTransferDataSource(const QUrl &srcUrl, const QDomElement &type, QObject *parent)
{
    qCDebug(KGET_DEBUG);

    //only use this TransferDataSource if no type is specified and the protocols match
    if (!type.attribute("type").isEmpty())
    {
        return nullptr;
    }

    if (isSupported(srcUrl))
    {
        return new MultiSegKioDataSource(srcUrl, parent);
    }
    return nullptr;
}

bool TransferMultiSegKioFactory::isSupported(const QUrl &url) const
{
    QString prot = url.scheme();
    qCDebug(KGET_DEBUG) << "Protocol = " << prot;
    return addsProtocols().contains(prot);
}

QStringList TransferMultiSegKioFactory::addsProtocols() const
{
    static const QStringList protocols = QStringList() << "http" << "https" << "ftp" << "sftp";
    return protocols;
}

#include "transfermultisegkiofactory.moc"
