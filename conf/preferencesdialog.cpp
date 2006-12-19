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
#include "ui_dlgplugins.h"

PreferencesDialog::PreferencesDialog( QWidget * parent, KConfigSkeleton * skeleton )
    : KConfigDialog( parent, "preferences", skeleton )
{
    appearance = new QWidget(this);
    network = new QWidget(this);
    directories = new QWidget(this);
    advanced = new QWidget(this);
    plugins = new QWidget(this);

    Ui::DlgAppearance dlgApp;
    Ui::DlgNetwork dlgNet;
    Ui::DlgDirectories dlgDir;
    Ui::DlgAdvanced dlgAdv;
    Ui::DlgPlugins dlgPlugins;

    dlgApp.setupUi(appearance);
    dlgNet.setupUi(network);
    dlgDir.setupUi(directories);
    dlgAdv.setupUi(advanced);
    dlgPlugins.setupUi(plugins);

    addPage(appearance, i18n("Appearance"), "looknfeel", i18n("Look and Feel"));
    addPage(directories, i18n("Folders"), "folder_open", i18n("Default Download Folders"));
    addPage(network, i18n("Network"), "network", i18n("Network and Downloads"));
    addPage(advanced, i18n("Advanced"), "kget", i18n("Advanced Options"));
    addPage(plugins, i18n("Plugins"), "usbpendrive_mount", i18n("Plugins Options"));
}
