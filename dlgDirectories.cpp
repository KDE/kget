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


#include <qlayout.h>

#ifdef Unsorted
#undef Unsorted
#endif

#include <qdir.h>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "settings.h"
#include "dlgDirectories.h"
#include <kapp.h>

DlgDirectories::DlgDirectories(QWidget * parent):QGroupBox(parent)
{

        setTitle(i18n("Directories Options"));

        QGridLayout *directoriesLayout = new QGridLayout(this, 4, 5, 20, 5);

        directoriesLayout->setRowStretch(0, 5);
        directoriesLayout->setRowStretch(1, 5);
        directoriesLayout->setRowStretch(2, 3);
        directoriesLayout->setRowStretch(3, 3);

        directoriesLayout->setColStretch(0, 5);
        directoriesLayout->setColStretch(1, 5);
        directoriesLayout->setColStretch(2, 5);
        directoriesLayout->setColStretch(3, 5);
        directoriesLayout->setColStretch(4, 3);

        // table of entries
        lv_entries = new QListView(this, "dirview");
        lv_entries->setMultiSelection(false);
        lv_entries->setAllColumnsShowFocus(true);
        lv_entries->setSorting(-1); // do not sort automatically

        lv_entries->addColumn(i18n("Extension"));
        lv_entries->addColumn(i18n("Default Directory"));
        connect(lv_entries, SIGNAL(selectionChanged(QListViewItem *)), SLOT(selectEntry(QListViewItem *)));

        // lv_entries->setMinimumHeight( lv_entries->sizeHint().height() );
        directoriesLayout->addMultiCellWidget(lv_entries, 0, 1, 0, 3);

        pb_up = new QPushButton(this);
        pb_up->setPixmap(BarIcon("up"));
        connect(pb_up, SIGNAL(clicked()), SLOT(upEntry()));
        directoriesLayout->addWidget(pb_up, 0, 4);
        pb_up->setEnabled(false);

        pb_down = new QPushButton(this);
        pb_down->setPixmap(BarIcon("down"));
        connect(pb_down, SIGNAL(clicked()), SLOT(downEntry()));
        directoriesLayout->addWidget(pb_down, 1, 4);
        pb_down->setEnabled(false);

        // edit entries
        le_ext = new QLineEdit(this);
        directoriesLayout->addMultiCellWidget(le_ext, 2, 2, 0, 1);

        le_dir = new QLineEdit(this);
        directoriesLayout->addMultiCellWidget(le_dir, 2, 2, 2, 3);

        // edit buttons
        pb_add = new QPushButton(i18n("Add"), this);
        directoriesLayout->addWidget(pb_add, 3, 0);
        connect(pb_add, SIGNAL(clicked()), SLOT(addEntry()));

        pb_delete = new QPushButton(i18n("Delete"), this);
        directoriesLayout->addWidget(pb_delete, 3, 1);
        connect(pb_delete, SIGNAL(clicked()), SLOT(deleteEntry()));

        pb_change = new QPushButton(i18n("Change"), this);
        directoriesLayout->addWidget(pb_change, 3, 2);
        connect(pb_change, SIGNAL(clicked()), SLOT(changeEntry()));

        pb_browse = new QPushButton(i18n("Browse"), this);
        directoriesLayout->addWidget(pb_browse, 3, 3);
        connect(pb_browse, SIGNAL(clicked()), SLOT(browse()));
}


void DlgDirectories::selectEntry(QListViewItem * item)
{
        if (item) {

                le_ext->setText(item->text(0));
                le_dir->setText(item->text(1));
                pb_up->setEnabled(true);
                pb_down->setEnabled(true);

        } else {
                pb_up->setEnabled(false);
                pb_down->setEnabled(false);
        }
}


void DlgDirectories::addEntry()
{
        QString ext = le_ext->text();
        QString dir = le_dir->text();

        if (ext.contains(",") || dir.contains(",") || ext.isEmpty() || dir.isEmpty()) {
                KMessageBox::error(this, i18n("Each row consists of exactly one\n extension type and one directory"), i18n("Error"));
                return;
        }

        QDir f(dir);

        if (!f.exists()) {
                KMessageBox::error(this, i18n("Directory doesn't exist:\n") + dir, i18n("Error"));
                return;
        }

        new QListViewItem(lv_entries, ext, dir);
}


void DlgDirectories::deleteEntry()
{
        QListViewItem *item = lv_entries->selectedItem();

        if (item) {
                delete item;
        }
}


void DlgDirectories::changeEntry()
{
        QListViewItem *old_item = lv_entries->selectedItem();

        if (old_item) {
                QString ext = le_ext->text();
                QString dir = le_dir->text();

                if (ext.contains(",") || dir.contains(",") || ext.isEmpty() || dir.isEmpty()) {
                        KMessageBox::error(this, i18n("Each row consists of exactly one\n extension type and one directory"), i18n("Error"));
                        return;
                }

                QDir f(dir);

                if (!f.exists()) {
                        KMessageBox::error(this, i18n("Directory doesn't exist :\n") + dir, i18n("Error"));
                        return;
                }

                new QListViewItem(lv_entries, old_item, ext, dir);
                delete old_item;
        }
}


void DlgDirectories::downEntry()
{
        QListViewItem *old_item = lv_entries->selectedItem();

        if (old_item) {
                QListViewItemIterator it(old_item);

                if (it.current()->nextSibling() == 0L) {
                        return;
                }

                QString ext = old_item->text(0);
                QString dir = old_item->text(1);

                it++;

                QListViewItem *new_item = new QListViewItem(lv_entries, it.current(), ext, dir);

                delete old_item;

                lv_entries->setSelected(new_item, true);
        }
}


void DlgDirectories::upEntry()
{
        QListViewItem *old_item = lv_entries->selectedItem();

        if (old_item) {
                QListViewItemIterator it(old_item);

                QString ext = old_item->text(0);
                QString dir = old_item->text(1);

                it--;
                it--;

                QListViewItem *new_item = new QListViewItem(lv_entries, it.current(), ext, dir);

                delete old_item;

                lv_entries->setSelected(new_item, true);
        }
}


void DlgDirectories::browse()
{

        le_dir->setText(KFileDialog::getExistingDirectory());

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
