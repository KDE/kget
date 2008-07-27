/* This file is part of the KDE project

   Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "contentfetchfactory.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "contentfetch.h"

#include <kdebug.h>

KGET_EXPORT_PLUGIN( ContentFetchFactory )

ContentFetchFactory::ContentFetchFactory(QObject *parent,
					 const QVariantList &args)
  : TransferFactory(parent, args)
{
}

ContentFetchFactory::~ContentFetchFactory()
{
}

Transfer * ContentFetchFactory::createTransfer( const KUrl &srcUrl,
						const KUrl &destUrl,
						TransferGroup * parent,
						Scheduler * scheduler,
						const QDomElement * e )
{
    // Hardcoded check for test
    if (srcUrl == KUrl("http://www.ustc.edu.cn/xx.html"))
    {
	kDebug(5001) << "ContentFetchFactory::createTransfer";
	return new ContentFetch(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

TransferHandler * ContentFetchFactory::createTransferHandler(
    Transfer * transfer,
    Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * ContentFetchFactory::createDetailsWidget(TransferHandler *transfer)
{
    Q_UNUSED(transfer);
    return 0;   //Temporary!!
}

QWidget * ContentFetchFactory::createSettingsWidget(KDialog *parent)
{
    Q_UNUSED(parent);
    return 0; // if there is no settings widget we must return 0
}

const QList<KAction*> ContentFetchFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler);
    return QList<KAction*>();
}

