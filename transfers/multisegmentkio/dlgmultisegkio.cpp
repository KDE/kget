/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include "dlgmultisegkio.h"
#include "MultiSegKioSettings.h"

dlgSettingsWidget::dlgSettingsWidget(QWidget *parent)
: QWidget(parent)
{
   ui.setupUi(this);
   init();
   connect(ui.numSegSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSetSegments(int)));
   connect(ui.checkBox, SIGNAL(clicked(bool)), SLOT(slotSetUseSearchEngines(bool)));
};

dlgSettingsWidget::~dlgSettingsWidget()
{
   MultiSegKioSettings::writeConfig();
}

void dlgSettingsWidget::init()
{
   ui.numSegSpinBox->setValue(MultiSegKioSettings::segments());
   ui.checkBox->setChecked(MultiSegKioSettings::useSearchEngines());
   ui.searchEngineGroupBox->setEnabled( ui.checkBox->isChecked() );
}

void dlgSettingsWidget::slotSetSegments(int seg)
{
   MultiSegKioSettings::setSegments(seg);
}

void dlgSettingsWidget::slotSetUseSearchEngines(bool)
{
   MultiSegKioSettings::setUseSearchEngines( ui.checkBox->isChecked() );
   ui.searchEngineGroupBox->setEnabled( ui.checkBox->isChecked() );
}

#include "dlgmultisegkio.moc"
