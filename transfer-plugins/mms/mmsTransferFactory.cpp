/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "mmsTransferFactory.h"

#include "mmsTransfer.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN( MmsTransferFactory )

MmsTransferFactory::MmsTransferFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

MmsTransferFactory::~MmsTransferFactory()
{
}

Transfer * MmsTransferFactory::createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001) << "MmsTransferFactory::createTransfer";

    if (isSupported(srcUrl))
    {
        return new MmsTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

bool MmsTransferFactory::isSupported(const KUrl &src) const
{
    return src.protocol() == "mms";
}

