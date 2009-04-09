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
    setCaption(i18n("Group Settings for %1", group->name()));
    showButtonSeparator(true);

    QWidget *widget = new QWidget(this);

    Ui::GroupSettingsDialog ui;
    ui.setupUi(widget);

    setMainWidget(widget);

    m_downloadBox = ui.downloadBox;
    m_downloadBox->setValue(group->downloadLimit(Transfer::VisibleSpeedLimit));

    m_uploadBox = ui.uploadBox;
    m_uploadBox->setValue(group->uploadLimit(Transfer::VisibleSpeedLimit));

    m_downloadCheck = ui.downloadCheck;
    if (m_downloadBox->value() != 0)
        m_downloadCheck->setChecked(true);

    m_uploadCheck = ui.uploadCheck;
    if (m_uploadBox->value() != 0)
        m_uploadCheck->setChecked(true);

    m_defaultFolderRequester = ui.defaultFolderRequester;
    m_defaultFolderRequester->setMode(KFile::Directory);
    m_defaultFolderRequester->setPath(group->defaultFolder());

    m_regExpEdit = ui.regExpEdit;
    m_regExpEdit->setText(group->regExp().pattern());

    connect(this, SIGNAL(accepted()), SLOT(save()));
}

GroupSettingsDialog::~GroupSettingsDialog()
{
}

void GroupSettingsDialog::save()
{
    m_group->setDefaultFolder(m_defaultFolderRequester->url().toLocalFile());

    if (m_downloadCheck->isChecked())
        m_group->setDownloadLimit(m_downloadBox->value(), Transfer::VisibleSpeedLimit);
    else
        m_group->setDownloadLimit(0, Transfer::VisibleSpeedLimit);

    if (m_uploadCheck->isChecked())
        m_group->setUploadLimit(m_uploadBox->value(), Transfer::VisibleSpeedLimit);
    else
        m_group->setUploadLimit(0, Transfer::VisibleSpeedLimit);

    QRegExp regExp;
    regExp.setPattern(m_regExpEdit->text());
    m_group->setRegExp(regExp);
}

#include "groupsettingsdialog.moc"
