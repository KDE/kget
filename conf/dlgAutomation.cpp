/***************************************************************************
*                               dlgAutomation.cpp
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

#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kdatetimewidget.h>
#include <knuminput.h>
#include "globals.h"

#include <kdialog.h>
#include <klineedit.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgAutomation.h"


DlgAutomation::DlgAutomation(QWidget * parent)
    : DlgAutomationBase(parent)
{
}


void DlgAutomation::disconnectToggled(bool flag)
{
    le_autoDisconnect->setEnabled(flag);
    cb_timedDisconnect->setEnabled(flag);
    if (cb_timedDisconnect->isChecked()) {
      spins->setEnabled(flag);
    }
}


void DlgAutomation::slotTypeChanged(int type)
{
    if (type == PERMANENT) {
        disconnectToggled(false);
        cb_autoDisconnect->setEnabled(false);
        cb_autoDisconnect->setChecked(false);
        cb_timedDisconnect->setChecked(false);
    } else {
        cb_autoDisconnect->setEnabled(true);
    }
}


void DlgAutomation::setData()
{
    // auto save
    le_autoSave->setEnabled(ksettings.b_autoSave);
    cb_autoSave->setChecked(ksettings.b_autoSave);

    le_autoSave->setValue(ksettings.autoSaveInterval);

    // auto disconnect
    le_autoDisconnect->setEnabled(ksettings.b_autoDisconnect);
    cb_timedDisconnect->setEnabled(ksettings.b_autoDisconnect);
    spins->setEnabled(ksettings.b_autoDisconnect);
    cb_autoDisconnect->setChecked(ksettings.b_autoDisconnect);
    le_autoDisconnect->setText(ksettings.disconnectCommand);

    cb_timedDisconnect->setChecked(ksettings.b_timedDisconnect);
    spins->setEnabled(ksettings.b_timedDisconnect);

    disconnectDateTime.setDate(ksettings.disconnectDate);
    disconnectDateTime.setTime(ksettings.disconnectTime);
    spins->setDateTime(disconnectDateTime);

    // auto shutdown
    cb_autoShutdown->setChecked(ksettings.b_autoShutdown);

    // auto paste
    cb_autoPaste->setChecked(ksettings.b_autoPaste);
}


void DlgAutomation::applyData()
{
    if (cb_autoSave->isChecked() != ksettings.b_autoSave || (uint) le_autoSave->value() != ksettings.autoSaveInterval) {
        ksettings.b_autoSave = cb_autoSave->isChecked();
        ksettings.autoSaveInterval = le_autoSave->value();
        //FIXME kmain->setAutoSave();
    }

    if (cb_autoDisconnect->isChecked() != ksettings.b_autoDisconnect) {
        //FIXME kmain->slotToggleAutoDisconnect();
        //FIXME kmain->setAutoDisconnect();
    }

    ksettings.disconnectCommand = le_autoDisconnect->text();
    ksettings.b_timedDisconnect = cb_timedDisconnect->isChecked();
    ksettings.disconnectDate = spins->dateTime().date();
    ksettings.disconnectTime = spins->dateTime().time();

    if (cb_autoShutdown->isChecked() != ksettings.b_autoShutdown) {
        kmain->slotToggleAutoShutdown();
    }

    if (cb_autoPaste->isChecked() != ksettings.b_autoPaste) {
        //FIXME kmain->slotToggleAutoPaste();
    }
}

void DlgAutomation::slotChanged()
{
    emit configChanged();
}
#include "dlgAutomation.moc"
