/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btfiletreeview.h"

#include <util/bitset.h>
#include <util/functions.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>

#include <kdebug.h>
#include <kmenu.h>
#include <klocale.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <krun.h>

using namespace bt;

BTFileTreeView::BTFileTreeView(bt::TorrentInterface *tc, QWidget * parent)
  : QTreeView(parent),
    m_tc(tc)
{
    fileTreeModel = new kt::IWFileTreeModel(tc, this);
    setModel(fileTreeModel);

    if (!m_tc->getStats().multi_file_torrent)
        setRootIsDecorated(false);

    contextMenu = new KMenu(this);
    open_action = contextMenu->addAction(KIcon("document-open"),i18n("Open"),this,SLOT(open()));
    contextMenu->addSeparator();
    download_first_action = contextMenu->addAction(i18n("Download first"),this,SLOT(downloadFirst()));
    download_normal_action = contextMenu->addAction(i18n("Download normally"),this,SLOT(downloadNormal()));
    download_last_action = contextMenu->addAction(i18n("Download last"),this,SLOT(downloadLast()));
    contextMenu->addSeparator();
    dnd_action = contextMenu->addAction(i18n("Do Not Download"),this,SLOT(doNotDownload()));
    delete_action = contextMenu->addAction(i18n("Delete File(s)"),this,SLOT(deleteFiles()));

    //connect(this, SIGNAL(customContextMenuRequested(const QPoint & )), this, SLOT(showContextMenu(const QPoint& )));
}

void BTFileTreeView::contextMenuEvent(QContextMenuEvent * e)
{
    kDebug(5001);
    const TorrentStats & s = m_tc->getStats();

    QModelIndexList sel = selectionModel()->selectedRows();
    if (sel.count() == 0)
        return;

    if (sel.count() > 1)
    {
        download_first_action->setEnabled(true);
        download_normal_action->setEnabled(true);
        download_last_action->setEnabled(true);
        open_action->setEnabled(false);
        dnd_action->setEnabled(true);
        delete_action->setEnabled(true);
        contextMenu->popup(QCursor::pos());
        return;
    }

    QModelIndex item = sel.front();
    bt::TorrentFileInterface* file = fileTreeModel->indexToFile(item);

    download_first_action->setEnabled(false);
    download_last_action->setEnabled(false);
    download_normal_action->setEnabled(false);
    dnd_action->setEnabled(false);
    delete_action->setEnabled(false);

    if (!s.multi_file_torrent)
    {
        open_action->setEnabled(true);
        preview_path = m_tc->getStats().output_path;
    }
    else if (file)
    {
        if (!file->isNull())
        {
            open_action->setEnabled(true);
            preview_path = file->getPathOnDisk();

            download_first_action->setEnabled(file->getPriority() != FIRST_PRIORITY);
            download_normal_action->setEnabled(file->getPriority() != NORMAL_PRIORITY);
            download_last_action->setEnabled(file->getPriority() != LAST_PRIORITY);
            dnd_action->setEnabled(file->getPriority() != ONLY_SEED_PRIORITY);
            delete_action->setEnabled(file->getPriority() != EXCLUDED);
        }
        else
        {
            open_action->setEnabled(false);
        }
    }
    else
    {
        download_first_action->setEnabled(true);
        download_normal_action->setEnabled(true);
        download_last_action->setEnabled(true);
        dnd_action->setEnabled(true);
        delete_action->setEnabled(true);
        open_action->setEnabled(true);
        preview_path = m_tc->getDataDir() + fileTreeModel->dirPath(item);
    }

    contextMenu->popup(QCursor::pos());
}

void BTFileTreeView::open()
{
    new KRun(KUrl(preview_path), 0, false, true);
}

void BTFileTreeView::changePriority(bt::Priority newpriority)
{
    fileTreeModel->changePriority(selectionModel()->selectedRows(2),newpriority);
}

void BTFileTreeView::downloadFirst()
{
    changePriority(FIRST_PRIORITY);
}

void BTFileTreeView::downloadLast()
{
    changePriority(LAST_PRIORITY);
}

void BTFileTreeView::downloadNormal()
{
    changePriority(NORMAL_PRIORITY);
}

void BTFileTreeView::doNotDownload()
{
    changePriority(ONLY_SEED_PRIORITY);
}

void BTFileTreeView::deleteFiles()
{
    QModelIndexList sel = selectionModel()->selectedRows();
    Uint32 n = sel.count();
    if (n == 1) // single item can be a directory
    {
        if (!fileTreeModel->indexToFile(sel.front()))
            n++;
    }

    QString msg = n > 1 ? i18n("You will lose all data in this file, are you sure you want to do this?") :
                          i18n("You will lose all data in these files, are you sure you want to do this?");

    if (KMessageBox::warningYesNo(0, msg) == KMessageBox::Yes)
            changePriority(EXCLUDED);
}
