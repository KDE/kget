/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef MIRRORSEARCH_FACTORY_H
#define MIRRORSEARCH_FACTORY_H

#include "core/plugin/transferfactory.h"

class Transfer;
class TransferGroup;
class Scheduler;
class TransferDataSource;

class MirrorSearchFactory : public TransferFactory
{
    Q_OBJECT
    public:
        MirrorSearchFactory(QObject *parent, const QVariantList &args);
        ~MirrorSearchFactory();

        TransferDataSource * createTransferDataSource(const KUrl &srcUrl, const QDomElement &type, QObject *parent);
};

#endif
