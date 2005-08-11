/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qstring.h>
#include <QWidget>

#include <kdebug.h>
#include <kurl.h>

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

Transfer * TransferKioFactory::createTransfer( KURL srcURL, KURL destURL,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kdDebug() << "TransferKioFactory::createTransfer" << endl;

    QString prot = srcURL.protocol();
    kdDebug() << "Protocol = " << prot << endl;
    if(    prot == "http" || prot == "https" 
        || prot == "ftp"  || prot == "sftp"
        || prot == "file")
    {
        return new TransferKio(parent, this, scheduler, srcURL, destURL, e);
    }
    return 0;
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

