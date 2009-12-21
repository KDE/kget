/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "mirrorsearchfactory.h"
#include "mirrorsearchtransferdatasource.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"

#include <QtXml/QDomElement>

#include <kdebug.h>

KGET_EXPORT_PLUGIN( MirrorSearchFactory )

MirrorSearchFactory::MirrorSearchFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

MirrorSearchFactory::~MirrorSearchFactory()
{
}

TransferDataSource *MirrorSearchFactory::createTransferDataSource(const KUrl &srcUrl, const QDomElement &type, QObject *parent)
{
    kDebug(5001);

    if (type.attribute("type") == "search") {
        return new MirrorSearchTransferDataSource(srcUrl, parent);
    }
    return 0;
}
