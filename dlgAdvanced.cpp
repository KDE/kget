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

#include <qcheckbox.h>
#include <qradiobutton.h>

#include <kprotocolmanager.h>

#include <klocale.h>
#include <kconfig.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgAdvanced.h"



DlgAdvanced::DlgAdvanced(QWidget * parent)
    : DlgAdvancedBase(parent)
{
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
        KConfig cfg("konquerorrc", false, false);
        cfg.setGroup("HTML Settings");
        cfg.writePathEntry("DownloadManager",QString((bIsKonquiEnable)?"kget":""));
        cfg.sync();
    }
}

void DlgAdvanced::slotChanged()
{
    emit configChanged();
}

#include "dlgAdvanced.moc"
