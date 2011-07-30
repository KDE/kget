/* This file is part of the KDE project

   Copyright (C) 2007 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "btsettingswidget.h"

#include "kget_export.h"
#include "bittorrentsettings.h"

#include <kdebug.h>
#include <kfiledialog.h>

KGET_EXPORT_PLUGIN_CONFIG(BTSettingsWidget)

BTSettingsWidget::BTSettingsWidget(QWidget * parent = 0, const QVariantList &args = QVariantList())
  : KCModule(KGetFactory::componentData(), parent, args)
{
    setupUi(this);

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
    torrentEdit->fileDialog()->setCaption(i18n("Select a default torrent folder"));
    tempEdit->setMode(KFile::Directory);
    tempEdit->fileDialog()->setCaption(i18n("Select a default temporary folder"));
    defaults();
}

void BTSettingsWidget::save()
{
    kDebug(5001) << "Save Bittorrent-config";
    BittorrentSettings::setPort(portBox->value());
    BittorrentSettings::setUploadLimit(uploadBox->value());
    BittorrentSettings::setDownloadLimit(downloadBox->value());
    BittorrentSettings::setTorrentDir(torrentEdit->url().url());
    BittorrentSettings::setTmpDir(tempEdit->url().url());
    BittorrentSettings::setPreAlloc(preallocBox->isChecked());
    BittorrentSettings::setEnableUTP(utpBox->isChecked());

    BittorrentSettings::self()->writeConfig();
}

void BTSettingsWidget::defaults()
{
    portBox->setValue(BittorrentSettings::port());
    uploadBox->setValue(BittorrentSettings::uploadLimit());
    downloadBox->setValue(BittorrentSettings::downloadLimit());
    torrentEdit->setUrl(BittorrentSettings::torrentDir());
    tempEdit->setUrl(BittorrentSettings::tmpDir());
    preallocBox->setChecked(BittorrentSettings::preAlloc());
    utpBox->setChecked(BittorrentSettings::enableUTP());
}

#include "btsettingswidget.moc"
 
