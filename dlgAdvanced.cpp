/***************************************************************************
*                               dlgAdvanced.cpp
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

#include <kprotocolmanager.h>

#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgAdvanced.h"



DlgAdvanced::DlgAdvanced(QWidget * parent):QGroupBox(parent)
{

    setTitle(i18n("Advanced Options"));

    QGridLayout *gridLayout = new QGridLayout(this, 7, 3, 20, KDialog::spacingHint());

    gridLayout->setRowStretch(0, 5);
    gridLayout->setRowStretch(1, 5);
    gridLayout->setRowStretch(2, 5);
    gridLayout->setRowStretch(3, 5);
    gridLayout->setRowStretch(4, 5);
    gridLayout->setRowStretch(5, 5);
    gridLayout->setRowStretch(6, 5);
    gridLayout->setRowStretch(7, 5);

    gridLayout->setColStretch(0, 5);
    gridLayout->setColStretch(1, 5);
    gridLayout->setColStretch(2, 5);

    // adding new transfers
    lb_adding = new QLabel(i18n("Add new transfers as:"), this);
    gridLayout->addWidget(lb_adding, 0, 0);

    bg_adding = new QButtonGroup();

    rb_queued = new QRadioButton(i18n("Queued"), this);
    bg_adding->insert(rb_queued);
    gridLayout->addWidget(rb_queued, 0, 1);

    rb_delayed = new QRadioButton(i18n("Delayed"), this);
    bg_adding->insert(rb_delayed);
    gridLayout->addWidget(rb_delayed, 0, 2);

    //
    cb_individual = new QCheckBox(i18n("Show individual windows"), this);
    gridLayout->addWidget(cb_individual, 1, 0);

    cb_iconify = new QCheckBox(i18n("Iconified"), this);
    gridLayout->addWidget(cb_iconify, 1, 1);

    connect(cb_individual, SIGNAL(toggled(bool)), cb_iconify, SLOT(setEnabled(bool)));

    cb_advanced = new QCheckBox(i18n("Advanced individual windows"), this);
    gridLayout->addWidget(cb_advanced, 2, 0);

    //
    cb_partial = new QCheckBox(i18n("Mark partial downloads"), this);
    gridLayout->addMultiCellWidget(cb_partial, 3, 3, 0, 2);

    //
    cb_remove = new QCheckBox(i18n("Remove files from a list after success"), this);
    gridLayout->addMultiCellWidget(cb_remove, 4, 4, 0, 2);

    //
    cb_getsizes = new QCheckBox(i18n("Get file sizes"), this);
    gridLayout->addMultiCellWidget(cb_getsizes, 5, 5, 0, 2);

    //
    cb_expertmode = new QCheckBox(i18n("Expert mode (don't prompt for cancel or delete)"), this);
    gridLayout->addMultiCellWidget(cb_expertmode, 6, 6, 0, 2);

    cb_konqiIntegration= new QCheckBox(i18n("Enable the integration with Konqueror"), this);
    gridLayout->addMultiCellWidget(cb_konqiIntegration, 7, 7, 0, 2);

    cb_ShowMain = new QCheckBox(i18n("Show main window at startup"), this);
    gridLayout->addMultiCellWidget(cb_ShowMain, 8, 8, 0, 2);


}


DlgAdvanced::~DlgAdvanced()
{
    delete bg_adding;
}


void
DlgAdvanced::setData()
{
    rb_queued->setChecked(ksettings.b_addQueued);
    rb_delayed->setChecked(!ksettings.b_addQueued);
    cb_iconify->setEnabled(ksettings.b_showIndividual);
    cb_individual->setChecked(ksettings.b_showIndividual);
    cb_iconify->setChecked(ksettings.b_iconifyIndividual);
    cb_advanced->setChecked(ksettings.b_advancedIndividual);
    cb_remove->setChecked(ksettings.b_removeOnSuccess);
    cb_getsizes->setChecked(ksettings.b_getSizes);
    cb_expertmode->setChecked(ksettings.b_expertMode);
    cb_partial->setChecked(KProtocolManager::markPartial());
    cb_konqiIntegration->setChecked(ksettings.b_KonquerorIntegration);
    cb_ShowMain->setChecked(ksettings.b_showMain);
}


void DlgAdvanced::applyData()
{

    ksettings.b_addQueued = rb_queued->isChecked();
    ksettings.b_showIndividual = cb_individual->isChecked();
    ksettings.b_iconifyIndividual = cb_iconify->isChecked();
    ksettings.b_advancedIndividual = cb_advanced->isChecked();
    ksettings.b_removeOnSuccess = cb_remove->isChecked();
    ksettings.b_getSizes = cb_getsizes->isChecked();
    ksettings.b_showMain=cb_ShowMain->isChecked();

    if (ksettings.b_expertMode != cb_expertmode->isChecked()) {
        kmain->slotToggleExpertMode();
    }

    bool bIsKonquiEnable=cb_konqiIntegration->isChecked();

    if (ksettings.b_KonquerorIntegration!=bIsKonquiEnable)
    {
        ksettings.b_KonquerorIntegration=!ksettings.b_KonquerorIntegration;
        KConfig *cfg = new KConfig("konquerorrc", false, false);
        cfg->setGroup("HTML Settings");
        cfg->writeEntry("DownloadManager",(bIsKonquiEnable)?"kget":"");
        cfg->sync();
        delete cfg;
    }


}

#include "dlgAdvanced.moc"
