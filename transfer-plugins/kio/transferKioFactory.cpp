/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferKioFactory.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "transferKio.h"

#include "kget_debug.h"
#include <KPluginFactory>
#include <QDebug>

K_PLUGIN_CLASS_WITH_JSON(TransferKioFactory, "kget_kiofactory.json")

TransferKioFactory::TransferKioFactory(QObject *parent, const QVariantList &args)
    : TransferFactory(parent, args)
{
}

TransferKioFactory::~TransferKioFactory()
{
}

Transfer *TransferKioFactory::createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup *parent, Scheduler *scheduler, const QDomElement *e)
{
    qCDebug(KGET_DEBUG) << "TransferKioFactory::createTransfer";
    qWarning(KGET_DEBUG) << "KIOFACTORY createTRANSFER";

    if (isSupported(srcUrl)) {
        return new TransferKio(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return nullptr;
}

bool TransferKioFactory::isSupported(const QUrl &url) const
{
    QString prot = url.scheme();
    qDebug() << "Protocol = " << prot;
    return addsProtocols().contains(prot);
}

QStringList TransferKioFactory::addsProtocols() const
{
    static const QStringList protocols = QStringList() << "http"
                                                       << "https"
                                                       << "ftp"
                                                       << "sftp";
    return protocols;
}

#include "transferKioFactory.moc"
