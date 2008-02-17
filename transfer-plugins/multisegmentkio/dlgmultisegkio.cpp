/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmultisegkio.h"

#include "multisegkiosettings.h"

DlgSettingsWidget::DlgSettingsWidget(KDialog *parent)
    : QWidget(parent),
      m_parent(parent)
{
    ui.setupUi(this);

    init();

    connect(parent, SIGNAL(accepted()), SLOT(slotSave()));
    connect(parent, SIGNAL(rejected()), SLOT(init()));

    connect(ui.numSegSpinBox, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
    connect(ui.minSegSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
    connect(ui.saveDataSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::init()
{
    ui.numSegSpinBox->setValue( MultiSegKioSettings::segments() );
    ui.minSegSizeSpinBox->setValue( MultiSegKioSettings::splitSize() );
    ui.saveDataSizeSpinBox->setValue( MultiSegKioSettings::saveSegSize() );

    ui.enginesCheckBox->setChecked(MultiSegKioSettings::useSearchEngines());
}

void DlgSettingsWidget::slotSave()
{
    kDebug(5001) << "Saving Multithreaded config";
    MultiSegKioSettings::setSegments(ui.numSegSpinBox->value());
    MultiSegKioSettings::setSplitSize(ui.minSegSizeSpinBox->value());
    MultiSegKioSettings::setSaveSegSize(ui.saveDataSizeSpinBox->value());
    MultiSegKioSettings::setUseSearchEngines(ui.enginesCheckBox->isChecked());

    MultiSegKioSettings::self()->writeConfig();
}

void DlgSettingsWidget::enableButtonApply()
{
    m_parent->enableButtonApply(true);
}

#include "dlgmultisegkio.moc"
