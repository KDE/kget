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

#ifdef index
#undef index
#endif

#include "kmainwidget.h"
#include "dlgPreferences.h"


DlgPreferences::DlgPreferences(QWidget * parent):
        KDialogBase(Tabbed, i18n("Configure"), Ok | Apply | Help | Cancel, Ok, parent, "", true)
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

    page = addPage(i18n("Limits"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    limDlg = new DlgLimits(page);
    topLayout->addWidget(limDlg);

    page = addPage(i18n("Advanced"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    advDlg = new DlgAdvanced(page);
    topLayout->addWidget(advDlg);

    // page = addPage(i18n("Search"));
    // topLayout = new QVBoxLayout(page, 0, spacingHint());
    //        seaDlg = new DlgSearch(page);
    // topLayout->addWidget(seaDlg);

    page = addPage(i18n("Directories"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    dirDlg = new DlgDirectories(page);
    topLayout->addWidget(dirDlg);

    page = addPage(i18n("System"));
    topLayout = new QVBoxLayout(page, 0, spacingHint());
    sysDlg = new DlgSystem(page);
    topLayout->addWidget(sysDlg);

    // type of connection influences autoDisconnect & timedDisconnect features
    connect(conDlg, SIGNAL(typeChanged(int)), autDlg, SLOT(slotTypeChanged(int)));

    connect(this, SIGNAL(applyClicked()), SLOT(applySettings()));

    conDlg->setData();
    autDlg->setData();
    limDlg->setData();
    advDlg->setData();
    //        seaDlg->setData();
    dirDlg->setData();
    sysDlg->setData();
}


void
DlgPreferences::closeEvent(QCloseEvent * e)
{
    kmain->m_paPreferences->setEnabled(true);
    KDialogBase::closeEvent(e);
}


void DlgPreferences::done(int r)
{
    if (r != Rejected) {
        applySettings();
    }

    hide();
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
}

#include "dlgPreferences.moc"
