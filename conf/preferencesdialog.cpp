/* This file is part of the KDE project
   Copyright (C) 2004 - 2007 KGet Developers <kget@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "preferencesdialog.h"

#include "ui_dlgappearance.h"
#include "ui_dlgnetwork.h"
#include "dlgdirectories.h"
#include "ui_dlgadvanced.h"

#include <klocale.h>
#include <ktabwidget.h>

PreferencesDialog::PreferencesDialog(QWidget * parent, KConfigSkeleton * skeleton)
    : KConfigDialog(parent, "preferences", skeleton)
{
    appearance = new QWidget(this);
    DlgDirectories *directories = new DlgDirectories(this);
    network = new QWidget(this);
    advanced = new QWidget(this);
    plugins = new KTabWidget(this);

    Ui::DlgAppearance dlgApp;
    Ui::DlgNetwork dlgNet;
    Ui::DlgAdvanced dlgAdv;

    dlgApp.setupUi(appearance);
    dlgNet.setupUi(network);
    dlgAdv.setupUi(advanced);

    addPage(appearance, i18n("Appearance"), "screen", i18n("Look and Feel"));
    addPage(directories, i18n("Folders"), "folder-open", i18n("Default Download Folders"));
    addPage(network, i18n("Network"), "network-wired", i18n("Network and Downloads"));
    addPage(advanced, i18n("Advanced"), "kget", i18n("Advanced Options"));
    addPage(plugins, i18n("Plugins"), "usbpendrive-unmount", i18n("Transfer Plugin Options"));
}
