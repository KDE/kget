/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef BTTRANSFERFACTORY_H
#define BTTRANSFERFACTORY_H

#include "core/plugin/transferfactory.h"

class BTTransferFactory : public TransferFactory
{
    public:
        BTTransferFactory();
        ~BTTransferFactory();

        Transfer * createTransfer(const KUrl &srcUrl, const KUrl &destUrl,
                                   TransferGroup * parent,
                                   Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler);

        QWidget * createDetailsWidget( TransferHandler * transfer );

        const QList<KAction *> actions();

        QWidget * createSettingsWidget() { return 0;}

        QString displayName(){return "Bittorrent";}
};

#endif
