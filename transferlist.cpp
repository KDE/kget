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
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>

#include <qfile.h>

#include "kmainwidget.h"
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


// TransferList static members
QPtrList < QPixmap > *TransferList::animConn = 0L;
QPtrList < QPixmap > *TransferList::animTry = 0L;
QPixmap *TransferList::pixQueued = 0L;
QPixmap *TransferList::pixScheduled = 0L;
QPixmap *TransferList::pixDelayed = 0L;
QPixmap *TransferList::pixFinished = 0L;
QPixmap *TransferList::pixRetrying = 0L;


TransferList::TransferList(QWidget * parent, const char *name):KListView(parent, name)
{

    if (pixQueued == 0L) {
        initStatic();
    }
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

    readConfig();

    QString connectPath = "pics/connect%2.png";
    QString tryPath = "pics/try%2.png";

    // Load animations
    QPixmap* curPix;
    if (animConn->count() == 0) {
        animConn->setAutoDelete(true);
        animTry->setAutoDelete(true);
        for (int i = 0; i < 8; i++) {
            curPix = new QPixmap();
            curPix->load(locate("appdata", connectPath.arg(i)));
            animConn->append(curPix);

            curPix = new QPixmap();
            curPix->load(locate("appdata", tryPath.arg(i)));
            animTry->append(curPix);
        }
    }

    pixQueued = new QPixmap();
    pixQueued->load(locate("appdata", "pics/md_queued.png"));

    pixScheduled = new QPixmap();
    pixScheduled->load(locate("appdata", "pics/md_scheduled.png"));

    pixDelayed = new QPixmap();
    pixDelayed->load(locate("appdata", "pics/md_delayed.png"));

    pixFinished = new QPixmap();
    pixFinished->load(locate("appdata", "pics/md_finished.png"));

    pixRetrying = new QPixmap();
    pixRetrying->load(locate("appdata", "pics/retrying.png"));

    phasesNum = animConn->count();

    connect(this, SIGNAL(doubleClicked(QListViewItem *)), SLOT(slotTransferSelected(QListViewItem *)));
    connect(this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)), SLOT(slotPopupMenu(QListViewItem *)));
}


TransferList::~TransferList()
{
    writeConfig();

    delete animConn;
    delete animTry ;
    delete pixQueued ;
    delete pixScheduled;
    delete pixDelayed;
    delete pixFinished;
    delete pixRetrying;

}


void
TransferList::initStatic()
{
    animConn = new QPtrList < QPixmap >;
    animTry = new QPtrList < QPixmap >;
    pixQueued = new QPixmap;
    pixScheduled = new QPixmap;
    pixDelayed = new QPixmap;
    pixFinished = new QPixmap;
    pixRetrying = new QPixmap;
}


Transfer *TransferList::addTransfer(const KURL & _source, const KURL & _dest)
{
    TransferIterator it(this);

    for (; it.current(); ++it) {
        if (it.current()->itemBelow() == 0L) {  // this will find the end of list
            break;
        }
    }

    Transfer *new_item = new Transfer(this, it.current(), _source, _dest);

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


void TransferList::readConfig()
{

    KConfig *config = kapp->config();

    // read listview geometry properties
    config->setGroup("ListView");
    for (int i = 0; i < NUM_COLS; i++) {
        QString tmps;

        tmps.sprintf("Col%d", i);
        setColumnWidth(i, config->readNumEntry(tmps, defaultColumnWidth[i]));
    }
}


void TransferList::writeConfig()
{

    KConfig *config = kapp->config();

    // write listview geometry properties
    config->setGroup("ListView");
    for (int i = 0; i < NUM_COLS; i++) {
        QString tmps;

        tmps.sprintf("Col%d", i);
        config->writeEntry(tmps, columnWidth(i));
    }

    config->sync();
}


void TransferList::moveToBegin(Transfer * item)
{
    TransferIterator it(this);

    //        ASSERT(item);

    Transfer *new_item = new Transfer(this, item->getSrc(), item->getDest());

    new_item->copy(item);

    delete item;

    clearSelection();
    new_item->updateAll();
}


void TransferList::moveToEnd(Transfer * item)
{

    //        ASSERT(item);

    Transfer *new_item = addTransfer(item->getSrc(), item->getDest());

    new_item->copy(item);

    delete item;

    clearSelection();
    new_item->updateAll();
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


bool TransferList::find(KURL _src)
{
    TransferIterator it(this);

    for (; it.current(); ++it) {
        if (it.current()->getSrc() == _src) {
            return true;
        }
    }

    return false;
}


void TransferList::readTransfers(QString file)
{

    QString tmpFile;

    if (KIO::NetAccess::download(file, tmpFile)) {
        KSimpleConfig config(tmpFile);

        config.setGroup("Common");
        int num = config.readNumEntry("Count", 0);

        Transfer *item;

        while (num--) {
            QString str;

            str.sprintf("Item%d", num);
            config.setGroup(str);

            item = addTransfer(config.readEntry("Source", ""), config.readEntry("Dest", ""));

            if (!item->read(&config, num)) {
                delete item;
            }
        }
    }

}

void TransferList::writeTransfers(QString file)
{
    sDebug << ">>>>Entering with file =" << file << endl;

    QFile f(file);


    if (!f.open(IO_WriteOnly)) {
        // TODO ADD Message LOG
        return;
    }

    KSimpleConfig config(file);
    int num = childCount();

    config.setGroup("Common");
    config.writeEntry("Count", num);

    TransferIterator it(this);

    for (int id = 0; it.current(); ++it, ++id)
        it.current()->write(&config, id);
    config.sync();

    f.close();




    sDebug << "<<<<Leaving" << endl;
}

#include "transferlist.moc"
