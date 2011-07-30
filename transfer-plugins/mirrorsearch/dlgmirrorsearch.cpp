/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmirrorsearch.h"

#include "kget_export.h"
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
    ui.urlLabel->setText(i18n("URL:"));
    connect(ui.urlEdit,SIGNAL(textChanged(QString)), SLOT(slotChangeText()));
    connect(ui.engineNameEdit,SIGNAL(textChanged(QString)),SLOT(slotChangeText()));
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

KGET_EXPORT_PLUGIN_CONFIG(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QWidget *parent, const QVariantList &args)
    : KCModule(KGetFactory::componentData(), parent, args)
{
    ui.setupUi(this);
    ui.newEngineBt->setIcon(KIcon("list-add"));
    ui.removeEngineBt->setIcon(KIcon("list-remove"));

    connect(ui.newEngineBt, SIGNAL(clicked()), SLOT(slotNewEngine()));
    connect(ui.removeEngineBt, SIGNAL(clicked()), SLOT(slotRemoveEngine()));
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::slotNewEngine()
{
    DlgEngineEditing dialog;
    if(dialog.exec()) {
        addSearchEngineItem(dialog.engineName(), dialog.engineUrl());
        changed();
    }
}

void DlgSettingsWidget::slotRemoveEngine()
{
    QList<QTreeWidgetItem *> selectedItems = ui.enginesTreeWidget->selectedItems();

    foreach(QTreeWidgetItem * selectedItem, selectedItems)
        delete(selectedItem);

    changed();
}

void DlgSettingsWidget::load()
{
    loadSearchEnginesSettings();
}

void DlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
    ui.enginesTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << name << url));
    changed();
}

void DlgSettingsWidget::loadSearchEnginesSettings()
{
    ui.enginesTreeWidget->clear();//Cleanup things first

    QStringList enginesNames = MirrorSearchSettings::self()->searchEnginesNameList();
    QStringList enginesUrls = MirrorSearchSettings::self()->searchEnginesUrlList();

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

    MirrorSearchSettings::self()->setSearchEnginesNameList(enginesNames);
    MirrorSearchSettings::self()->setSearchEnginesUrlList(enginesUrls);

    MirrorSearchSettings::self()->writeConfig();
}

void DlgSettingsWidget::save()
{
    kDebug(5001);
    saveSearchEnginesSettings();

    MirrorSearchSettings::self()->writeConfig();
}

#include "dlgmirrorsearch.moc"
