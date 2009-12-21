/* This file is part of the KDE project

   Copyright (C) 2004 Ningyu Shi <shiningyu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef CONTENT_FETCH_FACTORY_H
#define CONTENT_FETCH_FACTORY_H

#include "core/plugin/transferfactory.h"

#include <QStringList>
#include <QList>

class Transfer;
class TransferGroup;
class Scheduler;

class ContentFetchFactory : public TransferFactory
{
    Q_OBJECT
    public:
        ContentFetchFactory(QObject *parent, const QVariantList &args);
        ~ContentFetchFactory();

        Transfer * createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                   TransferGroup * parent, Scheduler * scheduler,
                                   const QDomElement * e = 0 );

        TransferHandler * createTransferHandler(Transfer * transfer,
                                                Scheduler * scheduler);

        QWidget * createDetailsWidget( TransferHandler * transfer );

        const QList<KAction *> actions(TransferHandler *handler = 0);
};

#endif // CONTENT_FETCH_FACTORY_H
