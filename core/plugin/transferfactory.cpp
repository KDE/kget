/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2009 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferfactory.h"

#include "kget.h"

#include <KLocalizedString>
#include <QDebug>

TransferFactory::TransferFactory(QObject *parent, const QVariantList &args)
  : KGetPlugin(parent, args)
{

}

Transfer * TransferFactory::createTransfer(const QUrl &srcUrl, const QUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler,
                                               const QDomElement * n)
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(destUrl)
    Q_UNUSED(parent)
    Q_UNUSED(scheduler)
    Q_UNUSED(n)
    return nullptr;
}

TransferHandler * TransferFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * TransferFactory::createDetailsWidget(TransferHandler * transfer)
{
    Q_UNUSED(transfer)
    return nullptr;
}
        
QDialog * TransferFactory::createNewTransferDialog(const QUrl &srcUrl, const QString &suggestedFileName, TransferGroupHandler * defaultGroup)
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(suggestedFileName)
    Q_UNUSED(defaultGroup)
    return nullptr;
}

const QList<QAction *> TransferFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler)
    return QList<QAction *>();
}

TransferDataSource * TransferFactory::createTransferDataSource(const QUrl &srcUrl, const QDomElement &type, QObject *parent)
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(type)
    Q_UNUSED(parent)
    return nullptr;
}

bool TransferFactory::isSupported(const QUrl &url) const
{
    Q_UNUSED(url)
    return false;
}

QStringList TransferFactory::addsProtocols() const
{
    return QStringList();
}

QString TransferFactory::displayName() const
{
    return QStringLiteral("Undefined");
}

