/***************************************************************************
*                               dlgSystem.h
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




#ifndef _DLGSYSTEM_H
#define _DLGSYSTEM_H

#include <qwidget.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

class DlgSystem:public QWidget
{
Q_OBJECT public:
        DlgSystem(QWidget * parent);
        ~DlgSystem()
        {}
        void applyData();
        void setData();

private:

        // sound settings
        QCheckBox * cb_useSound;

        QComboBox *cmb_sounds;
        QPushButton *pb_changesound;
        QPushButton *pb_testsound;

        QString soundAdded;
        QString soundStarted;
        QString soundFinished;
        QString soundFinishedAll;

        // animation settings
        QCheckBox *cb_useAnimation;

        // window style settings
        QButtonGroup *bg_window;
        QRadioButton *rb_normal;
        QRadioButton *rb_docked;
        QRadioButton *rb_droptarget;

        // font settings
        QGroupBox *gb_font;
        QLabel *lb_font;
        QPushButton *pb_browse;

private slots:
        void changeFont();

        void setupSound();
        void testSound();
};

#endif				// _DLGSYSTEM_H
