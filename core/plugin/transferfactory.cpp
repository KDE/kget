/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kmenu.h>
#include <klocale.h>

#include "model.h"
#include "transferfactory.h"

TransferFactory::TransferFactory()
{

}

KMenu * TransferFactory::createPopupMenu(QList<TransferHandler *> transfers)
{
    if( transfers.empty() )
        return 0;

    //First check whether all the transfers in the list belong to the same 
    //transferfactory
    bool sameFactory = true;

    QList<TransferHandler *>::iterator it = transfers.begin();
    QList<TransferHandler *>::iterator itEnd = transfers.end();

    for(; (it!=itEnd) && (sameFactory) ; ++it)
    {
        sameFactory = ( (*it)->m_transfer->factory() ==
                        transfers.first()->m_transfer->factory() );
    }

    //Get the right factory for the given list of transfers
    QList<KAction *> actionList;

    if(sameFactory)
        actionList = transfers.first()->m_transfer->factory()->actions();
    else
        actionList = this->actions();

    KMenu * popup = new KMenu( 0 );
    popup->addTitle( i18n("%n download", "%n downloads", transfers.count()) );

    //Plug all the actions in the popup menu
    Model::actionCollection()->action("transfer_start")->plug( popup );
    Model::actionCollection()->action("transfer_stop")->plug( popup );
    popup->addSeparator();
    Model::actionCollection()->action("transfer_remove")->plug( popup );
    popup->addSeparator();

    foreach(KAction * it, actionList)
    {
        //Plug each action in the popup menu
        it->plug( popup );
    }

    if(!actionList.isEmpty())
        popup->addSeparator();

    Model::actionCollection()->action("transfer_open_dest")->plug( popup );
    Model::actionCollection()->action("transfer_show_details")->plug( popup );
    Model::actionCollection()->action("transfer_copy_source_url")->plug( popup );

    return popup;
}

