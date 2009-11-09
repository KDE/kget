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

#include "ui_dlgappearance.h"
#include "ui_dlgnetwork.h"
#include "dlgwebinterface.h"

#include "transfersgroupwidget.h"
#include "pluginselector.h"
#include "verificationpreferences.h"

#include <klocale.h>
#include <ktabwidget.h>

PreferencesDialog::PreferencesDialog(QWidget * parent, KConfigSkeleton * skeleton)
    : KConfigDialog(parent, "preferences", skeleton)
{
    QWidget *appearance = new QWidget(this);
    QWidget *groups = new QWidget(this);
    DlgWebinterface *webinterface = new DlgWebinterface(this);
    QWidget *network = new QWidget(this);
    QWidget *advanced = new QWidget(this);
    VerificationPreferences *verification = new VerificationPreferences(this);
    connect(verification, SIGNAL(changed()), SLOT(enableApplyButton()));
    PluginSelector * pluginSelector = new PluginSelector(this);
    connect(pluginSelector, SIGNAL(changed(bool)), SLOT(enableApplyButton()));

    groups->setLayout(new TransfersGroupWidget());

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
#ifdef HAVE_NEPOMUK
    dlgAdv.kcfg_HistoryBackend->addItem(i18n("Nepomuk"), QVariant(TransferHistoryStore::Nepomuk));
#endif

#ifdef HAVE_KWORKSPACE
    dlgAdv.kcfg_AfterFinishAction->addItem(i18n("Turn Off Computer"), QVariant(KGet::Shutdown));
#endif

    // enable or disable the AfterFinishAction depends on the AfterFinishActionEnabled checkbox state
    dlgAdv.kcfg_AfterFinishAction->setEnabled(dlgAdv.kcfg_AfterFinishActionEnabled->checkState () == Qt::Checked);
    connect(dlgAdv.kcfg_AfterFinishActionEnabled, SIGNAL(stateChanged(int)),
                                                  SLOT(slotToggleAfterFinishAction(int)));

    // TODO: remove the following lines as soon as these features are ready
    dlgNet.lb_per_transfer->setVisible(false);
    dlgNet.kcfg_TransferSpeedLimit->setVisible(false);
    dlgNet.groupBoxCompleted->setVisible(false);
    dlgNet.lbl_maxnum_2->setVisible(false);
    dlgNet.kcfg_MaxConnectionsServer->setVisible(false);

    addPage(appearance, i18n("Appearance"), "preferences-desktop-theme", i18n("Change appearance settings"));
    addPage(groups, i18n("Groups"), "bookmarks", i18n("Manage the groups"));
    addPage(network, i18n("Network"), "network-workgroup", i18n("Network and Downloads"));
    addPage(webinterface, i18n("Web Interface"), "network-workgroup", i18n("Control KGet over a Network or the Internet"));
    addPage(verification, i18n("Verification"), "document-encrypt", i18n("Verification"));
    addPage(advanced, i18nc("Advanced Options", "Advanced"), "preferences-other", i18n("Advanced Options"));
    addPage(pluginSelector, i18n("Plugins"), "preferences-plugin", i18n("Transfer Plugins"));

    connect(this, SIGNAL(accepted()), SLOT(disableApplyButton()));
    connect(this, SIGNAL(rejected()), SLOT(disableApplyButton()));
}

void PreferencesDialog::disableApplyButton()
{
    enableButtonApply(false);
}

void PreferencesDialog::enableApplyButton()
{
    enableButtonApply(true);
}

void PreferencesDialog::slotToggleAfterFinishAction(int state)
{
    dlgAdv.kcfg_AfterFinishAction->setEnabled(state == Qt::Checked);
}
