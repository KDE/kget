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


#include <qgroupbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kprotocolmanager.h>
#include <kdialog.h>
#include <knuminput.h>
#include <kcombobox.h>


#include "kmainwidget.h"
#include "settings.h"
#include "dlgConnection.h"


DlgConnection::DlgConnection(QWidget * parent)
        :DlgConnectionBase(parent, "", 0)
{
    // TODO: these are not supported yet, so disable them
    le_nodata->setEnabled(false);
    le_noresume->setEnabled(false);
}


void DlgConnection::comboActivated(int Index)
{
    if (Index == 0) {
        lb_linknum->setEnabled(false);
        le_linknum->setEnabled(false);
        cb_offlinemode->setEnabled(true);
    } else {
        lb_linknum->setEnabled(true);
        le_linknum->setEnabled(true);
        cb_offlinemode->setEnabled(false);
        cb_offlinemode->setChecked(false);
    }

    emit typeChanged(Index);
}


void DlgConnection::setData()
{
    lb_after->setEnabled(ksettings.b_reconnectOnError);
    le_after->setEnabled(ksettings.b_reconnectOnError);
    lb_retries->setEnabled(ksettings.b_reconnectOnError);
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
        lb_linknum->setEnabled(false);
        le_linknum->setEnabled(false);
    } else {
        le_linknum->setValue(ksettings.linkNumber);
        lb_linknum->setEnabled(true);
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

void DlgConnection::slotChanged()
{
    emit configChanged();
}

#include "dlgConnection.moc"
