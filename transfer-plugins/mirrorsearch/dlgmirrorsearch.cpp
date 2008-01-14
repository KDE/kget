/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmirrorsearch.h"

#include "mirrorsearchsettings.h"

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
    connect(ui.urlEdit,SIGNAL(textChanged(const QString &)), SLOT(slotChangeText()));
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

QString DlgEngineEditing::engineName() const
{
    return ui.engineNameEdit->text();
}

QString DlgEngineEditing::engineUrl() const
{
    return ui.urlEdit->text();
}

DlgSettingsWidget::DlgSettingsWidget(KDialog *parent)
    : QWidget(parent),
      m_parent(parent)
{
    ui.setupUi(this);
    ui.newEngineBt->setIcon(KIcon("list-add"));
    ui.removeEngineBt->setIcon(KIcon("list-remove"));

    init();

    connect(ui.newEngineBt, SIGNAL(clicked()), SLOT(slotNewEngine()));
    connect(ui.removeEngineBt, SIGNAL(clicked()), SLOT(slotRemoveEngine()));
    connect(parent, SIGNAL(accepted()), SLOT(slotSave()));
    connect(parent, SIGNAL(rejected()), SLOT(init()));
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::slotNewEngine()
{
    DlgEngineEditing dialog;
    if(dialog.exec())
        addSearchEngineItem(dialog.engineName(), dialog.engineUrl());
}

void DlgSettingsWidget::slotRemoveEngine()
{
    QList<QTreeWidgetItem *> selectedItems = ui.enginesTreeWidget->selectedItems();

    foreach(QTreeWidgetItem * selectedItem, selectedItems)
        delete(selectedItem);
}

void DlgSettingsWidget::init()
{
    loadSearchEnginesSettings();
}

void DlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
    ui.enginesTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << name << url));
}

void DlgSettingsWidget::loadSearchEnginesSettings()
{
    ui.enginesTreeWidget->clear();//Cleanup things first

    QStringList enginesNames = MirrorSearchSettings::self()->findItem("SearchEnginesNameList")->property().toStringList();
    QStringList enginesUrls = MirrorSearchSettings::self()->findItem("SearchEnginesUrlList")->property().toStringList();

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

    MirrorSearchSettings::self()->findItem("SearchEnginesNameList")->setProperty(QVariant(enginesNames));
    MirrorSearchSettings::self()->findItem("SearchEnginesUrlList")->setProperty(QVariant(enginesUrls));

    MirrorSearchSettings::self()->writeConfig();
}

void DlgSettingsWidget::slotSave()
{
    kDebug(5001);
    saveSearchEnginesSettings();

    MirrorSearchSettings::self()->writeConfig();
}

#include "dlgmirrorsearch.moc"
