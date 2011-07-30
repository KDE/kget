/* This file is part of the KDE project

   Copyright (C) 2009 by Fabian Henze <flyser42 AT gmx DOT de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "ui/tray.h"
#include "mainwindow.h"
#include "ui/newtransferdialog.h"
#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kdebug.h>

#include <QClipboard>

/** class Tray
  * Reimplementation of the KStatusNotifierItem class
  */
Tray::Tray(MainWindow * parent)
    : KStatusNotifierItem(parent)
{
    // set up the context menu
    KMenu * cm = contextMenu();
    cm->addAction( parent->actionCollection()->action("new_download") );
    cm->addAction( parent->actionCollection()->action("import_links") );
    cm->addSeparator();
    cm->addAction( parent->actionCollection()->action("start_all_download") );
    cm->addAction( parent->actionCollection()->action("stop_all_download") );
    cm->addSeparator();
    cm->addAction( parent->actionCollection()->action("konqueror_integration") );
    cm->addAction( parent->actionCollection()->action("options_configure") );

    // Set up basic tray parameters
    setCategory(ApplicationStatus);
    setIconByName("kget");
    setTitle(i18n("KGet"));
    setContextMenu(cm);
    setAssociatedWidget(parent);
    setToolTipIconByName("kget");
    setToolTipTitle(i18n("Download Manager"));
    // Not of much use atm, but maybe we want to set this later?
    // setToolTipSubTitle("[..]");

    // filter middle mouse clicks to ask scheduler to paste URL
    connect( this, SIGNAL(secondaryActivateRequested(QPoint)),
             this, SLOT(slotActivated()) );
}


// filter middle mouse clicks to ask scheduler to paste URL
void Tray::slotActivated()
{
    // Here we paste the transfer
    QString newtransfer = QApplication::clipboard()->text();
    newtransfer = newtransfer.trimmed();

    if(!newtransfer.isEmpty())
        NewTransferDialogHandler::showNewTransferDialog(KUrl(newtransfer));
}

// display a play icon when downloading and
// switch between Active or Passive state
void Tray::setDownloading( bool downloading )
{
    kDebug(5001) << "Tray::setDownloading";

    if (downloading)
    {
        if (status() == KStatusNotifierItem::Active)
            return;
        setStatus(KStatusNotifierItem::Active);
        setOverlayIconByName("media-playback-start");
    }
    else
    {
        if (status() == KStatusNotifierItem::Passive)
            return;
        setStatus(KStatusNotifierItem::Passive);
        setOverlayIconByName(QString());
    } 	
}

bool Tray::isDownloading()
{
    // KStatusNotifierItem::NeedsAttention is not handled here,
    // as we do not use it.
    return (status() == KStatusNotifierItem::Active);
}

#include "tray.moc"
