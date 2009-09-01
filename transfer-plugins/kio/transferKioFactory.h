/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIO_FACTORY_H
#define KIO_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;

class TransferKioFactory : public TransferFactory
{
    Q_OBJECT
    public:
        TransferKioFactory(QObject *parent, const QVariantList &args);
        ~TransferKioFactory();

        Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                   TransferGroup * parent, Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler);

        QWidget * createDetailsWidget( TransferHandler * transfer );

        QWidget * createSettingsWidget(KDialog * parent);

        QString displayName(){return "HTTP(s) / FTP(s)";}

        const QList<KAction *> actions(TransferHandler *handler = 0);
        TransferDataSource * createTransferDataSource(const KUrl &srcUrl, const QDomElement &type) {Q_UNUSED(srcUrl); Q_UNUSED(type); return 0;}
};

#endif
