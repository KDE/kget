/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btsettingswidget.h"

#include "bittorrentsettings.h"
#include <KPluginFactory>

#include "kget_debug.h"
#include <qobject.h>

K_PLUGIN_CLASS(BTSettingsWidget)

BTSettingsWidget::BTSettingsWidget(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    setupUi(widget());

    connect(portBox, &QSpinBox::valueChanged, this, &BTSettingsWidget::markAsChanged);
    connect(uploadBox, &QSpinBox::valueChanged, this, &BTSettingsWidget::markAsChanged);
    connect(downloadBox, &QSpinBox::valueChanged, this, &BTSettingsWidget::markAsChanged);
    connect(torrentEdit, &KUrlRequester::textChanged, this, &BTSettingsWidget::markAsChanged);
    connect(tempEdit, &KUrlRequester::textChanged, this, &BTSettingsWidget::markAsChanged);
    connect(preallocBox, &QCheckBox::checkStateChanged, this, &BTSettingsWidget::markAsChanged);
    connect(utpBox, &QCheckBox::checkStateChanged, this, &BTSettingsWidget::markAsChanged);
}

void BTSettingsWidget::load()
{
    torrentEdit->setMode(KFile::Directory);
    tempEdit->setMode(KFile::Directory);
    defaults();
}

void BTSettingsWidget::save()
{
    qCDebug(KGET_DEBUG) << "Save Bittorrent-config";
    BittorrentSettings::setPort(portBox->value());
    BittorrentSettings::setUploadLimit(uploadBox->value());
    BittorrentSettings::setDownloadLimit(downloadBox->value());
    BittorrentSettings::setTorrentDir(torrentEdit->url().url());
    BittorrentSettings::setTmpDir(tempEdit->url().url());
    BittorrentSettings::setPreAlloc(preallocBox->isChecked());
    BittorrentSettings::setEnableUTP(utpBox->isChecked());

    BittorrentSettings::self()->save();
}

void BTSettingsWidget::defaults()
{
    portBox->setValue(BittorrentSettings::port());
    uploadBox->setValue(BittorrentSettings::uploadLimit());
    downloadBox->setValue(BittorrentSettings::downloadLimit());
    torrentEdit->setUrl(QUrl::fromLocalFile(BittorrentSettings::torrentDir()));
    tempEdit->setUrl(QUrl::fromLocalFile(BittorrentSettings::tmpDir()));
    preallocBox->setChecked(BittorrentSettings::preAlloc());
    utpBox->setChecked(BittorrentSettings::enableUTP());
}

#include "btsettingswidget.moc"
#include "moc_btsettingswidget.cpp"
