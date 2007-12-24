/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btsettingswidget.h"
#include "bittorrentsettings.h"

#include <kdebug.h>

BTSettingsWidget::BTSettingsWidget(KDialog * parent)
  : QWidget(parent),
    m_parent(parent)
{
    setupUi(this);

    setDefault();

    connect(portBox, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
    connect(uploadBox, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
    connect(downloadBox, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
    connect(preallocBox, SIGNAL(stateChanged(int)), SLOT(enableButtonApply()));
    connect(sharedRatioSpin, SIGNAL(valueChanged(int)), SLOT(enableButtonApply()));
    connect(torrentEdit, SIGNAL(textChanged(QString)), SLOT(enableButtonApply()));
    connect(tempEdit, SIGNAL(textChanged(QString)), SLOT(enableButtonApply()));
    connect(parent, SIGNAL(accepted()), SLOT(dialogAccepted()));
    connect(parent, SIGNAL(rejected()), SLOT(setDefault()));
}

void BTSettingsWidget::dialogAccepted()
{
    kDebug(5001) << "Save Bittorrent-config";
    BittorrentSettings::setPort(portBox->value());
    BittorrentSettings::setUploadLimit(uploadBox->value());
    BittorrentSettings::setDownloadLimit(downloadBox->value());
    BittorrentSettings::setTorrentDir(torrentEdit->url().url());
    BittorrentSettings::setTmpDir(tempEdit->url().url());
    BittorrentSettings::setPreAlloc(preallocBox->isChecked());
    BittorrentSettings::setMaxSharedRatio(sharedRatioSpin->value());

    BittorrentSettings::self()->writeConfig();
}

void BTSettingsWidget::setDefault()
{
    portBox->setValue(BittorrentSettings::port());
    uploadBox->setValue(BittorrentSettings::uploadLimit());
    downloadBox->setValue(BittorrentSettings::downloadLimit());
    sharedRatioSpin->setValue(BittorrentSettings::maxSharedRatio());
    torrentEdit->setUrl(BittorrentSettings::torrentDir());
    tempEdit->setUrl(BittorrentSettings::tmpDir());
    preallocBox->setChecked(BittorrentSettings::preAlloc());
}

void BTSettingsWidget::enableButtonApply()
{
    m_parent->enableButtonApply(true);
}

#include "btsettingswidget.moc"
 
