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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <kdebug.h>

#include <qfile.h>

// header inclusion order is crucial because of signal emit clashes
#include "bttransfer.h"
#include "bttransferfactory.h"

KGET_EXPORT_PLUGIN(BTTransferFactory)

Transfer * BTTransferFactory::createTransfer( KURL srcURL, KURL destURL,
                                              TransferGroup * parent,
                                              Scheduler * scheduler, 
                                              const QDomElement * e )
{
    kdDebug() << "BTTransferFactory::createTransfer" << endl;

    if (srcURL.fileName().endsWith(".torrent") && srcURL.isLocalFile())
    {
        //Make sure that the given url points to an existing torrent file
        QFile torrentFile(srcURL.path());
        if(!torrentFile.exists())
            return 0;

        return new BTTransfer(parent, this, scheduler, srcURL, destURL, e);
    }
    return 0;
}
