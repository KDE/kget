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

BTSettingsWidget::BTSettingsWidget(QWidget * parent)
  : QWidget(parent),
    m_port(BittorrentSettings::port()),
    m_uploadLimit(BittorrentSettings::uploadLimit()),
    m_downloadLimit(BittorrentSettings::downloadLimit()),
    m_torrentDir(BittorrentSettings::torrentDir()),
    m_tmpDir(BittorrentSettings::tmpDir())
{
    setupUi(this);

    init();
}

void BTSettingsWidget::init()
{
    portBox->setValue(m_port);
    uploadBox->setValue(m_uploadLimit);
    downloadBox->setValue(m_downloadLimit);
    torrentEdit->setUrl(m_torrentDir);
    tempEdit->setUrl(m_tmpDir);

    connect(portBox, SIGNAL(valueChanged(int)), SLOT(setPort(int)));
    connect(uploadBox, SIGNAL(valueChanged(int)), SLOT(setUploadLimit(int)));
    connect(downloadBox, SIGNAL(valueChanged(int)), SLOT(setDownloadLimit(int)));
    connect(torrentEdit, SIGNAL(textChanged(QString)), SLOT(setDefaultTorrentDir(QString)));
    connect(tempEdit, SIGNAL(textChanged(QString)), SLOT(setDefaultTempDir(QString)));
}

void BTSettingsWidget::setPort(int port)
{
    m_port = port;
}

void BTSettingsWidget::setUploadLimit(int uploadLimit)
{
    m_uploadLimit = uploadLimit;
}

void BTSettingsWidget::setDownloadLimit(int downloadLimit)
{
    m_downloadLimit = downloadLimit;
}

void BTSettingsWidget::setDefaultTorrentDir(QString torrentDir)
{
    m_torrentDir = torrentDir;
}

void BTSettingsWidget::setDefaultTempDir(QString tmpDir)
{
    m_tmpDir = tmpDir;
}

void BTSettingsWidget::dialogAccepted()
{
    BittorrentSettings::setPort(m_port);
    BittorrentSettings::setUploadLimit(m_uploadLimit);
    BittorrentSettings::setDownloadLimit(m_downloadLimit);
    BittorrentSettings::setTorrentDir(m_torrentDir);
    BittorrentSettings::setTmpDir(m_tmpDir);

    BittorrentSettings::self()->writeConfig();
}

#include "btsettingswidget.moc"
 
