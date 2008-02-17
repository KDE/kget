/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "mirrorsearchfactory.h"
#include "dlgmirrorsearch.h"
#include "mirrorsearchtransferdatasource.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN( MirrorSearchFactory )

MirrorSearchFactory::MirrorSearchFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

MirrorSearchFactory::~MirrorSearchFactory()
{
}

Transfer * MirrorSearchFactory::createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
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

TransferHandler * MirrorSearchFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * MirrorSearchFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer);
    return 0;   //Temporary!!
}

QWidget * MirrorSearchFactory::createSettingsWidget(KDialog * parent)
{
    kDebug(5001);
    return new DlgSettingsWidget(parent);
}

const QList<KAction *> MirrorSearchFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler);
    return QList<KAction *>();
}

TransferDataSource *MirrorSearchFactory::createTransferDataSource(const KUrl &srcUrl)
{
    kDebug(5001);

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot;
/**
We will use a bogus protocol named "search" for this
*/
    if( prot == "search" )
    {
        return new MirrorSearchTransferDataSource(srcUrl);
    }
    return 0;
}
