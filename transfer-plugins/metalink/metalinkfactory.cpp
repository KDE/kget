/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "metalinkfactory.h"
#include "metalink.h"

KGET_EXPORT_PLUGIN( metalinkFactory )

metalinkFactory::metalinkFactory()
{
}

metalinkFactory::~metalinkFactory()
{
}

Transfer * metalinkFactory::createTransfer( KUrl srcUrl, KUrl destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001) << "metalinkFactory::createTransfer" << endl;

    if( srcUrl.fileName().endsWith (".metalink") )
    {
        return new metalink(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

TransferHandler * metalinkFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    return new TransferHandler(transfer, scheduler);
}

QWidget * metalinkFactory::createDetailsWidget( TransferHandler * transfer )
{
  Q_UNUSED(transfer);
    return new QWidget();   //Temporary!!
}

QWidget * metalinkFactory::createSettingsWidget()
{
    return 0; // if there is no settings widget we must return 0
}

const QList<KAction *> metalinkFactory::actions()
{
    return QList<KAction *>();
}

