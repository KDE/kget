/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/model.h"
#include "core/transferaction.h"

TransferAction::TransferAction( const QString& text, const QString& pix, 
                                const KShortcut& cut, KActionCollection* parent, 
                                const char* name )
    : KAction(text, pix, cut, this, SLOT( activate() ), parent, name)
{
    
}

void TransferAction::activate()
{
    execute( Model::selectedTransfers() );
}
