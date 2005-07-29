/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kpopupmenu.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "model.h"
#include "transferfactory.h"
#include "transferaction.h"

TransferFactory::TransferFactory()
{
    //ActionStart action
    m_actions.append( new ActionStart( i18n("Start"), "tool_resume",
                                       0, Model::actionCollection(),
                                       "transfer_start" ) );

    //ActionStop action
    m_actions.append( new ActionStop( i18n("Stop"), "tool_pause",
                                      0, Model::actionCollection(),
                                      "transfer_stop" ) );

    //ActionDelete action
    m_actions.append( new ActionDelete( i18n("Delete"), "editdelete",
                                        0, Model::actionCollection(),
                                        "transfer_remove" ) );

    //ActionOpenDestination action
    m_actions.append( new ActionOpenDestination( i18n("Open Destination"), "folder",
                                                 0, Model::actionCollection(),
                                                 "transfer_open_destination" ) );
}

KPopupMenu * TransferFactory::createPopupMenu(QValueList<TransferHandler *> transfers)
{
    if( transfers.empty() )
        return 0;

    //First check whether all the transfers in the list belong to the same 
    //transferfactory
    bool sameFactory = true;

    QValueList<TransferHandler *>::iterator it = transfers.begin();
    QValueList<TransferHandler *>::iterator itEnd = transfers.end();

    for(; (it!=itEnd) && (sameFactory) ; ++it)
    {
        sameFactory = ( (*it)->m_transfer->factory() ==
                        transfers.first()->m_transfer->factory() );
    }

    //Get the right factory for the given list of transfers
    QValueList<TransferAction *> actionList;
    if(sameFactory)
        actionList = transfers.first()->m_transfer->factory()->actions();
    else
        actionList = this->actions();

    KPopupMenu * popup = new KPopupMenu( 0 );
    popup->insertTitle( i18n("%n download", "%n downloads", transfers.count()) );

    //Plug all the actions in the popup menu
    QValueList<TransferAction *>::iterator it2 = actionList.begin();
    QValueList<TransferAction *>::iterator itEnd2 = actionList.end();

    for( ; it2!=itEnd2 ; ++it2 )
    {
        //Plug each action in the popup menu
        (*it2)->plug( popup );
    }

    return popup;
}

