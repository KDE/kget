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

BTSettingsWidget::BTSettingsWidget()
{
    setupUi(this);

    connect(portBox, SIGNAL(valueChanged(int)), SLOT(setPort(int)));
    connect(uploadBox, SIGNAL(valueChanged(int)), SLOT(setUploadRate(int)));
    connect(downloadBox, SIGNAL(valueChanged(int)), SLOT(setDownloadRate(int)));
    connect(torrentEdit, SIGNAL(textChanged(QString)), SLOT(setDefaultTorrentDir(QString)));
    connect(tempEdit, SIGNAL(textChanged(QString)), SLOT(setDefaultTempDir(QString)));
}

void BTSettingsWidget::setPort(int port)
{
    BittorrentSettings::setPort(port);
}

void BTSettingsWidget::setUploadRate(int uploadRate)
{
    BittorrentSettings::setUploadRate(uploadRate);
}

void BTSettingsWidget::setDownloadRate(int downloadRate)
{
    BittorrentSettings::setDownloadRate(downloadRate);
}

void BTSettingsWidget::setDefaultTorrentDir(QString torrentDir)
{
    BittorrentSettings::setTorrentDir(torrentDir);
}

void BTSettingsWidget::setDefaultTempDir(QString tmpDir)
{
    BittorrentSettings::setTmpDir(tmpDir);
}

#include "btsettingswidget.moc"
 
