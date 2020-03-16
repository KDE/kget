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
    Q_OBJECT
    public:
        TransferMultiSegKioFactory(QObject *parent, const QVariantList &args);
        ~TransferMultiSegKioFactory() override;

        Transfer * createTransfer( const QUrl &srcUrl, const QUrl &destUrl,
                                 TransferGroup * parent, Scheduler * scheduler,
                                 const QDomElement * e = nullptr ) override;

        TransferHandler * createTransferHandler(Transfer * transfer,
                                              Scheduler * scheduler) override;

        QWidget * createDetailsWidget( TransferHandler * transfer ) override;

        const QList<QAction *> actions(TransferHandler *handler = nullptr) override;
        TransferDataSource * createTransferDataSource(const QUrl &srcUrl, const QDomElement &type, QObject *parent) override;
        bool isSupported(const QUrl &url) const override;
        QStringList addsProtocols() const override;
};

#endif
