/***************************************************************************
*                                transferlist.h
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

#ifndef _TransferList_h
#define _TransferList_h

#include <klistview.h>
#include <qptrlist.h>

#include <kurl.h>

class Transfer;

class TransferIterator:public QListViewItemIterator
{

public:

    TransferIterator(QListView * view):QListViewItemIterator(view)
    {
    }
    Transfer *current() const
    {
        return (Transfer *) QListViewItemIterator::current();
    }
    void reset()
    {
        curr = listView->firstChild();
    }

};


class TransferList:public KListView
{
Q_OBJECT public:


    TransferList(QWidget * parent = 0, const char *name = 0);
    virtual ~ TransferList();

    Transfer *addTransfer(const KURL & _source, const KURL & _dest, 
                          bool canShow = true );

    virtual void setSelected(QListViewItem * item, bool selected);

    void moveToBegin(Transfer * item);
    void moveToEnd(Transfer * item);

    uint getPhasesNum()
    {
        return phasesNum;
    }
    bool updateStatus(int counter);
    Transfer * find(const KURL& _src);
    bool isQueueEmpty();

    void readTransfers(const KURL& file);
    void writeTransfers(const QString& file);

    friend class Transfer;

signals:
    void transferSelected(Transfer * item);
    void popupMenu(Transfer * item);

protected slots:
    void slotTransferSelected(QListViewItem * item);
    void slotPopupMenu(QListViewItem * item);

protected:

    void readConfig();
    void writeConfig();

    // ListView IDs
    int lv_pixmap, lv_filename, lv_resume, lv_count, lv_progress;
    int lv_total, lv_speed, lv_remaining, lv_url;

    QPtrList < QPixmap > animConn;
    QPtrList < QPixmap > animTry;
    QPixmap pixQueued;
    QPixmap pixScheduled;
    QPixmap pixDelayed;
    QPixmap pixFinished;
    QPixmap pixRetrying;

    uint phasesNum;
};


#endif                          // _TransferList_h
