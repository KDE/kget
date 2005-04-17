/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/transferaction.h"

TransferAction::TransferAction( const QString& text, const QIconSet& pix, 
                                const KShortcut& cut, KActionCollection* parent, 
                                const char* name )
    : KAction(text, pix, cut, this, SLOT( activate() ), parent, name)
{
    
}

void TransferAction::connectToTransfer(Transfer * transfer)
{
    if(!m_transfers.contains(transfer))
    {
        //Now we are sure that the list doesn't already contain this transfer
        m_transfers.append(transfer);
    }
}

void TransferAction::disconnectAllTransfers()
{
    m_transfers.clear();
}

void TransferAction::activate()
{
    QValueList<Transfer *>::iterator it = m_transfers.begin();
    QValueList<Transfer *>::iterator itEnd = m_transfers.end();

    for( ; it!=itEnd ; ++it )
    {
        execute(*it);
    }
}
