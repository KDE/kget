/* This file is part of the KDE project

   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmultisegkio.h"

#include "multisegkiosettings.h"

DlgEngineEditing::DlgEngineEditing(QWidget *parent)
    : KDialog(parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget(mainWidget);

    setWindowTitle(i18n("Insert Engine"));
    setModal(true);
    setButtons(KDialog::Ok | KDialog::Cancel);
    showButtonSeparator(true);

    ui.engineNameLabel->setText(i18n("Engine name:"));
    ui.urlLabel->setText(i18n("Url:"));
    connect(ui.urlEdit,SIGNAL(textChanged(const QString &)),SLOT(slotChangeText()));
    connect(ui.engineNameEdit,SIGNAL(textChanged(const QString &)),SLOT(slotChangeText()));
    slotChangeText();
}

DlgEngineEditing::~DlgEngineEditing()
{
}

void DlgEngineEditing::slotChangeText()
{
  enableButton(KDialog::Ok, !ui.urlEdit->text().isEmpty());
}

DlgSettingsWidget::DlgSettingsWidget(KDialog *parent)
    : QWidget(parent),
    m_segments(0),
    m_minsegsize(0),
    m_savesegsize(0),
    m_searchengines(false)
{
    ui.setupUi(this);
    ui.newEngineBt->setIcon(KIcon("list-add"));
    ui.removeEngineBt->setIcon(KIcon("list-remove"));

    init();
    connect(ui.numSegSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSetSegments(int)));
    connect(ui.minSegSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSetMinSegSize(int)));
    connect(ui.saveDataSizeSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSetSaveDataSize(int)));
    connect(ui.enginesCheckBox, SIGNAL(clicked(bool)), SLOT(slotSetUseSearchEngines(bool)));
    connect(ui.newEngineBt, SIGNAL(clicked()), SLOT(slotNewEngine()));
    connect(ui.removeEngineBt, SIGNAL(clicked()), SLOT(slotRemoveEngine()));
    connect(parent, SIGNAL(accepted()), SLOT(slotSave()));
    connect(parent, SIGNAL(rejected()), SLOT(init()));
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

QString DlgEngineEditing::engineName() const
{
    return ui.engineNameEdit->text();
}

QString DlgEngineEditing::engineUrl() const
{
    return ui.urlEdit->text();
}

void DlgSettingsWidget::slotSetSegments(int seg)
{
     m_segments = seg;
}

void DlgSettingsWidget::slotSetMinSegSize(int size)
{
    m_minsegsize = size;
}

void DlgSettingsWidget::slotSetSaveDataSize(int size)
{
    m_savesegsize = size;
}

void DlgSettingsWidget::slotSetUseSearchEngines(bool)
{
    m_searchengines = ui.enginesCheckBox->isChecked();
    ui.searchEngineGroupBox->setEnabled(ui.enginesCheckBox->isChecked());
}

void DlgSettingsWidget::slotNewEngine()
{
    DlgEngineEditing dialog;
    if(dialog.exec())
        addSearchEngineItem(dialog.engineName(), dialog.engineUrl());

    saveSearchEnginesSettings();
}

void DlgSettingsWidget::slotRemoveEngine()
{
    QList<QTreeWidgetItem *> selectedItems = ui.enginesTreeWidget->selectedItems();

    foreach(QTreeWidgetItem * selectedItem, selectedItems)
        delete(selectedItem);

    saveSearchEnginesSettings();
}

void DlgSettingsWidget::init()
{
    ui.numSegSpinBox->setValue( MultiSegKioSettings::segments() );
    ui.minSegSizeSpinBox->setValue( MultiSegKioSettings::splitSize() );
    ui.saveDataSizeSpinBox->setValue( MultiSegKioSettings::saveSegSize() );

    ui.enginesCheckBox->setChecked(MultiSegKioSettings::useSearchEngines());
    ui.searchEngineGroupBox->setEnabled( ui.enginesCheckBox->isChecked() );

    loadSearchEnginesSettings();
}

void DlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
    ui.enginesTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << name << url));
}

void DlgSettingsWidget::loadSearchEnginesSettings()
{
    QStringList enginesNames = MultiSegKioSettings::self()->findItem("SearchEnginesNameList")->property().toStringList();
    QStringList enginesUrls = MultiSegKioSettings::self()->findItem("SearchEnginesUrlList")->property().toStringList();

    for(int i = 0; i < enginesNames.size(); i++)
    {
        addSearchEngineItem(enginesNames[i], enginesUrls[i]);
    }
}

void DlgSettingsWidget::saveSearchEnginesSettings()
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

    MultiSegKioSettings::self()->writeConfig();
}

void DlgSettingsWidget::slotSave()
{
    kDebug(5001) << "Saving Multithreaded config";
    MultiSegKioSettings::setSegments(m_segments);
    MultiSegKioSettings::setSplitSize(m_minsegsize);
    MultiSegKioSettings::setSaveSegSize(m_savesegsize);
    MultiSegKioSettings::setUseSearchEngines(m_searchengines);
    MultiSegKioSettings::self()->writeConfig();
}

#include "dlgmultisegkio.moc"
