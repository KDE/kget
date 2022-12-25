/* This file is part of the KDE project
   Copyright (C) 2004 - 2007 KGet Developers <kget@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "preferencesdialog.h"
#include "core/kget.h"
#include "core/transferhistorystore.h"

#include "dlgwebinterface.h"
#include "ui_dlgappearance.h"
#include "ui_dlgnetwork.h"

#include "integrationpreferences.h"
#include "pluginselector.h"
#include "transfersgroupwidget.h"
#include "verificationpreferences.h"

#include <KConfigSkeleton>
#include <KLocalizedString>

PreferencesDialog::PreferencesDialog(QWidget *parent, KConfigSkeleton *skeleton)
    : KConfigDialog(parent, "preferences", skeleton)
{
    auto *appearance = new QWidget(this);
    auto *groups = new TransfersGroupWidget(this);
    auto *webinterface = new DlgWebinterface(this);
    connect(webinterface, &DlgWebinterface::changed, this, &PreferencesDialog::enableApplyButton);
    connect(webinterface, &DlgWebinterface::saved, this, &PreferencesDialog::settingsChangedSlot);
    auto *network = new QWidget(this);
    auto *advanced = new QWidget(this);
    auto *integration = new IntegrationPreferences(this);
    connect(integration, &IntegrationPreferences::changed, this, &PreferencesDialog::enableApplyButton);
    auto *verification = new VerificationPreferences(this);
    connect(verification, &VerificationPreferences::changed, this, &PreferencesDialog::enableApplyButton);
    auto *pluginSelector = new PluginSelector(this);
    connect(pluginSelector, &PluginSelector::changed, this, &PreferencesDialog::enableApplyButton);

    Ui::DlgAppearance dlgApp;
    Ui::DlgNetwork dlgNet;

    dlgApp.setupUi(appearance);
    dlgNet.setupUi(network);
    dlgAdv.setupUi(advanced);

    // history backend entries
    dlgAdv.kcfg_HistoryBackend->addItem(i18n("Xml"), QVariant(TransferHistoryStore::Xml));
#ifdef HAVE_SQLITE
    dlgAdv.kcfg_HistoryBackend->addItem(i18n("Sqlite"), QVariant(TransferHistoryStore::SQLite));
#endif

#ifdef HAVE_KWORKSPACE
    dlgAdv.kcfg_AfterFinishAction->addItem(i18n("Turn Off Computer"), QVariant(KGet::Shutdown));
    dlgAdv.kcfg_AfterFinishAction->addItem(i18n("Hibernate Computer"), QVariant(KGet::Hibernate));
    dlgAdv.kcfg_AfterFinishAction->addItem(i18n("Suspend Computer"), QVariant(KGet::Suspend));
#endif

    // enable or disable the AfterFinishAction depends on the AfterFinishActionEnabled checkbox state
    dlgAdv.kcfg_AfterFinishAction->setEnabled(dlgAdv.kcfg_AfterFinishActionEnabled->checkState() == Qt::Checked);
    connect(dlgAdv.kcfg_AfterFinishActionEnabled, &QCheckBox::stateChanged, this, &PreferencesDialog::slotToggleAfterFinishAction);

    // TODO: remove the following lines as soon as these features are ready
    dlgNet.lb_per_transfer->setVisible(false);
    dlgNet.kcfg_TransferSpeedLimit->setVisible(false);

    addPage(appearance, i18n("Appearance"), "preferences-desktop-theme", i18n("Change appearance settings"));
    addPage(groups, i18n("Groups"), "bookmarks", i18n("Manage the groups"));
    addPage(network, i18n("Network"), "network-workgroup", i18n("Network and Downloads"));
    addPage(webinterface, i18n("Web Interface"), "network-workgroup", i18n("Control KGet over a Network or the Internet"));
    addPage(verification, i18n("Verification"), "document-encrypt", i18n("Verification"));
    addPage(integration,
            i18nc("integration of KGet with other applications", "Integration"),
            "konqueror",
            i18nc("integration of KGet with other applications", "Integration"));
    addPage(advanced, i18nc("Advanced Options", "Advanced"), "preferences-other", i18n("Advanced Options"));
    addPage(pluginSelector, i18n("Plugins"), "preferences-plugin", i18n("Transfer Plugins"));

    connect(this, &PreferencesDialog::accepted, this, &PreferencesDialog::disableApplyButton);
    connect(this, &PreferencesDialog::rejected, this, &PreferencesDialog::disableApplyButton);
}

void PreferencesDialog::disableApplyButton()
{
    button(QDialogButtonBox::Apply)->setEnabled(false);
}

void PreferencesDialog::enableApplyButton()
{
    button(QDialogButtonBox::Apply)->setEnabled(true);
}

void PreferencesDialog::slotToggleAfterFinishAction(int state)
{
    dlgAdv.kcfg_AfterFinishAction->setEnabled(state == Qt::Checked);
}

void PreferencesDialog::updateWidgetsDefault()
{
    Q_EMIT resetDefaults();
    KConfigDialog::updateWidgetsDefault();
}
