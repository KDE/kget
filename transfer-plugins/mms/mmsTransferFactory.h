/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MMSFACTORY_H
#define MMSFACTORY_H

#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"

class Transfer;
class TransferGroup;
class Scheduler;

class MmsTransferFactory : public TransferFactory
{
    Q_OBJECT
    public:
        MmsTransferFactory(QObject *parent, const QVariantList &args);
        ~MmsTransferFactory();

        Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                   TransferGroup * parent, Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler) {return new TransferHandler(transfer, scheduler);}

        QWidget * createDetailsWidget( TransferHandler * transfer ) {return 0;}

        QWidget * createSettingsWidget(KDialog * parent) {return 0;}

        QString displayName(){return "Microsoft Media Stream";}

        const QList<KAction *> actions(TransferHandler *handler = 0) {return QList<KAction *>();}
        TransferDataSource * createTransferDataSource(const KUrl &srcUrl) {Q_UNUSED(srcUrl); return 0;}
};

#endif
