/***************************************************************************
*                               dlgDirectories.h
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


#ifndef _DLGDIRECTORIES_H
#define _DLGDIRECTORIES_H

#include <qgroupbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstringlist.h>

#include <qlistview.h>

class DlgDirectories:public QGroupBox
{

Q_OBJECT public:

        DlgDirectories(QWidget * parent);
        ~DlgDirectories()
        {}
        void applyData();
        void setData();

private:

        // table of entries
        QListView * lv_entries;

        // edit entries
        QLineEdit *le_ext;
        QLineEdit *le_dir;

        // maximum bandwidth
        QPushButton *pb_add;
        QPushButton *pb_delete;
        QPushButton *pb_change;
        QPushButton *pb_browse;

        QPushButton *pb_up;
        QPushButton *pb_down;

protected slots:
        void selectEntry(QListViewItem * item);
        void addEntry();
        void deleteEntry();
        void changeEntry();
        void browse();

        void upEntry();
        void downEntry();

};

#endif                          // _DLGDIRECTORIES_H
