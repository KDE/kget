/* This file is part of the KDE project
   Copyright (C) 2004 - 2007 KGet Developers <kget@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "preferencesdialog.h"

#include "ui_dlgappearance.h"
#include "ui_dlgnetwork.h"
#include "dlgdirectories.h"
#include "ui_dlgadvanced.h"
#include "transfersgroupwidget.h"

#include <klocale.h>
#include <ktabwidget.h>

PreferencesDialog::PreferencesDialog(QWidget * parent, KConfigSkeleton * skeleton)
    : KConfigDialog(parent, "preferences", skeleton)
{
    appearance = new QWidget(this);
    groups = new QWidget(this);
    DlgDirectories *directories = new DlgDirectories(this);
    network = new QWidget(this);
    advanced = new QWidget(this);
    plugins = new KTabWidget(this);

    groups->setLayout(new TransfersGroupWidget());

    Ui::DlgAppearance dlgApp;
    Ui::DlgNetwork dlgNet;
    Ui::DlgAdvanced dlgAdv;

    dlgApp.setupUi(appearance);
    dlgNet.setupUi(network);
    dlgAdv.setupUi(advanced);

    // TODO: remove the following lines as soon as these features are ready
    dlgNet.kcfg_SpeedLimit->setVisible(false);
    dlgNet.groupBoxCompleted->setVisible(false);
    dlgNet.lbl_maxnum_2->setVisible(false);
    dlgNet.kcfg_MaxConnectionsServer->setVisible(false);

    addPage(appearance, i18n("Appearance"), "video-display", i18n("Change appearance settings"));
    addPage(groups, i18n("Groups"), "bookmark", i18n("Manage the groups"));
    addPage(directories, i18n("Folders"), "folder-open", i18n("Default Download Folders"));
    addPage(network, i18n("Network"), "network-wired", i18n("Network and Downloads"));
    addPage(advanced, i18n("Advanced"), "kget", i18n("Advanced Options"));
    addPage(plugins, i18n("Plugins"), "drive-removable-media-usb-pendrive", i18n("Transfer Plugin Options"));
}
