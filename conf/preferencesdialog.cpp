/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <klocale.h>

// reimplementing this
#include "preferencesdialog.h"

PreferencesDialog::PreferencesDialog( QWidget * parent, KConfigSkeleton * skeleton )
    : KConfigDialog( parent, "preferences", skeleton )
{
    appearance = new QWidget(this);
    network = new QWidget(this);
    directories = new QWidget(this);
    advanced = new QWidget(this);

    DlgAppearance dlgApp;
    DlgNetwork dlgNet;
    DlgDirectories dlgDir;
    DlgAdvanced dlgAdv;

    dlgApp.setupUi(appearance);
    dlgNet.setupUi(network);
    dlgDir.setupUi(directories);
    dlgAdv.setupUi(advanced);

    addPage( appearance, i18n("Appearance"), "looknfeel", i18n("Look and feel") );
    addPage( directories, i18n("Folders"), "folder_open", i18n("Default download folders") );
    addPage( network, i18n("Network"), "network", i18n("Network and downloads") );
    addPage( advanced, i18n("Advanced"), "exec", i18n("Advanced options") );
}
