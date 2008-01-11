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
#include "dlgmultisegkio.h"
#include "transfermultisegkio.h"
#include "multisegkiodatasource.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN( TransferMultiSegKioFactory )

TransferMultiSegKioFactory::TransferMultiSegKioFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

TransferMultiSegKioFactory::~TransferMultiSegKioFactory()
{
}

Transfer * TransferMultiSegKioFactory::createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler,
                                               const QDomElement * e )
{
    kDebug(5001);

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot;
    if( (prot == "http" || prot == "https" ||
         prot == "ftp"  || prot == "sftp")  &&
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

QWidget * TransferMultiSegKioFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer);
    return 0;   //Temporary!!
}

QWidget * TransferMultiSegKioFactory::createSettingsWidget(KDialog * parent)
{
    return new DlgSettingsWidget(parent);
}

const QList<KAction *> TransferMultiSegKioFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler);
    return QList<KAction *>();
}

 TransferDataSource * TransferMultiSegKioFactory::createTransferDataSource(const KUrl &srcUrl)
{
   kDebug(5001);

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot;
    if( prot == "http" || prot == "https" ||
        prot == "ftp"  || prot == "sftp"
      )
    {
        return new MultiSegKioDataSource();
    }
}
