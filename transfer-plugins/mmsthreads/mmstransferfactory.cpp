/*
    This file is part of the KDE project
    Copyright (C) 2011 Ernesto Rodriguez Ortiz <eortiz@uci.cu>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mmstransferfactory.h"
#include "mmssettings.h"
#include "mmstransfer.h"

#include <KPluginFactory>

#include "kget_debug.h"
#include <QDebug>

K_PLUGIN_CLASS_WITH_JSON(MmsTransferFactory, "kget_mmsfactory.json")

MmsTransferFactory::MmsTransferFactory(QObject *parent, const QVariantList &args)
    : TransferFactory(parent, args)
{
}

MmsTransferFactory::~MmsTransferFactory()
{
}

Transfer *MmsTransferFactory::createTransfer(const QUrl &srcUrl, const QUrl &destUrl, TransferGroup *parent, Scheduler *scheduler, const QDomElement *e)
{
    qCDebug(KGET_DEBUG) << "MmsTransferFactory::createTransfer";

    QString prot = srcUrl.scheme();
    qCDebug(KGET_DEBUG) << "Protocol = " << prot;
    if (prot == "mms" || prot == "mmsh") {
        return new MmsTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return nullptr;
}

QWidget *MmsTransferFactory::createDetailsWidget(TransferHandler *transfer)
{
    Q_UNUSED(transfer)
    return nullptr; // Temporary!!
}

const QList<QAction *> MmsTransferFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler)
    return QList<QAction *>();
}

bool MmsTransferFactory::isSupported(const QUrl &url) const
{
    QString prot = url.scheme();
    qCDebug(KGET_DEBUG) << "Protocol = " << prot;
    return (prot == "mms" || prot == "mmsh");
}

#include "mmstransferfactory.moc"
