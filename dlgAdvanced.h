/***************************************************************************
*                                dlgAdvanced.h
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


#ifndef _DLGADVANCED_H
#define _DLGADVANCED_H

#include <qgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qstringlist.h>

class DlgAdvanced:public QGroupBox
{

Q_OBJECT public:

        DlgAdvanced(QWidget * parent);
        ~DlgAdvanced();

        void applyData();
        void setData();

private:

        QLabel * lb_adding;
        QButtonGroup *bg_adding;
        QRadioButton *rb_queued;
        QRadioButton *rb_delayed;

        QCheckBox *cb_individual;
        QCheckBox *cb_iconify;
        QCheckBox *cb_advanced;
        QCheckBox *cb_partial;
        QCheckBox *cb_remove;
        QCheckBox *cb_getsizes;
        QCheckBox *cb_expertmode;

};

#endif				// _DLGADVANCED_H
