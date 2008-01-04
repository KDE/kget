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

#include <kdebug.h>

KGET_EXPORT_PLUGIN( TransferKioFactory )

TransferKioFactory::TransferKioFactory()
{
}

TransferKioFactory::~TransferKioFactory()
{
}

Transfer * TransferKioFactory::createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001) << "TransferKioFactory::createTransfer";

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot;
    if(    prot == "http" || prot == "https" 
        || prot == "ftp"  || prot == "sftp"
        || prot == "file")
    {
        return new TransferKio(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

TransferHandler * TransferKioFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * TransferKioFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer);
    return 0;   //Temporary!!
}

QWidget * TransferKioFactory::createSettingsWidget(KDialog * parent)
{
    Q_UNUSED(parent);
    return 0; // if there is no settings widget we must return 0
}

const QList<KAction *> TransferKioFactory::actions()
{
    return QList<KAction *>();
}

