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
   connect(ui.enginesCheckBox, SIGNAL(clicked(bool)), SLOT(slotSetUseSearchEngines(bool)));
   connect(ui.urlAddButton, SIGNAL(clicked()), SLOT(slotAddUrl()));
};

dlgSettingsWidget::~dlgSettingsWidget()
{
   MultiSegKioSettings::writeConfig();
}

void dlgSettingsWidget::slotSetSegments(int seg)
{
   MultiSegKioSettings::setSegments(seg);
}

void dlgSettingsWidget::slotSetUseSearchEngines(bool)
{
   MultiSegKioSettings::setUseSearchEngines( ui.enginesCheckBox->isChecked() );
   ui.searchEngineGroupBox->setEnabled( ui.enginesCheckBox->isChecked() );
}

void dlgSettingsWidget::slotAddUrl()
{
   if(ui.engineNameLineEdit->text().isEmpty() || ui.urlLineEdit->text().isEmpty())
      return;

   addSearchEngineItem(ui.engineNameLineEdit->text(), ui.urlLineEdit->text());
   saveSearchEnginesSettings();
}

void dlgSettingsWidget::init()
{
   ui.numSegSpinBox->setValue(MultiSegKioSettings::segments());
   ui.enginesCheckBox->setChecked(MultiSegKioSettings::useSearchEngines());
   ui.searchEngineGroupBox->setEnabled( ui.enginesCheckBox->isChecked() );

   loadSearchEnginesSettings();
}

void dlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
   ui.enginesTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << name << url));
}

void dlgSettingsWidget::loadSearchEnginesSettings()
{
    QStringList enginesNames = MultiSegKioSettings::self()->findItem("SearchEnginesNameList")->property().toStringList();
    QStringList enginesUrls = MultiSegKioSettings::self()->findItem("SearchEnginesUrlList")->property().toStringList();

    for(int i = 0; i < enginesNames.size(); i++)
    {
        addSearchEngineItem(enginesNames[i], enginesUrls[i]);
    }
}

void dlgSettingsWidget::saveSearchEnginesSettings()
{
    QStringList enginesNames;
    QStringList enginesUrls;

    for(int i = 0; i < ui.enginesTreeWidget->topLevelItemCount(); i++)
    {
        enginesNames.append(ui.enginesTreeWidget->topLevelItem(i)->text(0));
        enginesUrls.append(ui.enginesTreeWidget->topLevelItem(i)->text(1));
    }

    MultiSegKioSettings::self()->findItem("SearchEnginesNameList")->setProperty(QVariant(enginesNames));
    MultiSegKioSettings::self()->findItem("SearchEnginesUrlList")->setProperty(QVariant(enginesUrls));

    MultiSegKioSettings::writeConfig();
}

#include "dlgmultisegkio.moc"
