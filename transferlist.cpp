/***************************************************************************
*                                transferlist.cpp
*                             -------------------
*
*    Revision     : $Id$
*    begin        : Tue Jan 29 2002
*    copyright    : (C) 2002 by Patrick Charbonnier
*                 : Based On Caitoo v.0.7.3 (c) 1998 - 2000, Matej Koss
*    email        : pch@freeshell.org
*
****************************************************************************/

/***************************************************************************
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 ***************************************************************************/

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>

#include "transfer.h"
#include "transferlist.h"

#define NUM_COLS  9

static int defaultColumnWidth[] = {
                                      26,                         // PIXMAP
                                      160,                        // LOCAL FILENAME
                                      40,                         // RESUME
                                      60,                         // COUNT
                                      30,                         // PROGRESS
                                      65,                         // TOTAL
                                      70,                         // SPEED
                                      70,                         // REMAINING TIME
                                      450                         // URL
                                  };


TransferList::TransferList(QWidget * parent, const char *name)
    : KListView(parent, name)
{
    // enable selection of more than one item
    setSelectionMode( QListView::Extended );

    // // disable sorting and clicking on headers
    // setSorting( -1 );

    setAllColumnsShowFocus(true);

    lv_pixmap = addColumn(i18n("S"));
    lv_filename = addColumn(i18n("Local Filename"));
    lv_resume = addColumn(i18n("Resumed"));
    lv_count = addColumn(i18n("Count"));
    lv_progress = addColumn(i18n("%"));
    lv_total = addColumn(i18n("Total"));
    lv_speed = addColumn(i18n("Speed"));
    lv_remaining = addColumn(i18n("Rem. Time"));
    lv_url = addColumn(i18n("Address (URL)"));

    // initial layout
    KConfig *config = KGlobal::config();
    config->setGroup("ListView");
    if ( config->readListEntry("ColumnWidths").isEmpty() )
    {
        for (int i = 0; i < NUM_COLS; i++)
            setColumnWidth(i, defaultColumnWidth[i]);
    }
    else
        restoreLayout( KGlobal::config(), "ListView" );

    QString connectPath = "pics/connect%2.png";
    QString tryPath = "pics/try%2.png";

    // Load animations
    QPixmap* curPix;
    if (animConn.count() == 0) {
        animConn.setAutoDelete(true);
        animTry.setAutoDelete(true);
        for (int i = 0; i < 8; i++) {
            curPix = new QPixmap();
            curPix->load(locate("appdata", connectPath.arg(i)));
            animConn.append(curPix);

            curPix = new QPixmap();
            curPix->load(locate("appdata", tryPath.arg(i)));
            animTry.append(curPix);
        }
    }

    pixQueued = UserIcon("md_queued");
    pixScheduled = UserIcon("md_scheduled");
    pixDelayed = UserIcon("md_delayed.png");
    pixFinished = UserIcon("md_finished");
    pixRetrying = UserIcon("retrying");

    phasesNum = animConn.count();

    connect(this, SIGNAL(doubleClicked(QListViewItem *)), SLOT(slotTransferSelected(QListViewItem *)));
    connect(this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)), SLOT(slotPopupMenu(QListViewItem *)));
}


TransferList::~TransferList()
{
    saveLayout( KGlobal::config(), "ListView" );
}


Transfer *TransferList::addTransfer(const KURL & _source, const KURL & _dest,
                                    bool canShow)
{
    Transfer *last = static_cast<Transfer*>( lastItem() );
    Transfer *new_item = new Transfer(this, last, _source, _dest);
    if ( canShow )
        new_item->maybeShow();

    return new_item;
}


void TransferList::slotTransferSelected(QListViewItem * item)
{
    emit transferSelected((Transfer *) item);
}


void TransferList::slotPopupMenu(QListViewItem * item)
{
    if (!item)
        return;
    emit popupMenu((Transfer *) item);
}


void TransferList::setSelected(QListViewItem * item, bool selected)
{
    bool tmpb = selected;

    if (tmpb && item->isSelected()) {
        tmpb = false;
    }

    QListView::setSelected(item, tmpb);
}


void TransferList::moveToBegin(Transfer * item)
{
    //        ASSERT(item);

    item->synchronousAbort();
    Transfer *new_item = new Transfer(this, item->getSrc(), item->getDest());
    new_item->copy(item);

    delete item;

    clearSelection();
}


void TransferList::moveToEnd(Transfer * item)
{
    //        ASSERT(item);

    item->synchronousAbort();

    Transfer *last = static_cast<Transfer*>( lastItem() );
    Transfer *new_item = new Transfer(this, last, item->getSrc(), item->getDest());
    new_item->copy(item);

    delete item;

    clearSelection();
}


bool TransferList::updateStatus(int counter)
{
    bool isTransfer = false;

    TransferIterator it(this);

    for (; it.current(); ++it) {
        isTransfer |= it.current()->updateStatus(counter);
    }

    return isTransfer;
}


bool TransferList::isQueueEmpty()
{
    TransferIterator it(this);

    if (childCount() <= 0)
        return true;
    else
        for (; it.current(); ++it)
            if (it.current()->getMode() == Transfer::MD_NONE)
                return true;

    return false;
}


Transfer * TransferList::find(const KURL& _src)
{
    TransferIterator it(this);

    for (; it.current(); ++it) {
        if (it.current()->getSrc() == _src) {
            return it.current();
        }
    }

    return 0L;
}


void TransferList::readTransfers(const KURL& file)
{
    QString tmpFile;

    if (KIO::NetAccess::download(file, tmpFile)) {
        KSimpleConfig config(tmpFile);

        config.setGroup("Common");
        int num = config.readNumEntry("Count", 0);

        Transfer *item;
        KURL src, dest;

        for ( int i = 0; i < num; i++ )
        {
            QString str;

            str.sprintf("Item%d", i);
            config.setGroup(str);

            src  = KURL::fromPathOrURL( config.readPathEntry("Source") );
            dest = KURL::fromPathOrURL( config.readPathEntry("Dest") );
            item = addTransfer( src, dest, false ); // don't show!

            if (!item->read(&config, i))
                delete item;
            else
            {
                // configuration read, now we know the status to determine
                // whether to show or not
                item->maybeShow();
            }
        }
    }
}

void TransferList::writeTransfers(const QString& file)
{
    sDebug << ">>>>Entering with file =" << file << endl;

    KSimpleConfig config(file);
    int num = childCount();

    config.setGroup("Common");
    config.writeEntry("Count", num);

    TransferIterator it(this);

    for (int id = 0; it.current(); ++it, ++id)
        it.current()->write(&config, id);
    config.sync();

    sDebug << "<<<<Leaving" << endl;
}

#include "transferlist.moc"
