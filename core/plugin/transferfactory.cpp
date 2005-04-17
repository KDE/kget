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

#include "transferfactory.h"
#include "transferaction.h"

TransferFactory::TransferFactory()
{
    //ActionStart action
    m_actions.append( new ActionStart( i18n("Start"), SmallIcon("down"),
                                       0, 0, "start_transfer" ) );

    //ActionStop action
    m_actions.append( new ActionStop( i18n("Stop"), SmallIcon("stop"),
                                      0, 0, "stop_transfer" ) );
}

KPopupMenu * TransferFactory::createPopupMenu(QValueList<Transfer *> transfers)
{
    if( transfers.empty() )
        return 0;

    //First check whether all the transfers in the list belong to the same 
    //transferfactory
    bool sameFactory = true;

    QValueList<Transfer *>::iterator it = transfers.begin();
    QValueList<Transfer *>::iterator itEnd = transfers.end();

    for(; (it!=itEnd) && (sameFactory) ; ++it)
    {
        sameFactory = ( (*it)->factory() == transfers.first()->factory() );
    }

    //Get the right factory for the given list of transfers
    QValueList<TransferAction *> actionList;
    if(sameFactory)
        actionList = transfers.first()->factory()->actions();
    else
        actionList = this->actions();

    KPopupMenu * popup = new KPopupMenu( 0 );
    popup->insertTitle( i18n("%n download", "%n downloads", transfers.count()) );

    //Plug all the actions in the popup menu
    QValueList<TransferAction *>::iterator it2 = actionList.begin();
    QValueList<TransferAction *>::iterator itEnd2 = actionList.end();

    for( ; it2!=itEnd2 ; ++it2 )
    {
        //Now connect each action to the given list of transfers
        (*it2)->disconnectAllTransfers();

        it = transfers.begin();
        itEnd = transfers.end();

        for( ; it!=itEnd ; ++it )
        {
            (*it2)->connectToTransfer( *it );
        }

        //Plug each action in the popup menu
        (*it2)->plug( popup );
    }

    return popup;

//     KPopupMenu * popup = new KPopupMenu( this );
// 
//     TransferList t = getSelectedList();
//     // insert title
//     QString t1 = i18n("%n download", "%n downloads", t.count());
//     QString t2 = i18n("KGet");
//     popup->insertTitle( column!=-1 ? t1 : t2 );
// 
//     // add menu entries
//     if ( column!=-1 )
//     {   // menu over an item
//         popup->insertItem( SmallIcon("down"), i18n("R&esume"), this, SLOT(slotResumeItems()) );
//         popup->insertItem( SmallIcon("stop"), i18n("&Stop"), this, SLOT(slotStopItems()) );
//         popup->insertItem( SmallIcon("editdelete"), i18n("&Remove"), this, SLOT(slotRemoveItems()) );
// 
//         KPopupMenu * subPrio = new KPopupMenu( popup );
//         subPrio->insertItem( SmallIcon("2uparrow"), i18n("highest"), this,  SLOT( slotSetPriority(int) ), 0, 1);
//         subPrio->insertItem( SmallIcon("1uparrow"), i18n("high"), this,  SLOT( slotSetPriority(int) ), 0, 2);
//         subPrio->insertItem( SmallIcon("1rightarrow"), i18n("normal"), this,  SLOT( slotSetPriority(int) ), 0, 3);
//         subPrio->insertItem( SmallIcon("1downarrow"), i18n("low"), this,  SLOT( slotSetPriority(int) ), 0, 4);
//         subPrio->insertItem( SmallIcon("2downarrow"), i18n("lowest"), this,  SLOT( slotSetPriority(int) ), 0, 5);
//         subPrio->insertItem( SmallIcon("stop"), i18n("do now download"), this,  SLOT( slotSetPriority(int) ), 0, 6 );
//         popup->insertItem( i18n("Set &priority"), subPrio );
// 
//         KPopupMenu * subGroup = new KPopupMenu( popup );
//         //for loop inserting all existant groups
//         QMap<QString, TransferGroupItem *>::iterator it = m_groupsMap.begin();
//         QMap<QString, TransferGroupItem *>::iterator itEnd = m_groupsMap.end();
// 
//         for(int i=0; it != itEnd; ++it, ++i)
//         {
//             subGroup->insertItem( SmallIcon("folder"),
//                                     (*it)->group()->info().name, 
//                                     this,  SLOT( slotSetGroup( int ) ),
//                                     0, i);
//         }
//         //subGroup->insertItem( i18n("new ..."), this,  SLOT( slotSetGroup() ) );
//         popup->insertItem( SmallIcon("folder"), i18n("Set &group"), subGroup );
//     }
//     else
//         // menu on empty space
//         ac->action("open_transfer")->plug(popup);
// 
//     // show popup
//     popup->popup( pos );
}

