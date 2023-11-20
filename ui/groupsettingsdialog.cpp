/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "groupsettingsdialog.h"

#include "core/transfergrouphandler.h"

GroupSettingsDialog::GroupSettingsDialog(QWidget *parent, TransferGroupHandler *group)
    : KGetSaveSizeDialog("GroupSettingsDialog", parent)
    , m_group(group)
{
    setWindowTitle(i18n("Group Settings for %1", group->name()));

    ui.setupUi(this);

    ui.downloadBox->setValue(group->downloadLimit(Transfer::VisibleSpeedLimit));
    ui.uploadBox->setValue(group->uploadLimit(Transfer::VisibleSpeedLimit));

    ui.defaultFolderRequester->setMode(KFile::Directory);
    QString path = group->defaultFolder();
    ui.defaultFolderRequester->setUrl(QUrl::fromLocalFile(path));
    ui.defaultFolderRequester->setStartDir(QUrl(KGet::generalDestDir(true)));

    ui.regExpEdit->setText(group->regExp().pattern());

    ui.nepomukWidget->hide();

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(this, &GroupSettingsDialog::accepted, this, &GroupSettingsDialog::save);
}

GroupSettingsDialog::~GroupSettingsDialog()
{
}

QSize GroupSettingsDialog::sizeHint() const
{
    QSize sh = QDialog::sizeHint();
    sh.setWidth(sh.width() * 1.4);
    return sh;
}

void GroupSettingsDialog::save()
{
    // check needed, otherwise "/" would be added as folder if the line was empty!
    if (ui.defaultFolderRequester->text().isEmpty()) {
        m_group->setDefaultFolder(QString());
    } else {
        m_group->setDefaultFolder(ui.defaultFolderRequester->url().toLocalFile());
    }

    m_group->setDownloadLimit(ui.downloadBox->value(), Transfer::VisibleSpeedLimit);
    m_group->setUploadLimit(ui.uploadBox->value(), Transfer::VisibleSpeedLimit);

    QRegularExpression regExp;
    regExp.setPattern(ui.regExpEdit->text());
    m_group->setRegExp(regExp);
}

#include "moc_groupsettingsdialog.cpp"
