/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#ifndef KIO_FACTORY_H
#define KIO_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;

class TransferKioFactory : public TransferFactory
{
    public:
        TransferKioFactory();
        ~TransferKioFactory();

        Transfer * createTransfer( KUrl srcUrl, KUrl destUrl,
                                   TransferGroup * parent, Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler);

        QWidget * dlgSettings();

        QWidget * createDetailsWidget( TransferHandler * transfer );

        const QList<KAction *> actions();
};

#endif
