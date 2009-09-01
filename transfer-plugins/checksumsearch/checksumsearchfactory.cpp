/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "checksumsearchfactory.h"
#include "checksumsearchtransferdatasource.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"

#include <QtXml/QDomElement>

#include <kdebug.h>

KGET_EXPORT_PLUGIN( ChecksumSearchFactory )

ChecksumSearchFactory::ChecksumSearchFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

ChecksumSearchFactory::~ChecksumSearchFactory()
{
}

Transfer * ChecksumSearchFactory::createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001);
    Q_UNUSED(srcUrl);
    Q_UNUSED(destUrl);
    Q_UNUSED(parent);
    Q_UNUSED(scheduler);
    Q_UNUSED(e);
/**
*We do not implement the transfer interface
**/
    return 0;
}

TransferHandler * ChecksumSearchFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * ChecksumSearchFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer);
    return 0;   //Temporary!!
}

const QList<KAction *> ChecksumSearchFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler);
    return QList<KAction *>();
}

TransferDataSource *ChecksumSearchFactory::createTransferDataSource(const KUrl &srcUrl, const QDomElement &type)
{
    kDebug(5001);

    if (type.attribute("type") == "checksumsearch")
    {
        return new ChecksumSearchTransferDataSource(srcUrl);
    }
    return 0;
}
