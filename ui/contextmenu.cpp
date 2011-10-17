/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "contextmenu.h"

#ifdef HAVE_NEPOMUK
    #include <kfilemetadatawidget.h>
    #include "core/nepomukhandler.h"
#endif

#include "core/kget.h"
#include "core/plugin/transferfactory.h"
#include "core/transferhandler.h"
#include "core/transfergrouphandler.h"
#include <KMenu>
#include <QWidgetAction>

#ifdef HAVE_KONQUEROR
    #include <KFileItem>
    #include <kfileitemlistproperties.h>
    #include <konq_menuactions.h>
#endif

KMenu * ContextMenu::createTransferContextMenu(QList<TransferHandler*> transfers, QWidget *parent)
{
    if (transfers.empty())
        return 0;

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

    KMenu *popup = new KMenu(parent);
    popup->addTitle(transfers.first()->dest().fileName());
    //Get the transfer factory actions
    QList<QAction*> actionList = transfers.first()->factoryActions();
//     popup->addTitle( i18np("%1 Download selected", "%1 Downloads selected", transfers.count()) );

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

#ifdef HAVE_NEPOMUK
    QList<KFileItem> items;
    foreach (TransferHandler *transfer, KGet::selectedTransfers()) {
        items << transfer->nepomukHandler()->fileItems();
    }

    KMenu *nepomukMenu = new KMenu(i18n("Semantic Desktop"), parent);
    nepomukMenu->setIcon(KIcon("nepomuk"));
    QWidgetAction *nepomukWidgetAction = new QWidgetAction(parent);
    KFileMetaDataWidget *nepomukWidget = new KFileMetaDataWidget(parent);
    nepomukWidget->setItems(items);
    nepomukWidgetAction->setDefaultWidget(nepomukWidget);
    nepomukMenu->addAction(nepomukWidgetAction);
    popup->addMenu(nepomukMenu);
#endif

    popup->addAction( KGet::actionCollection()->action("transfer_open_dest") );
    //popup->addAction( KGet::actionCollection()->action("transfer_open_file") );
    popup->addAction( KGet::actionCollection()->action("transfer_show_details") );
    popup->addAction( KGet::actionCollection()->action("transfer_copy_source_url") );

    return popup;
}

KMenu * ContextMenu::createTransferContextMenu(TransferHandler* handler, QWidget *parent)
{
    KMenu *popup = ContextMenu::createTransferContextMenu(QList<TransferHandler*>() << handler, parent);

#ifdef HAVE_KONQUEROR
    // only shows the open with actions if the transfer is finished
    if (handler->status() == Job::Finished || handler->status() == Job::FinishedKeepAlive) {
        KFileItemList items;
        items << KFileItem(KFileItem::Unknown, KFileItem::Unknown, handler->dest());

        KonqMenuActions menuActions;

        menuActions.setItemListProperties(KFileItemListProperties(items));
        menuActions.setParentWidget(parent);

        menuActions.addActionsTo(popup);
        menuActions.addOpenWithActionsTo(popup, "DesktopEntryName != 'kget'");

        // TODO : seems like the popup menu has to be showed while the KonqMenuActions instance exists ?
        popup->exec(QCursor::pos());
        popup->deleteLater();

        return 0;
    }
#endif

    return popup;
}

KMenu * ContextMenu::createTransferGroupContextMenu(TransferGroupHandler *handler, QWidget *parent)
{
    if (!handler)
        return 0;

    KMenu * popup = new KMenu(parent);
    popup->addTitle(handler->name());

    popup->addActions(handler->actions());
    popup->addSeparator();
    popup->addAction(KGet::actionCollection()->action("transfer_group_settings"));
    popup->addSeparator();

    bool containsMainGroup;
    QList<TransferGroupHandler *> transferGroups = KGet::selectedTransferGroups(&containsMainGroup);
    if(!containsMainGroup) {
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
