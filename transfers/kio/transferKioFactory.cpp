/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <qstring.h>

#include <kdebug.h>
#include <kurl.h>

#include "core/transfergroup.h"
#include "transferKioFactory.h"

KGET_EXPORT_PLUGIN( TransferKioFactory )

TransferKioFactory::TransferKioFactory()
{
    
}

TransferKioFactory::~TransferKioFactory()
{
    
}

Transfer * TransferKioFactory::createTransfer( KURL src, const QString& destDir, 
                                               TransferGroup * parent )
{
    QString protocol = src.protocol();
    kdDebug() << "Protocol = " << protocol << endl;
    //if( protocol)
}
