/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmirrorsearch.h"

#include "kget_debug.h"
#include "mirrorsearchsettings.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

DlgEngineEditing::DlgEngineEditing(QWidget *parent)
    : QDialog(parent)
{
    auto *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    setWindowTitle(i18n("Insert Engine"));
    setModal(true);
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgEngineEditing::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgEngineEditing::reject);
    mainLayout->addWidget(buttonBox);

    ui.engineNameLabel->setText(i18n("Engine name:"));
    ui.urlLabel->setText(i18n("URL:"));
    connect(ui.urlEdit, &QLineEdit::textChanged, this, &DlgEngineEditing::slotChangeText);
    connect(ui.engineNameEdit, &QLineEdit::textChanged, this, &DlgEngineEditing::slotChangeText);
    slotChangeText();
}

DlgEngineEditing::~DlgEngineEditing()
{
}

void DlgEngineEditing::slotChangeText()
{
    okButton->setEnabled(!ui.urlEdit->text().isEmpty());
}

QString DlgEngineEditing::engineName() const
{
    return ui.engineNameEdit->text();
}

QString DlgEngineEditing::engineUrl() const
{
    return ui.urlEdit->text();
}

K_PLUGIN_CLASS(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    ui.setupUi(widget());
    ui.newEngineBt->setIcon(QIcon::fromTheme("list-add"));
    ui.removeEngineBt->setIcon(QIcon::fromTheme("list-remove"));

    connect(ui.newEngineBt, &QAbstractButton::clicked, this, &DlgSettingsWidget::slotNewEngine);
    connect(ui.removeEngineBt, &QAbstractButton::clicked, this, &DlgSettingsWidget::slotRemoveEngine);
}

DlgSettingsWidget::~DlgSettingsWidget()
{
}

void DlgSettingsWidget::slotNewEngine()
{
    DlgEngineEditing dialog;
    if (dialog.exec()) {
        addSearchEngineItem(dialog.engineName(), dialog.engineUrl());
        Q_EMIT markAsChanged();
    }
}

void DlgSettingsWidget::slotRemoveEngine()
{
    QList<QTreeWidgetItem *> selectedItems = ui.enginesTreeWidget->selectedItems();

    foreach (QTreeWidgetItem *selectedItem, selectedItems)
        delete (selectedItem);
    Q_EMIT markAsChanged();
}

void DlgSettingsWidget::load()
{
    loadSearchEnginesSettings();
}

void DlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
    ui.enginesTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << name << url));
    Q_EMIT markAsChanged();
}

void DlgSettingsWidget::loadSearchEnginesSettings()
{
    ui.enginesTreeWidget->clear(); // Cleanup things first

    QStringList enginesNames = MirrorSearchSettings::self()->searchEnginesNameList();
    QStringList enginesUrls = MirrorSearchSettings::self()->searchEnginesUrlList();

    for (int i = 0; i < enginesNames.size(); i++) {
        addSearchEngineItem(enginesNames[i], enginesUrls[i]);
    }
}

void DlgSettingsWidget::saveSearchEnginesSettings()
{
    QStringList enginesNames;
    QStringList enginesUrls;

    for (int i = 0; i < ui.enginesTreeWidget->topLevelItemCount(); i++) {
        enginesNames.append(ui.enginesTreeWidget->topLevelItem(i)->text(0));
        enginesUrls.append(ui.enginesTreeWidget->topLevelItem(i)->text(1));
    }

    MirrorSearchSettings::self()->setSearchEnginesNameList(enginesNames);
    MirrorSearchSettings::self()->setSearchEnginesUrlList(enginesUrls);

    MirrorSearchSettings::self()->save();
}

void DlgSettingsWidget::save()
{
    qCDebug(KGET_DEBUG);
    saveSearchEnginesSettings();

    MirrorSearchSettings::self()->save();
}

#include "dlgmirrorsearch.moc"
#include "moc_dlgmirrorsearch.cpp"
