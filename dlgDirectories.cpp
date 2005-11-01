/***************************************************************************
*                               dlgDirectories.cpp
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


#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlistview.h>

#include <qdir.h>

#include <kfiledialog.h>
#include <klineedit.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlrequester.h>

#include "settings.h"
#include "dlgDirectories.h"
#include <kapplication.h>

DlgDirectories::DlgDirectories(QWidget * parent)
    : DlgDirectoriesBase(parent)
{
    connect( le_ext, SIGNAL( textChanged ( const QString & ) ), this,  SLOT( slotDirectoryChanged( ) ) );
    connect( le_dir, SIGNAL( textChanged ( const QString & ) ), this,  SLOT( slotDirectoryChanged( ) ) );

    le_dir->setMode( KFile::Directory );
    lv_entries->setSortColumn( -1 );

    slotDirectoryChanged();
}

void DlgDirectories::slotDirectoryChanged( )
{
    pb_add->setEnabled(!le_ext->text().isEmpty() &&!le_dir->url().isEmpty() );
}

void DlgDirectories::selectEntry(QListViewItem * item)
{
    if (item) {
        le_ext->setText(item->text(0));
        le_dir->setURL(item->text(1));

    } else {
        le_ext->clear();
        le_dir->clear();
    }
    updateUpDown();
}


void DlgDirectories::updateUpDown()
{
    QListViewItem *item = lv_entries->selectedItem();

    pb_up->setEnabled( item && item->itemAbove() );
    pb_down->setEnabled( item && item->itemBelow() );
}

void DlgDirectories::addEntry()
{
    QString ext = le_ext->text();
    QString dir = le_dir->url();

    if (ext.contains(",") || dir.contains(",") || ext.isEmpty() || dir.isEmpty()) {
        KMessageBox::error(this, i18n("Each row consists of exactly one\nextension type and one folder."), i18n("Error"));
        return;
    }

    QDir f(dir);

    if (!f.exists()) {
        KMessageBox::error(this, i18n("Folder does not exist:\n%1").arg(dir), i18n("Error"));
        return;
    }

    new QListViewItem(lv_entries, ext, dir);
    updateUpDown();

    emit configChanged();
}


void DlgDirectories::deleteEntry()
{
    QListViewItem *item = lv_entries->selectedItem();
    delete item;
    updateUpDown();
    emit configChanged();
}


void DlgDirectories::changeEntry()
{
    QListViewItem *old_item = lv_entries->selectedItem();

    if (old_item) {
        QString ext = le_ext->text();
        QString dir = le_dir->url();

        if (ext.contains(",") || dir.contains(",") || ext.isEmpty() || dir.isEmpty()) {
            KMessageBox::error(this, i18n("Each row consists of exactly one\nextension type and one folder."), i18n("Error"));
            return;
        }

        QDir f(dir);

        if (!f.exists()) {
            KMessageBox::error(this, i18n("Folder does not exist:\n%1").arg(dir), i18n("Error"));
            return;
        }

        new QListViewItem(lv_entries, old_item, ext, dir);
        delete old_item;
        emit configChanged();
    }
}


void DlgDirectories::downEntry()
{
    QListViewItem *item = lv_entries->selectedItem();

    if ( !item )
        return;

    item->moveItem( item->itemBelow() );

    updateUpDown();
    emit configChanged();
}


void DlgDirectories::upEntry()
{
    QListViewItem *item = lv_entries->selectedItem();

    if ( !item || !item->itemAbove() )
        return;

    item->moveItem( item->itemAbove()->itemAbove() );

    updateUpDown();
    emit configChanged();
}


void DlgDirectories::setData()
{
    DirList::Iterator it;

    if (ksettings.defaultDirList.count() > 0) {
        // we need to insert items in the reverse order
        // because "new QListViewItem" puts itself at the beginning
        for (it = ksettings.defaultDirList.fromLast(); it != ksettings.defaultDirList.begin(); it--) {
            new QListViewItem(lv_entries, (*it).extRegexp, (*it).defaultDir);
        }
        new QListViewItem(lv_entries, (*it).extRegexp, (*it).defaultDir);
    }
}


void DlgDirectories::applyData()
{
    ksettings.defaultDirList.clear();
    QListViewItemIterator it(lv_entries);

    for (; it.current(); ++it) {
        QListViewItem *item = it.current();

        DirItem ditem;

        ditem.extRegexp = item->text(0);
        ditem.defaultDir = item->text(1);
        ksettings.defaultDirList.append(ditem);
    }
}

#include "dlgDirectories.moc"
