/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "contextmenu.h"

#include "core/kget.h"
#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include <QMenu>
#include <QWidgetAction>

#include <KFileItem>
#include <KFileItemListProperties>
#include <KFileItemActions>

QMenu * ContextMenu::createTransferContextMenu(QList<TransferHandler*> transfers, QWidget *parent)
{
    if (transfers.empty())
        return nullptr;

    //First check whether all the transfers in the list belong to the same 
    //transferfactory
    //bool sameFactory = true;

    /*QList<TransferHandler *>::iterator it = transfers.begin();
    QList<TransferHandler *>::iterator itEnd = transfers.end();

    for(; (it!=itEnd) && (sameFactory) ; ++it)
    {
        //sameFactory = ( (*it)->m_transfer->factory() == //Port factory() to transferhandler
         //               transfers.first()->m_transfer->factory() );
    }*/

    auto *popup = new QMenu(parent);
    popup->addSection(transfers.first()->dest().fileName());
    //Get the transfer factory actions
    QList<QAction*> actionList = transfers.first()->factoryActions();
//     popup->addSection( i18np("%1 Download selected", "%1 Downloads selected", transfers.count()) );

    //Plug all the actions in the popup menu
    popup->addActions(transfers.first()->contextActions());
    popup->addSeparator();
    popup->addAction( KGet::actionCollection()->action("transfer_settings") );
    popup->addSeparator();

    foreach (QAction * it, actionList)
    {
        //Plug each action in the popup menu
        popup->addAction( it );
    }

    if (!actionList.isEmpty())
        popup->addSeparator();

    popup->addAction( KGet::actionCollection()->action("transfer_open_dest") );
    //popup->addAction( KGet::actionCollection()->action("transfer_open_file") );
    popup->addAction( KGet::actionCollection()->action("transfer_show_details") );
    popup->addAction( KGet::actionCollection()->action("transfer_copy_source_url") );

    return popup;
}

QMenu * ContextMenu::createTransferContextMenu(TransferHandler* handler, QWidget *parent)
{
    QMenu *popup = ContextMenu::createTransferContextMenu(QList<TransferHandler*>() << handler, parent);

    // only shows the open with actions if the transfer is finished
    if (handler->status() == Job::Finished || handler->status() == Job::FinishedKeepAlive) {
        KFileItemList items;
        items << KFileItem(handler->dest());

        KFileItemActions menuActions;

        menuActions.setItemListProperties(KFileItemListProperties(items));
        menuActions.setParentWidget(parent);

        menuActions.addServiceActionsTo(popup);
        menuActions.addOpenWithActionsTo(popup, "DesktopEntryName != 'kget'");

        // TODO : seems like the popup menu has to be showed while the KonqMenuActions instance exists ?
        popup->exec(QCursor::pos());
        popup->deleteLater();

        return nullptr;
    }

    return popup;
}

QMenu * ContextMenu::createTransferGroupContextMenu(TransferGroupHandler *handler, QWidget *parent)
{
    if (!handler)
        return nullptr;

    auto * popup = new QMenu(parent);
    popup->addSection(handler->name());

    popup->addActions(handler->actions());
    popup->addSeparator();
    popup->addAction(KGet::actionCollection()->action("transfer_group_settings"));
    popup->addSeparator();

    QList<TransferGroupHandler *> transferGroups = KGet::selectedTransferGroups();
    if (transferGroups.count() != KGet::allTransferGroups().count()) {
        const int numGroups = transferGroups.count();
        QAction *action = KGet::actionCollection()->action("delete_groups");
        action->setText(i18np("Delete Group", "Delete Groups", numGroups));
        popup->addAction(action);

        action = KGet::actionCollection()->action("rename_groups");
        action->setText(i18np("Rename Group...", "Rename Groups...", numGroups));
        popup->addAction(action);
    }
    popup->addAction( KGet::actionCollection()->action("seticon_groups") );
    return popup;
}
