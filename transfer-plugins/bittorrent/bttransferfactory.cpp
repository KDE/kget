/*
 *  Copyright (C) 2005 Felix Berger <felixberger@beldesign.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bttransferfactory.h"

// header inclusion order is crucial because of signal emit clashes
#include "bttransfer.h"
#include "bttransferhandler.h"
#include "btdetailswidget.h"

#include <kdebug.h>

#include <QFile>

KGET_EXPORT_PLUGIN(BTTransferFactory)

Transfer * BTTransferFactory::createTransfer( KUrl srcUrl, KUrl destUrl,
                                              TransferGroup * parent,
                                              Scheduler * scheduler, 
                                              const QDomElement * e )
{
    kDebug(5001) << "BTTransferFactory::createTransfer";

    if (srcUrl.fileName().endsWith(".torrent") && srcUrl.isLocalFile())
    {
        //Make sure that the given url points to an existing torrent file
        QFile torrentFile(srcUrl.path());
        if(!torrentFile.exists())
            return 0;

        return new BTTransfer(parent, this, scheduler, srcUrl, destUrl, e);
    }
    return 0;
}


TransferHandler * BTTransferFactory::createTransferHandler(Transfer * transfer, Scheduler * scheduler)
{
    BTTransfer * bttransfer = dynamic_cast<BTTransfer *>(transfer);

    if(!bttransfer)
    {
        kError(5001) << "BTTransferFactory::createTransferHandler: WARNING!\n"
                      "passing a non-BTTransfer pointer!!" << endl;
        return 0;
    }

    return new BTTransferHandler(bttransfer, scheduler);
}

QWidget * BTTransferFactory::createDetailsWidget( TransferHandler * transfer )
{
    BTTransferHandler * bttransfer = static_cast<BTTransferHandler *>(transfer);
    return new BTDetailsWidget(bttransfer);
}

const QList<KAction *> BTTransferFactory::actions()
{
    return QList<KAction *>();
}
