/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransferfactory.h"

// header inclusion order is crucial because of signal emit clashes
#include "bttransfer.h"
#include "bttransferhandler.h"
#include "btdetailswidget.h"
#include "advanceddetails/btadvanceddetailswidget.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN(BTTransferFactory)

BTTransferFactory::BTTransferFactory()
{
}

BTTransferFactory::~BTTransferFactory()
{
}

Transfer * BTTransferFactory::createTransfer(const KUrl &srcUrl, const KUrl &destUrl,
                                              TransferGroup * parent,
                                              Scheduler * scheduler, 
                                              const QDomElement * e )
{
    kDebug(5001) << "BTTransferFactory::createTransfer";

    if (srcUrl.fileName().endsWith(".torrent"))
    {
            return new BTTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

TransferHandler * BTTransferFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    BTTransfer * bttransfer = dynamic_cast<BTTransfer *>(transfer);

    if (!bttransfer)
    {
        kError(5001) << "BTTransferFactory::createTransferHandler: WARNING!\n"
                      "passing a non-BTTransfer pointer!!" << endl;
        return 0;
    }

    return new BTTransferHandler(bttransfer, scheduler);
}

QWidget * BTTransferFactory::createDetailsWidget( TransferHandler * transfer )
{
    BTTransferHandler * bttransfer = static_cast<BTTransferHandler *>(transfer);

    if (bttransfer->ready())
    {
        BTAdvancedDetailsWidget * details = new BTAdvancedDetailsWidget(bttransfer);
        details->show();
        details->resize(500, 400);
    }

    return new BTDetailsWidget(bttransfer);
}

const QList<KAction *> BTTransferFactory::actions()
{
    /**if (!handler)
        return QList<KAction *>();
    else**/
        return QList<KAction *>();
}

