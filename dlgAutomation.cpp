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
#include "common.h"

#include <kdialog.h>
#include <klineedit.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgAutomation.h"


DlgAutomation::DlgAutomation(QWidget * parent):QGroupBox(parent)
{
    setTitle(i18n("Automation Options"));

    QGridLayout *automationLayout = new QGridLayout(this, 5, 5, 20, KDialog::spacingHint());

    automationLayout->setRowStretch(0, 5);
    automationLayout->setRowStretch(1, 5);
    automationLayout->setRowStretch(2, 5);
    automationLayout->setRowStretch(3, 5);
    automationLayout->setRowStretch(4, 5);

    automationLayout->setColStretch(0, 5);
    automationLayout->setColStretch(1, 5);
    automationLayout->setColStretch(2, 5);
    automationLayout->setColStretch(3, 5);
    automationLayout->setColStretch(4, 5);

    // auto save
    cb_autoSave = new QCheckBox(i18n("Auto save file list every:"), this);
    automationLayout->addWidget(cb_autoSave, 0, 0);

    le_autoSave = new KIntNumInput(0, this, 10);
    le_autoSave->setSuffix(i18n(" minutes"));
    le_autoSave->setRange(1, 3600, 1, false);
    automationLayout->addMultiCellWidget(le_autoSave, 0, 0, 1, 2);

    connect(cb_autoSave, SIGNAL(toggled(bool)), le_autoSave, SLOT(setEnabled(bool)));


    cb_autoDisconnect = new QCheckBox(i18n("Auto disconnect after finished downloading"), this);
    automationLayout->addMultiCellWidget(cb_autoDisconnect, 1, 1, 0, 2);

    lb_autoDisconnect = new QLabel(i18n("Disconnect command:"), this);
    automationLayout->addWidget(lb_autoDisconnect, 2, 0, AlignRight | AlignVCenter);

    le_autoDisconnect = new KLineEdit(this);
    automationLayout->addMultiCellWidget(le_autoDisconnect, 2, 2, 1, 3);

    cb_timedDisconnect = new QCheckBox(i18n("Timed disconnect"), this);
    automationLayout->addMultiCellWidget(cb_timedDisconnect, 3, 3, 0, 2);

    spins = new BWDateTime(disconnectDateTime, this, "spins");
    automationLayout->addMultiCellWidget(spins, 4, 4, 0, 2);

    connect(cb_autoDisconnect, SIGNAL(toggled(bool)), this, SLOT(disconnectToggled(bool)));
    connect(cb_timedDisconnect, SIGNAL(toggled(bool)), spins, SLOT(setEnabled(bool)));

    // auto shutdown
    cb_autoShutdown = new QCheckBox(i18n("Auto shutdown after finished downloading"), this);
    automationLayout->addWidget(cb_autoShutdown, 5, 0);

    // auto paste
    cb_autoPaste = new QCheckBox(i18n("Auto paste from clipboard"), this);
    automationLayout->addWidget(cb_autoPaste, 6, 0);
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
        kmain->setAutoSave();
    }

    if (cb_autoDisconnect->isChecked() != ksettings.b_autoDisconnect) {
        kmain->slotToggleAutoDisconnect();
        kmain->setAutoDisconnect();
    }

    ksettings.disconnectCommand = le_autoDisconnect->text();
    ksettings.b_timedDisconnect = cb_timedDisconnect->isChecked();
    ksettings.disconnectDate = spins->getDateTime().date();
    ksettings.disconnectTime = spins->getDateTime().time();

    if (cb_autoShutdown->isChecked() != ksettings.b_autoShutdown) {
        kmain->slotToggleAutoShutdown();
    }

    if (cb_autoPaste->isChecked() != ksettings.b_autoPaste) {
        kmain->slotToggleAutoPaste();
    }
}

#include "dlgAutomation.moc"
