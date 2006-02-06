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
#include "transfers/kio/transferKioFactory.h"
#include "transfers/kio/transferKio.h"

KGET_EXPORT_PLUGIN( TransferKioFactory )

TransferKioFactory::TransferKioFactory()
{
}

TransferKioFactory::~TransferKioFactory()
{
}

Transfer * TransferKioFactory::createTransfer( KUrl srcURL, KUrl destURL,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug() << "TransferKioFactory::createTransfer" << endl;

    QString prot = srcURL.protocol();
    kDebug() << "Protocol = " << prot << endl;
    if(    prot == "http" || prot == "https" 
        || prot == "ftp"  || prot == "sftp"
        || prot == "file")
    {
        return new TransferKio(parent, this, scheduler, srcURL, destURL, e);
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

const QList<KAction *> TransferKioFactory::actions()
{
    return QList<KAction *>();
}

