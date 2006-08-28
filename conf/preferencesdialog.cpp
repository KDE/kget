/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <klocale.h>

#include "preferencesdialog.h"

// single config pages
#include "ui_dlgappearance.h"
#include "ui_dlgnetwork.h"
#include "ui_dlgdirectories.h"
#include "ui_dlgadvanced.h"

PreferencesDialog::PreferencesDialog( QWidget * parent, KConfigSkeleton * skeleton )
    : KConfigDialog( parent, "preferences", skeleton )
{
    appearance = new QWidget(this);
    network = new QWidget(this);
    directories = new QWidget(this);
    advanced = new QWidget(this);

    Ui::DlgAppearance dlgApp;
    Ui::DlgNetwork dlgNet;
    Ui::DlgDirectories dlgDir;
    Ui::DlgAdvanced dlgAdv;

    dlgApp.setupUi(appearance);
    dlgNet.setupUi(network);
    dlgDir.setupUi(directories);
    dlgAdv.setupUi(advanced);

    //WORKAROUND: create network as the first page because it's the biggeset one
    addPage(network, i18n("Network"), "network", i18n("Network and Downloads"));
    addPage(appearance, i18n("Appearance"), "looknfeel", i18n("Look and Feel"));
    addPage(directories, i18n("Folders"), "folder_open", i18n("Default Download Folders"));
    addPage(advanced, i18n("Advanced"), "kget", i18n("Advanced Options"));
}
