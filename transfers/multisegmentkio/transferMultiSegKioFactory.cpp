/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "MultiSegKioSettings.h"
#include "dlgmultisegkio.h"
#include "transfers/multisegmentkio/transferMultiSegKioFactory.h"
#include "transfers/multisegmentkio/transferMultiSegKio.h"

KGET_EXPORT_PLUGIN( TransferMultiSegKioFactory )

TransferMultiSegKioFactory::TransferMultiSegKioFactory()
{
}

TransferMultiSegKioFactory::~TransferMultiSegKioFactory()
{
}

Transfer * TransferMultiSegKioFactory::createTransfer( KUrl srcUrl, KUrl destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler,
                                               const QDomElement * e )
{
   kDebug(5001) << "TransferMultiSegKioFactory::createTransfer" << endl;

   QString prot = srcUrl.protocol();
   kDebug(5001) << "Protocol = " << prot << endl;
   if( prot == "http" || prot == "https" ||
       prot == "ftp"  || prot == "sftp"  &&
       MultiSegKioSettings::segments() > 1
     )
   {
      return new transferMultiSegKio(parent, this, scheduler, srcUrl, destUrl, e);
   }
   return 0;
}

TransferHandler * TransferMultiSegKioFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
   return new TransferHandler(transfer, scheduler);
}

QWidget * TransferMultiSegKioFactory::dlgSettings()
{
   return new dlgSettingsWidget();
}

QWidget * TransferMultiSegKioFactory::createDetailsWidget( TransferHandler * transfer )
{
   Q_UNUSED(transfer);
   return new QWidget();   //Temporary!!
}

const QList<KAction *> TransferMultiSegKioFactory::actions()
{
   return QList<KAction *>();
}
