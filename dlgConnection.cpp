/***************************************************************************
*                               dlgConnection.cpp
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

#include <klocale.h>
#include <kprotocolmanager.h>
#include <kdialog.h>

#include "kmainwidget.h"
#include "settings.h"
#include "dlgConnection.h"


DlgConnection::DlgConnection(QWidget * parent):QWidget(parent, "", 0)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this, 0, KDialog::spacingHint());

    // reconnect settings
    gb_reconnect = new QGroupBox(this, "gb_reconnect");
    gb_reconnect->setTitle(i18n("Reconnecting Options"));
    topLayout->addWidget(gb_reconnect, 15);

    QGridLayout *reconnectLayout = new QGridLayout(gb_reconnect, 5, 4, 20, KDialog::spacingHint());

    reconnectLayout->setRowStretch(0, 5);
    reconnectLayout->setRowStretch(1, 5);
    reconnectLayout->setRowStretch(2, 5);
    reconnectLayout->setRowStretch(3, 5);
    reconnectLayout->setRowStretch(4, 5);

    reconnectLayout->setColStretch(0, 5);
    reconnectLayout->setColStretch(1, 5);
    reconnectLayout->setColStretch(2, 5);
    reconnectLayout->setColStretch(3, 5);

    cb_onerror = new QCheckBox(i18n("On login or timeout error"), gb_reconnect);
    reconnectLayout->addWidget(cb_onerror, 0, 0);

    lb_after = new QLabel(i18n("Reconnect after:"), gb_reconnect);
    reconnectLayout->addWidget(lb_after, 1, 1);

    le_after = new KIntNumInput(0, gb_reconnect, 10, "le_after");
    le_after->setSuffix(i18n(" minutes"));
    le_after->setRange(1, 3600, 1, false);
    reconnectLayout->addWidget(le_after, 1, 2);

    lb_retries = new QLabel(i18n("Number of retries:"), gb_reconnect);
    reconnectLayout->addWidget(lb_retries, 2, 1);

    le_retries = new KIntNumInput(0, gb_reconnect, 10, "le_retries");
    le_retries->setRange(1, 3600, 1, false);
    reconnectLayout->addWidget(le_retries, 2, 2);

    connect(cb_onerror, SIGNAL(toggled(bool)), le_after, SLOT(setEnabled(bool)));
    connect(cb_onerror, SIGNAL(toggled(bool)), le_retries, SLOT(setEnabled(bool)));

    cb_onbroken = new QCheckBox(i18n("On broken connection"), gb_reconnect);
    reconnectLayout->addWidget(cb_onbroken, 3, 0);

    cb_autoresume = new QCheckBox(i18n("Automatically resume if possible"), gb_reconnect);
    reconnectLayout->addMultiCellWidget(cb_autoresume, 4, 4, 0, 2);

    // timeout settings
    gb_timeout = new QGroupBox(this, "gb_timeout");
    gb_timeout->setTitle(i18n("Timeout Options"));
    topLayout->addWidget(gb_timeout, 10);

    QGridLayout *timeoutLayout = new QGridLayout(gb_timeout, 2, 4, 20, KDialog::spacingHint());

    timeoutLayout->setRowStretch(0, 5);
    timeoutLayout->setRowStretch(1, 5);

    timeoutLayout->setColStretch(0, 5);
    timeoutLayout->setColStretch(1, 5);
    timeoutLayout->setColStretch(2, 5);
    timeoutLayout->setColStretch(3, 5);

    lb_nodata = new QLabel(i18n("If no data arrives in"), gb_timeout);
    timeoutLayout->addMultiCellWidget(lb_nodata, 0, 0, 0, 1);

    le_nodata = new KIntNumInput(0, gb_timeout, 10);
    timeoutLayout->addWidget(le_nodata, 0, 2);
    le_nodata->setRange(1, 3600, 1, false);

    lb_noresume = new QLabel(i18n("Or"), gb_timeout);
    timeoutLayout->addWidget(lb_noresume, 1, 0, AlignCenter);

    le_noresume = new KIntNumInput(0, gb_timeout, 10);
    le_noresume->setSuffix(i18n(" minutes"));
    le_noresume->setRange(1, 3600, 1, false);
    timeoutLayout->addWidget(le_noresume, 1, 1);

    lb_cannot = new QLabel(i18n("if server can't resume"), gb_timeout);
    timeoutLayout->addMultiCellWidget(lb_cannot, 1, 1, 2, 3);

    // type settings
    gb_type = new QGroupBox(this, "gb_type");
    gb_type->setTitle(i18n("Type of Connection"));
    topLayout->addWidget(gb_type, 10);

    QGridLayout *typeLayout = new QGridLayout(gb_type, 2, 3, 20, KDialog::spacingHint());

    typeLayout->setRowStretch(0, 5);
    typeLayout->setRowStretch(1, 5);

    typeLayout->setColStretch(0, 5);
    typeLayout->setColStretch(1, 5);
    typeLayout->setColStretch(2, 5);

    cmb_type = new QComboBox(gb_type);

    cmb_type->insertItem(i18n("Permanent"));
    cmb_type->insertItem(i18n("Ethernet"));
    cmb_type->insertItem(i18n("PLIP"));
    cmb_type->insertItem(i18n("SLIP"));
    cmb_type->insertItem(i18n("PPP"));
    cmb_type->insertItem(i18n("ISDN"));
    typeLayout->addWidget(cmb_type, 0, 0);

    lb_linknum = new QLabel(i18n("Link number:"), gb_type);
    typeLayout->addWidget(lb_linknum, 0, 1, AlignCenter);

    le_linknum = new KIntNumInput(0, gb_type, 10);
    typeLayout->addWidget(le_linknum, 0, 2);
    le_linknum->setRange(0, 100, 1, false);

    cb_offlinemode = new QCheckBox(i18n("Offline mode"), gb_type);
    typeLayout->addWidget(cb_offlinemode, 1, 0);

    connect(cmb_type, SIGNAL(activated(int)), this, SLOT(comboActivated(int)));

    // TODO: these are not supported yet, so disable them
    le_nodata->setEnabled(false);
    le_noresume->setEnabled(false);
}


void DlgConnection::comboActivated(int Index)
{
    if (Index == 0) {
        le_linknum->setEnabled(false);
        cb_offlinemode->setEnabled(true);
    } else {
        le_linknum->setEnabled(true);
        cb_offlinemode->setEnabled(false);
        cb_offlinemode->setChecked(false);
    }

    emit typeChanged(Index);
}


void DlgConnection::setData()
{
    le_after->setEnabled(ksettings.b_reconnectOnError);
    le_retries->setEnabled(ksettings.b_reconnectOnError);
    cb_onerror->setChecked(ksettings.b_reconnectOnError);

    le_after->setValue(ksettings.reconnectTime);
    le_retries->setValue(ksettings.reconnectRetries);

    cb_onbroken->setChecked(ksettings.b_reconnectOnBroken);

    cb_autoresume->setChecked(KProtocolManager::autoResume());

    le_nodata->setValue(ksettings.timeoutData);
    le_noresume->setValue(ksettings.timeoutDataNoResume);

    cmb_type->setCurrentItem(ksettings.connectionType);

    if (cmb_type->currentItem() == 0) {
        le_linknum->setValue(0);
        le_linknum->setEnabled(false);
    } else {
        le_linknum->setValue(ksettings.linkNumber);
        le_linknum->setEnabled(true);
    }

    cb_offlinemode->setChecked(ksettings.b_offlineMode);
    if (ksettings.connectionType == 0)
        cb_offlinemode->setChecked(ksettings.b_offlineMode);
    else {
        cb_offlinemode->setEnabled(false);
        cb_offlinemode->setChecked(false);
    }
}


void DlgConnection::applyData()
{
    ksettings.b_reconnectOnError = cb_onerror->isChecked();
    ksettings.reconnectTime = le_after->value();
    ksettings.reconnectRetries = le_retries->value();
    ksettings.b_reconnectOnBroken = cb_onbroken->isChecked();

    // KProtocolManager::setAutoResume(cb_autoresume->isChecked());

    ksettings.timeoutData = le_nodata->value();
    ksettings.timeoutDataNoResume = le_noresume->value();

    ksettings.connectionType = cmb_type->currentItem();
    ksettings.linkNumber = le_linknum->value();

    if (cb_offlinemode->isChecked() != ksettings.b_offlineMode) {
        kmain->slotToggleOfflineMode();
    }
}

#include "dlgConnection.moc"
