/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2012 Aish Raj Dahal <dahalaishraj@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "metalinkfactory.h"

#include "core/scheduler.h"
#include "core/transfergroup.h"
#include "metalinkhttp.h"
#include "metalinkxml.h"

#include "kget_debug.h"
#include <QDebug>

K_PLUGIN_CLASS_WITH_JSON(MetalinkFactory, "kget_metalinkfactory.json")

MetalinkFactory::MetalinkFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{
}

MetalinkFactory::~MetalinkFactory()
{
}

Transfer * MetalinkFactory::createTransfer( const QUrl &srcUrl, const QUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler,
                                               const QDomElement * e )
{
    qCDebug(KGET_DEBUG) << "metalinkFactory::createTransfer";
    auto *metalinkHttpChecker = new KGetMetalink::MetalinkHttpParser(srcUrl);

    if (metalinkHttpChecker->isMetalinkHttp()) {
        qCDebug(KGET_DEBUG) << "Create MetalinkHTTP";
        return new MetalinkHttp(parent, this, scheduler, srcUrl, destUrl, metalinkHttpChecker, e);
    }
    // No one takes ownership of this checker
    metalinkHttpChecker->deleteLater();

    if (isSupported(srcUrl)) {
        return new MetalinkXml(parent, this, scheduler, srcUrl, destUrl, e);        
    }

    return nullptr;
}

bool MetalinkFactory::isSupported(const QUrl &url) const
{
    return (url.fileName().endsWith(QLatin1String(".metalink")) || url.fileName().endsWith(QLatin1String(".meta4")));
}

#include "metalinkfactory.moc"
