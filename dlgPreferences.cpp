/***************************************************************************
*                               dlgPreferences.cpp
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

#include <kapplication.h>
#include <klocale.h>
#include <kaction.h>
#include <kcombobox.h>

#include "dlgConnection.h"
#include "dlgAutomation.h"
#include "dlgLimits.h"
#include "dlgAdvanced.h"
#include "dlgDirectories.h"
#include "dlgSystem.h"

#include "settings.h"

#ifdef index
#undef index
#endif

#include "kmainwidget.h"
#include "dlgPreferences.h"


DlgPreferences::DlgPreferences(QWidget * parent):
        KDialogBase(Tabbed, i18n("Configure"), Ok|Apply|Help|Cancel, Ok, parent, "DlgPreferences", true)
{
    // add pages
    QFrame *page = addPage(i18n("Connection"));
    QVBoxLayout *topLayout = new QVBoxLayout(page, 0, spacingHint());

    conDlg = new DlgConnection(page);
    topLayout->addWidget(conDlg);

    page = addPage(i18n("Automation"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    autDlg = new DlgAutomation(page);
    topLayout->addWidget(autDlg);
    topLayout->addStretch();

    page = addPage(i18n("Limits"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    limDlg = new DlgLimits(page);
    topLayout->addWidget(limDlg);
    topLayout->addStretch();

    page = addPage(i18n("Advanced"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    advDlg = new DlgAdvanced(page);
    topLayout->addWidget(advDlg);
    topLayout->addStretch();

    // page = addPage(i18n("Search"));
    // topLayout = new QVBoxLayout(page, 0, spacingHint());
    //        seaDlg = new DlgSearch(page);
    // topLayout->addWidget(seaDlg);

    page = addPage(i18n("Folders"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    dirDlg = new DlgDirectories(page);
    topLayout->addWidget(dirDlg);
    topLayout->addStretch();

    page = addPage(i18n("System"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    sysDlg = new DlgSystem(page);
    topLayout->addWidget(sysDlg);
    topLayout->addStretch();

    // type of connection influences autoDisconnect & timedDisconnect features
    connect(conDlg, SIGNAL(typeChanged(int)), autDlg, SLOT(slotTypeChanged(int)));

    loadAllData();

    connect( conDlg, SIGNAL( configChanged() ), this, SLOT( slotChanged() ) );
    connect( autDlg, SIGNAL( configChanged() ), this, SLOT( slotChanged() ) );
    connect( limDlg, SIGNAL( configChanged() ), this, SLOT( slotChanged() ) );
    connect( advDlg, SIGNAL( configChanged() ), this, SLOT( slotChanged() ) );
    connect( dirDlg, SIGNAL( configChanged() ), this, SLOT( slotChanged() ) );
    connect( sysDlg, SIGNAL( configChanged() ), this, SLOT( slotChanged() ) );
}


void DlgPreferences::slotChanged()
{
    changed = true;
    enableButton( Apply, true );
}

void DlgPreferences::applySettings()
{
    conDlg->applyData();
    autDlg->applyData();
    limDlg->applyData();
    advDlg->applyData();
    //        seaDlg->applyData();
    dirDlg->applyData();
    sysDlg->applyData();

    ksettings.save();
    changed = false;
    enableButton( Apply, false );
}

void DlgPreferences::slotOk()
{
    if ( changed )
        applySettings();
    accept();
}

void DlgPreferences::slotCancel()
{
    if ( changed )
        loadAllData();
    reject();
}

void DlgPreferences::slotApply()
{
    applySettings();
}

void DlgPreferences::loadAllData()
{
    conDlg->setData();
    autDlg->setData();
    limDlg->setData();
    advDlg->setData();
    //        seaDlg->setData();
    dirDlg->setData();
    sysDlg->setData();
    changed = false;
    enableButton( Apply, false );
}

#include "dlgPreferences.moc"
