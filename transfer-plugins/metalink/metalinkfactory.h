/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2012 Aish Raj Dahal <dahalaishraj@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef METALINK_FACTORY_H
#define METALINK_FACTORY_H

#include "core/plugin/transferfactory.h"
#include "ui/metalinkcreator/metalinker.h"

class Transfer;
class TransferGroup;
class Scheduler;

class MetalinkFactory : public TransferFactory
{
    Q_OBJECT
public:
    MetalinkFactory(QObject *parent, const QVariantList &args);
    ~MetalinkFactory() override;

    Transfer *createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup *parent, Scheduler *scheduler, const QDomElement *e = nullptr) override;

    QString displayName() const override
    {
        return "MetaLink";
    }
    /**
     * Checks if a URL is supported by this plugin.
     *
     * @param url The URL to be tested
     * @return True if the URL is a metalink (xml or http).
     */
    bool isSupported(const QUrl &url) const override;
};

#endif
