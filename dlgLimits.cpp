/***************************************************************************
*                               dlgLimits.cpp
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


#include <qlabel.h>
#include <qlayout.h>

#include <knuminput.h>
#include <klocale.h>
#include <kdialog.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgLimits.h"


DlgLimits::DlgLimits(QWidget * parent)
    : DlgLimitsBase(parent)
{
    // TODO: these are not supported yet, so hide them
    lb_maxband->hide();
    le_maxband->hide();
    lb_minband->hide();
    le_minband->hide();
}


void DlgLimits::setData()
{
    le_maxnum->setValue(ksettings.maxSimultaneousConnections);
    le_minband->setValue(ksettings.minimumBandwidth);
    le_maxband->setValue(ksettings.maximumBandwidth);
}


void DlgLimits::applyData()
{
    ksettings.maxSimultaneousConnections = le_maxnum->value();
    ksettings.minimumBandwidth = le_minband->value();
    ksettings.maximumBandwidth = le_maxband->value();
    kmain->checkQueue();
}

void DlgLimits::slotChanged()
{
    emit configChanged();
}

#include "dlgLimits.moc"
