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
#include "dlgAdvanced.h"


DlgAdvanced::DlgAdvanced(QWidget * parent)
    : DlgAdvancedBase(parent)
{
    cb_partial->setChecked(KProtocolManager::markPartial());
/*    bool bIsKonquiEnable=cb_konqiIntegration->isChecked();
    if (Settings::konquerorIntegration()!=bIsKonquiEnable)
    {
        Settings::setKonquerorIntegration( bIsKonquiEnable );
        KConfig cfg("konquerorrc", false, false);
        cfg.setGroup("HTML Settings");
        cfg.writePathEntry("DownloadManager",QString((bIsKonquiEnable)?"kget":""));
        cfg.sync();
    }*/
}

#include "dlgAdvanced.moc"
