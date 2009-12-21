/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "checksumsearchfactory.h"
#include "checksumsearchtransferdatasource.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"

#include <QtXml/QDomElement>

#include <kdebug.h>

KGET_EXPORT_PLUGIN( ChecksumSearchFactory )

ChecksumSearchFactory::ChecksumSearchFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

ChecksumSearchFactory::~ChecksumSearchFactory()
{
}

TransferDataSource *ChecksumSearchFactory::createTransferDataSource(const KUrl &srcUrl, const QDomElement &type, QObject *parent)
{
    kDebug(5001);

    if (type.attribute("type") == "checksumsearch") {
        return new ChecksumSearchTransferDataSource(srcUrl, parent);
    }
    return 0;
}
