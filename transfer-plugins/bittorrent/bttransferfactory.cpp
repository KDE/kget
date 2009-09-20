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
#include "btdatasource.h"
#include "bttransferhandler.h"
#include "btdetailswidget.h"
#include "advanceddetails/btadvanceddetailswidget.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN(BTTransferFactory)

BTTransferFactory::BTTransferFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
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

    if (srcUrl.fileName().endsWith(QLatin1String(".torrent")))
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
        kError(5001) << "WARNING! passing a non-BTTransfer pointer!!";
        return 0;
    }

    return new BTTransferHandler(bttransfer, scheduler);
}

QWidget * BTTransferFactory::createDetailsWidget( TransferHandler * transfer )
{
    BTTransferHandler * bttransfer = static_cast<BTTransferHandler *>(transfer);

    return new BTDetailsWidget(bttransfer);
}

const QList<KAction *> BTTransferFactory::actions(TransferHandler *handler)
{
     BTTransferHandler * bttransfer = static_cast<BTTransferHandler *>(handler);

     QList<KAction*> actions;
     if (bttransfer)
     {
         KAction *openAdvancedDetailsAction = new KAction(KIcon("document-open"), i18n("&Advanced Details"), this);
 
         connect(openAdvancedDetailsAction, SIGNAL(triggered()), bttransfer, SLOT(createAdvancedDetails()));
 
         actions.append(openAdvancedDetailsAction);

         KAction *openScanDlg = new KAction(KIcon("document-open"), i18n("&Scan Files"), this);
 
         connect(openScanDlg, SIGNAL(triggered()), bttransfer, SLOT(createScanDlg()));
 
         actions.append(openScanDlg);
     }
 
     if (bttransfer)
         return actions;
     else
         return QList<KAction *>();
}

TransferDataSource * BTTransferFactory::createTransferDataSource(const KUrl &srcUrl, const QDomElement &type)
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(type)
    /*if (srcUrl.fileName().endsWith(".torrent"))
        return new BTDataSource();*/
    return 0;
}

