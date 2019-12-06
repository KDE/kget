/* This file is part of the KDE project

   Copyright (C) 2008 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "dlgmirrorsearch.h"

#include "kget_macro.h"
#include "kget_debug.h"
#include "mirrorsearchsettings.h"

#include <KLocalizedString>
#include <KConfigGroup>
#include <kconfigwidgets_version.h>

#include <QDebug>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

DlgEngineEditing::DlgEngineEditing(QWidget *parent)
    : QDialog(parent)
{
    QWidget *mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    setWindowTitle(i18n("Insert Engine"));
    setModal(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DlgEngineEditing::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DlgEngineEditing::reject);
    mainLayout->addWidget(buttonBox);

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

KGET_EXPORT_PLUGIN_CONFIG(DlgSettingsWidget)

DlgSettingsWidget::DlgSettingsWidget(QWidget *parent, const QVariantList &args)
    : KCModule(/*KGetFactory::componentData(),*/ parent, args)
{
    ui.setupUi(this);
    ui.newEngineBt->setIcon(QIcon::fromTheme("list-add"));
    ui.removeEngineBt->setIcon(QIcon::fromTheme("list-remove"));

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
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 64, 0)
        emit changed();
#else
        emit markAsChanged();
#endif
    }
}

void DlgSettingsWidget::slotRemoveEngine()
{
    QList<QTreeWidgetItem *> selectedItems = ui.enginesTreeWidget->selectedItems();

    foreach(QTreeWidgetItem * selectedItem, selectedItems)
        delete(selectedItem);
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 64, 0)
        emit changed();
#else
        emit markAsChanged();
#endif
}

void DlgSettingsWidget::load()
{
    loadSearchEnginesSettings();
}

void DlgSettingsWidget::addSearchEngineItem(const QString &name, const QString &url)
{
    ui.enginesTreeWidget->addTopLevelItem(new QTreeWidgetItem(QStringList() << name << url));
#if KCONFIGWIDGETS_VERSION < QT_VERSION_CHECK(5, 64, 0)
        emit changed();
#else
        emit markAsChanged();
#endif
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

    MirrorSearchSettings::self()->save();
}

void DlgSettingsWidget::save()
{
    qCDebug(KGET_DEBUG);
    saveSearchEnginesSettings();

    MirrorSearchSettings::self()->save();
}

#include "dlgmirrorsearch.moc"
