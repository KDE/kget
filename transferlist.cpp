/***************************************************************************
*                                transferlist.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/

#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>

#include "transfer.h"
#include "transferlist.h"


TransferList::TransferList()
    : QValueList<Transfer *>()
{

}


TransferList::~TransferList()
{

}


Transfer *TransferList::addTransfer(const KURL & _source, const KURL & _dest)
{
/*    Transfer *last = static_cast<Transfer*>( lastItem() );
    Transfer *new_item = new Transfer(this, last, _source, _dest, jobid);
    jobid++;
    if ( canShow )
        new_item->maybeShow();

    return new_item;
*/
}

/*
void TransferList::moveToBegin(Transfer * item)
{
    //        ASSERT(item);

    Transfer *oldfirst=static_cast<Transfer*>(firstChild());
    item->moveItem(oldfirst); //move item after oldfirst
    oldfirst->moveItem(item); //move oldfirst after item
}


void TransferList::moveToEnd(Transfer * item)
{
    //        ASSERT(item);

    Transfer *oldlast=static_cast<Transfer*>(lastItem());
    item->moveItem(oldlast);
}
*/


Transfer * TransferList::find(const KURL& _src)
{
/*    TransferIterator it(this);

    for (; it.current(); ++it) {
        if (it.current()->getSrc() == _src) {
            return it.current();
        }
    }

    return 0L;
*/
}


void TransferList::readTransfers(const KURL& file)
{
    QString tmpFile;

    if (KIO::NetAccess::download(file, tmpFile)) {
        KSimpleConfig config(tmpFile);

        config.setGroup("Common");
        int num = config.readNumEntry("Count", 0);

        Transfer *item;
        KURL src, dest;

        for ( int i = 0; i < num; i++ )
        {
            QString str;

            str.sprintf("Item%d", i);
            config.setGroup(str);

            src  = KURL::fromPathOrURL( config.readPathEntry("Source") );
            dest = KURL::fromPathOrURL( config.readPathEntry("Dest") );
            item = addTransfer( src, dest); // don't show!

            if (!item->read(&config, i))
                delete item;
            else
            {
                // configuration read, now we know the status to determine
                // whether to show or not
                //item->maybeShow();
            }
        }
    }
}

void TransferList::writeTransfers(const QString& file)
{
    sDebug << ">>>>Entering with file =" << file << endl;

    KSimpleConfig config(file);
    int num = size();

    config.setGroup("Common");
    config.writeEntry("Count", num);

    iterator it = begin();

    for (int id = 0; *it; ++it, ++id)
        (*it)->write(&config, id);
    config.sync();

    sDebug << "<<<<Leaving" << endl;
}
