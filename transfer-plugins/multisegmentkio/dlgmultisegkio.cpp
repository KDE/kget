/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmultisegkio.h"

#include "multisegkiosettings.h"

#include <KPluginFactory>

K_PLUGIN_CLASS(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());

    connect(ui.numSegSpinBox, &QSpinBox::valueChanged, this, &DlgSettingsWidget::markAsChanged);
    connect(ui.enginesCheckBox, &QCheckBox::clicked, this, &DlgSettingsWidget::markAsChanged);
    connect(ui.verificationCheckBox, &QCheckBox::clicked, this, &DlgSettingsWidget::markAsChanged);
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::load()
{
    ui.numSegSpinBox->setValue(MultiSegKioSettings::segments());

    ui.enginesCheckBox->setChecked(MultiSegKioSettings::useSearchEngines());
    ui.verificationCheckBox->setChecked(MultiSegKioSettings::useSearchVerification());
}

void DlgSettingsWidget::save()
{
    MultiSegKioSettings::setSegments(ui.numSegSpinBox->value());
    MultiSegKioSettings::setUseSearchEngines(ui.enginesCheckBox->isChecked());
    MultiSegKioSettings::setUseSearchVerification(ui.verificationCheckBox->isChecked());

    MultiSegKioSettings::self()->save();
}

#include "dlgmultisegkio.moc"
#include "moc_dlgmultisegkio.cpp"
