/***************************************************************************
*                               dlgAutomation.h
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


#ifndef _DLGAUTOMATION_H
#define _DLGAUTOMATION_H

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qstringlist.h>

#include <knuminput.h>

#include "bwdatetime.h"

class DlgAutomation:public QGroupBox {

  Q_OBJECT public:

     DlgAutomation(QWidget * parent);
    ~DlgAutomation() {
    } void applyData();
    void setData();

  private:

    // auto save settings
    QCheckBox * cb_autoSave;
    KIntNumInput *le_autoSave;

    // auto disconnect settings
    QCheckBox *cb_autoDisconnect;
    QLabel *lb_autoDisconnect;
    QLineEdit *le_autoDisconnect;
    QCheckBox *cb_timedDisconnect;
    BWDateTime *spins;

    QDateTime disconnectDateTime;

    // auto shutdown settings
    QCheckBox *cb_autoShutdown;

    // auto paste settings
    QCheckBox *cb_autoPaste;

    public slots: void slotTypeChanged(int);

    protected slots:void disconnectToggled(bool);

};

#endif				// _DLGAUTOMATION_H
