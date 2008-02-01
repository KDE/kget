/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "groupsettingsdialog.h"

#include "core/transfergrouphandler.h"

GroupSettingsDialog::GroupSettingsDialog(QWidget *parent, TransferGroupHandler *group)
  : KDialog(parent),
    m_group(group)
{
    setCaption(i18n("Group-Settings for %1", group->name()));
    QWidget *widget = new QWidget(this);
    Ui::GroupSettingsDialog ui;
    ui.setupUi(widget);
    setMainWidget(widget);
    m_downloadBox = ui.downloadBox;
    m_downloadBox->setValue(group->visibleDownloadLimit());
    m_uploadBox = ui.uploadBox;
    m_uploadBox->setValue(group->visibleUploadLimit());
    m_downloadCheck = ui.downloadCheck;
    if (m_downloadBox->value() != 0)
        m_downloadCheck->setChecked(true);
    m_uploadCheck = ui.uploadCheck;
    if (m_uploadBox->value() != 0)
        m_uploadCheck->setChecked(true);
    m_defaultFolderRequester = ui.defaultFolderRequester;
    m_defaultFolderRequester->setMode(KFile::Directory);
    m_defaultFolderRequester->setPath(group->defaultFolder());
    connect(this, SIGNAL(accepted()), SLOT(save()));
}

GroupSettingsDialog::~GroupSettingsDialog()
{
}

void GroupSettingsDialog::save()
{
    m_group->setDefaultFolder(m_defaultFolderRequester->url().path());

    if (m_downloadCheck->isChecked())
        m_group->setVisibleDownloadLimit(m_downloadBox->value());
    else
        m_group->setVisibleDownloadLimit(0);

    if (m_uploadCheck->isChecked())
        m_group->setVisibleUploadLimit(m_uploadBox->value());
    else
        m_group->setVisibleUploadLimit(0);
}

#include "groupsettingsdialog.moc"
