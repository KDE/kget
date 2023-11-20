/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btsettingswidget.h"

#include "bittorrentsettings.h"
#include "kget_macro.h"

#include "kget_debug.h"
#include <qobject.h>

KGET_EXPORT_PLUGIN_CONFIG(BTSettingsWidget)

BTSettingsWidget::BTSettingsWidget(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    setupUi(widget());

    connect(portBox, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(uploadBox, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(downloadBox, SIGNAL(valueChanged(int)), SLOT(changed()));
    connect(torrentEdit, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(tempEdit, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(preallocBox, SIGNAL(stateChanged(int)), SLOT(changed()));
    connect(utpBox, SIGNAL(stateChanged(int)), SLOT(changed()));
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
