/***************************************************************************
*                               dlgSystem.cpp
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

#ifdef index
#undef index
#endif

#include <kfontrequester.h>

#include "settings.h"
#include "kmainwidget.h"
#include "dlgSystem.h"

#include <qbuttongroup.h>
#include <qlabel.h>


DlgSystem::DlgSystem(QWidget * parent)
    : DlgSystemBase(parent)
{
    bg_window->hide();
    textLabel4->hide();
}


void DlgSystem::setData()
{
    cb_useAnimation->setChecked(ksettings.b_useAnimation);

    le_font->setFont(ksettings.listViewFont);
}


void DlgSystem::applyData()
{
    if (cb_useAnimation->isChecked() != ksettings.b_useAnimation)
    {
        kmain->slotToggleAnimation();
    }

    ksettings.listViewFont = le_font->font();
    kmain->setListFont();
}

void DlgSystem::slotChanged()
{
    emit configChanged();
}

#include "dlgSystem.moc"
