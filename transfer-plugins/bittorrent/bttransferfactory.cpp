/* This file is part of the KDE project

   Copyright (C) 2007 - 2010 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "bttransferfactory.h"

// header inclusion order is crucial because of signal Q_EMIT clashes
#include "advanceddetails/btadvanceddetailswidget.h"
#include "btdatasource.h"
#include "btdetailswidget.h"
#include "bttransfer.h"
#include "bttransferhandler.h"

#include "kget_debug.h"
#include <torrent/job.h>
#include <util/functions.h>
#include <version.h>

K_PLUGIN_CLASS_WITH_JSON(BTTransferFactory, "kget_bittorrentfactory.json")

BTTransferFactory::BTTransferFactory(QObject *parent, const QVariantList &args)
    : TransferFactory(parent, args)
{
    if (!bt::InitLibKTorrent()) {
        qCCritical(KGET_DEBUG) << "Failed to initialize libktorrent";
        KGet::showNotification(nullptr, "error", i18n("Cannot initialize libktorrent. Torrent support might not work."));
    }
}

BTTransferFactory::~BTTransferFactory()
{
}

Transfer *BTTransferFactory::createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup *parent, Scheduler *scheduler, const QDomElement *e)
{
    qCDebug(KGET_DEBUG) << "BTTransferFactory::createTransfer";

    if (isSupported(srcUrl)) {
        return new BTTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return nullptr;
}

TransferHandler *BTTransferFactory::createTransferHandler(Transfer *transfer, Scheduler *scheduler)
{
    auto *bttransfer = qobject_cast<BTTransfer *>(transfer);

    if (!bttransfer) {
        qCCritical(KGET_DEBUG) << "WARNING! passing a non-BTTransfer pointer!!";
        return nullptr;
    }

    return new BTTransferHandler(bttransfer, scheduler);
}

QWidget *BTTransferFactory::createDetailsWidget(TransferHandler *transfer)
{
    auto *bttransfer = static_cast<BTTransferHandler *>(transfer);

    return new BTDetailsWidget(bttransfer);
}

const QList<QAction *> BTTransferFactory::actions(TransferHandler *handler)
{
    auto *bttransfer = static_cast<BTTransferHandler *>(handler);

    QList<QAction *> actions;
    if (bttransfer && bttransfer->torrentControl()) {
        auto *openAdvancedDetailsAction = new QAction(QIcon::fromTheme("document-open"), i18n("&Advanced Details"), this);

        connect(openAdvancedDetailsAction, &QAction::triggered, bttransfer, &BTTransferHandler::createAdvancedDetails);

        actions.append(openAdvancedDetailsAction);

        auto *openScanDlg = new QAction(QIcon::fromTheme("document-open"), i18n("&Scan Files"), this);

        connect(openScanDlg, &QAction::triggered, bttransfer, &BTTransferHandler::createScanDlg);

        actions.append(openScanDlg);
    }

    if (bttransfer)
        return actions;
    else
        return QList<QAction *>();
}

TransferDataSource *BTTransferFactory::createTransferDataSource(const QUrl &srcUrl, const QDomElement &type, QObject *parent)
{
    Q_UNUSED(srcUrl)
    Q_UNUSED(type)
    Q_UNUSED(parent)
    /*if (srcUrl.fileName().endsWith(".torrent"))
        return new BTDataSource();*/
    return nullptr;
}

bool BTTransferFactory::isSupported(const QUrl &url) const
{
    return url.url().endsWith(QLatin1String(".torrent"));
}

#include "bttransferfactory.moc"
