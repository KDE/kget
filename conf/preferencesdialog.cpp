/* This file is part of the KDE project
   Copyright (C) 2004 KGet Developers < >

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <klocale.h>

// single config pages
#include "dlgappearance.h"
#include "dlgnetwork.h"
#include "dlgdirectories.h"
#include "dlgadvanced.h"

// reimplementing this
#include "preferencesdialog.h"

PreferencesDialog::PreferencesDialog( QWidget * parent, KConfigSkeleton * skeleton )
    : KConfigDialog( parent, "preferences", skeleton )
{
    appearance = new DlgAppearance(0);
    network = new DlgNetwork(0);
    directories = new DlgDirectories(0);
    advanced = new DlgAdvanced(0);
     
    addPage( appearance, i18n("Appearance"), "looknfeel", i18n("Look and feel") );
    addPage( directories, i18n("Directories"), "folder_open", i18n("Default download directories") );
    addPage( network, i18n("Network"), "network", i18n("Network and downloads") );
    addPage( advanced, i18n("Advanced"), "exec", i18n("Advanced options") );
}
