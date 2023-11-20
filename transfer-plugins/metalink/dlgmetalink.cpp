/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmetalink.h"

#include "metalinksettings.h"

#include "kget_macro.h"
#include <kpluginmetadata.h>

KGET_EXPORT_PLUGIN_CONFIG(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());

    connect(ui.numSimultaneousFiles, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui.kcfg_MirrorsPerFile, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(ui.kcfg_ConnectionsPerUrl, SIGNAL(valueChanged(int)), SLOT(changed()));
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::load()
{
    ui.numSimultaneousFiles->setValue(MetalinkSettings::simultaneousFiles());
    ui.kcfg_MirrorsPerFile->setValue(MetalinkSettings::mirrorsPerFile());
    ui.kcfg_ConnectionsPerUrl->setValue(MetalinkSettings::connectionsPerUrl());
}

void DlgSettingsWidget::save()
{
    MetalinkSettings::setSimultaneousFiles(ui.numSimultaneousFiles->value());
    MetalinkSettings::setMirrorsPerFile(ui.kcfg_MirrorsPerFile->value());
    MetalinkSettings::setConnectionsPerUrl(ui.kcfg_ConnectionsPerUrl->value());

    MetalinkSettings::self()->save();
}

#include "dlgmetalink.moc"
#include "moc_dlgmetalink.cpp"
