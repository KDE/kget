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
    lb_after->setEnabled(Settings::reconnectOnError());
    le_after->setEnabled(Settings::reconnectOnError());
//    lb_retries->setEnabled(Settings::reconnectOnError());
//    le_retries->setEnabled(Settings::reconnectOnError());
    cb_onerror->setChecked(Settings::reconnectOnError());

    le_after->setValue(Settings::reconnectTime());
    le_retries->setValue(Settings::reconnectRetries());

    cb_onbroken->setChecked(Settings::reconnectOnBroken());

//    cb_autoresume->setChecked(KProtocolManager::autoResume());

    le_nodata->setValue(Settings::timeoutData());
    le_noresume->setValue(Settings::timeoutDataNoResume());

    cmb_type->setCurrentItem(Settings::connectionType());

    if (cmb_type->currentItem() == 0) {
        le_linknum->setValue(0);
        lb_linknum->setEnabled(false);
        le_linknum->setEnabled(false);
    } else {
        le_linknum->setValue(Settings::linkNumber());
        lb_linknum->setEnabled(true);
        le_linknum->setEnabled(true);
    }

    cb_offlinemode->setChecked(Settings::offlineMode());
    if (Settings::connectionType() == 0)
        cb_offlinemode->setChecked(Settings::offlineMode());
    else {
        cb_offlinemode->setEnabled(false);
        cb_offlinemode->setChecked(false);
    }
}


void DlgConnection::applyData()
{
    Settings::setReconnectOnError( cb_onerror->isChecked() );
    Settings::setReconnectTime( le_after->value() );
    Settings::setReconnectRetries( le_retries->value() );
    Settings::setReconnectOnBroken( cb_onbroken->isChecked() );

    // KProtocolManager::setAutoResume(cb_autoresume->isChecked());

    Settings::setTimeoutData( le_nodata->value() );
    Settings::setTimeoutDataNoResume( le_noresume->value() );

    Settings::setConnectionType( cmb_type->currentItem() );
    Settings::setLinkNumber( le_linknum->value() );

    if (cb_offlinemode->isChecked() != Settings::offlineMode()) {
        //FIXME kmain->slotToggleOfflineMode();
    }
}

int DlgConnection::type() const
{
    return cmb_type->currentItem();
}

void DlgConnection::slotChanged()
{
    emit configChanged();
}

#include "dlgConnection.moc"
