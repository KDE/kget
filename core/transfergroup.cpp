/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kurl.h>
#include <kdebug.h>

#include "core/transfer.h"
#include "core/transfergroup.h"

TransferGroup::TransferGroup(const QString & name)
    : m_name(name)
{

}

Transfer * TransferGroup::findTransfer(KURL src)
{
    QValueList<Transfer *>::iterator it = begin();
    QValueList<Transfer *>::iterator itEnd = end();

    for(; it!=itEnd ; ++it)
    {
        if( (*it)->source().url() == src.url() )
            return *it;
    }
    return 0;
}
