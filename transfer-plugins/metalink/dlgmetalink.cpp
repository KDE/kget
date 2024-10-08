/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmetalink.h"

#include "metalinksettings.h"

#include <KPluginFactory>
#include <KPluginMetaData>

K_PLUGIN_CLASS(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());

    connect(ui.numSimultaneousFiles, &QSpinBox::valueChanged, this, &DlgSettingsWidget::markAsChanged);
    connect(ui.kcfg_MirrorsPerFile, &QSpinBox::valueChanged, this, &DlgSettingsWidget::markAsChanged);
    connect(ui.kcfg_ConnectionsPerUrl, &QSpinBox::valueChanged, this, &DlgSettingsWidget::markAsChanged);
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
