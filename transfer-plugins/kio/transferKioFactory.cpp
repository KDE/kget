/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "transferKioFactory.h"
#include "transferKio.h"

KGET_EXPORT_PLUGIN( TransferKioFactory )

TransferKioFactory::TransferKioFactory()
{
}

TransferKioFactory::~TransferKioFactory()
{
}

Transfer * TransferKioFactory::createTransfer( KUrl srcUrl, KUrl destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001) << "TransferKioFactory::createTransfer" << endl;

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot << endl;
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
    return new QWidget();   //Temporary!!
}

QWidget * TransferKioFactory::createSettingsWidget()
{
    return 0; // if there is no settings widget we must return 0
}

const QList<KAction *> TransferKioFactory::actions()
{
    return QList<KAction *>();
}

