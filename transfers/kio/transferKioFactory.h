/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef _KIO_FACTORY_H
#define _KIO_FACTORY_H

#include <qstring.h>
#include <kurl.h>

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;

class TransferKioFactory : public TransferFactory
{
    public:
        TransferKioFactory();
        ~TransferKioFactory();

        Transfer * createTransfer( KURL src, const QString& destDir,
                                   TransferGroup * parent );
};

#endif
