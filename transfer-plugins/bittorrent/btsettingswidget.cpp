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
{
    setupUi(this);

    setDefault();

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

    BittorrentSettings::self()->writeConfig();
}

void BTSettingsWidget::setDefault()
{
    portBox->setValue(BittorrentSettings::port());
    uploadBox->setValue(BittorrentSettings::uploadLimit());
    downloadBox->setValue(BittorrentSettings::downloadLimit());
    torrentEdit->setUrl(BittorrentSettings::torrentDir());
    tempEdit->setUrl(BittorrentSettings::tmpDir());
    preallocBox->setChecked(BittorrentSettings::preAlloc());
}

#include "btsettingswidget.moc"
 
