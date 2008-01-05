/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transferfactory.h"

#include "kget.h"

#include <kmenu.h>
#include <klocale.h>
#include <kdebug.h>

TransferFactory::TransferFactory(QObject *parent, const QVariantList &args)
  : KGetPlugin(parent, args)
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

    if (transfers.size() == 1)
        actionList = transfers.first()->m_transfer->factory()->actions(transfers.first());
    else if(sameFactory)
        actionList = transfers.first()->m_transfer->factory()->actions();
    else
        actionList = this->actions();

    KMenu * popup = new KMenu( 0 );
//     popup->addTitle( i18np("1 Download selected", "%1 Downloads selected", transfers.count()) );

    //Plug all the actions in the popup menu
    popup->addAction( KGet::actionCollection()->action("start_selected_download") );
    popup->addAction( KGet::actionCollection()->action("stop_selected_download") );
    popup->addAction( KGet::actionCollection()->action("delete_selected_download") );
    popup->addSeparator();

    foreach(KAction * it, actionList)
    {
        //Plug each action in the popup menu
        popup->addAction( it );
    }

    if(!actionList.isEmpty())
        popup->addSeparator();

    popup->addAction( KGet::actionCollection()->action("transfer_open_dest") );
    popup->addAction( KGet::actionCollection()->action("transfer_open_file") );
    popup->addAction( KGet::actionCollection()->action("transfer_show_details") );
    popup->addAction( KGet::actionCollection()->action("transfer_copy_source_url") );

    return popup;
}

