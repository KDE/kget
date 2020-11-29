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
    Q_OBJECT
    public:
        BTTransferFactory(QObject *parent, const QVariantList &args);
        ~BTTransferFactory() override;

        Transfer * createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup * parent, Scheduler * scheduler, const QDomElement * e = nullptr) override;

        TransferHandler * createTransferHandler(Transfer * transfer, Scheduler * scheduler) override;

        QWidget * createDetailsWidget(TransferHandler * transfer) override;

        const QList<QAction *> actions(TransferHandler * handler = nullptr) override;

        TransferDataSource * createTransferDataSource(const QUrl &srcUrl, const QDomElement &type, QObject *parent) override;

        bool isSupported(const QUrl &url) const override;
};

#endif
