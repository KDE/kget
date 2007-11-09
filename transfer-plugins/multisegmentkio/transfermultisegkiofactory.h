/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MULTISEGKIO_FACTORY_H
#define MULTISEGKIO_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;

class TransferMultiSegKioFactory : public TransferFactory
{
    public:
        TransferMultiSegKioFactory();
        ~TransferMultiSegKioFactory();

        Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                 TransferGroup * parent, Scheduler * scheduler,
                                 const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                              Scheduler * scheduler);

        QWidget * createDetailsWidget( TransferHandler * transfer );

        QWidget * createSettingsWidget();

        QString displayName(){return "Multithreaded HTTP(s) / FTP(s)";}

        const QList<KAction *> actions();
};

#endif
