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

#include <kdebug.h>

KGET_EXPORT_PLUGIN( MmsTransferFactory )

MmsTransferFactory::MmsTransferFactory(QObject *parent, const QVariantList &args)
  : TransferFactory(parent, args)
{}

MmsTransferFactory::~MmsTransferFactory()
{}

Transfer * MmsTransferFactory::createTransfer( const KUrl &srcUrl, const KUrl &destUrl,
                                               TransferGroup * parent,
                                               Scheduler * scheduler, 
                                               const QDomElement * e )
{
    kDebug(5001) << "MmsTransferFactory::createTransfer";

    QString prot = srcUrl.protocol();
    kDebug(5001) << "Protocol = " << prot;
    if (prot == "mms" || prot == "mmsh") {
        return new MmsTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}

QWidget * MmsTransferFactory::createDetailsWidget( TransferHandler * transfer )
{
    Q_UNUSED(transfer)
    return 0;   //Temporary!!
}

const QList<KAction *> MmsTransferFactory::actions(TransferHandler *handler)
{
    Q_UNUSED(handler)
    return QList<KAction *>();
}

bool MmsTransferFactory::isSupported(const KUrl &url) const
{
    QString prot = url.protocol();
    kDebug(5001) << "Protocol = " << prot;
    return (prot == "mms" || prot == "mmsh");
}
