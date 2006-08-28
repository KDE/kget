/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kurl.h>

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "transferMultiThreadedFactory.h"
#include "transferMultiThreaded.h"
#include "mtdetailswidget.h"

KGET_EXPORT_PLUGIN( TransferMultiThreadedFactory )

TransferMultiThreadedFactory::TransferMultiThreadedFactory()
{
}

TransferMultiThreadedFactory::~TransferMultiThreadedFactory()
{
}

Transfer * TransferMultiThreadedFactory::createTransfer( KUrl srcUrl, KUrl destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001) << "TransferMultiThreadedFactory::createTransfer" << endl;

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot << endl;
    if( prot == "http" || prot == "ftp" )
    {
        return new TransferMultiThreaded(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

TransferHandler * TransferMultiThreadedFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * TransferMultiThreadedFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer);
    return new MTDetailsWidget();   //Temporary!!
}

const QList<KAction *> TransferMultiThreadedFactory::actions()
{
    return QList<KAction *>();
}

