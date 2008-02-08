/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "transfersettingsdialog.h"

#include "core/transferhandler.h"

TransferSettingsDialog::TransferSettingsDialog(QWidget *parent, TransferHandler *transfer)
  : KDialog(parent),
    m_transfer(transfer)
{
    setCaption(i18n("Transfer-Settings for %1", m_transfer->source().fileName()));
    QWidget *widget = new QWidget(this);
    Ui::TransferSettingsDialog ui;
    ui.setupUi(widget);
    setMainWidget(widget);
    m_downloadSpin = ui.downloadSpin;
    m_downloadSpin->setValue(m_transfer->visibleDownloadLimit());
    m_uploadSpin = ui.uploadSpin;
    m_uploadSpin->setValue(m_transfer->visibleUploadLimit());
    m_downloadCheck = ui.downloadCheck;
    m_downloadCheck->setChecked(m_downloadSpin->value() != 0);
    m_uploadCheck = ui.uploadCheck;
    m_uploadCheck->setChecked(m_uploadSpin->value() != 0);

    if (!transfer->supportsSpeedLimits())
    {
        m_downloadCheck->setDisabled(true);
        m_downloadSpin->setDisabled(true);
        m_uploadCheck->setDisabled(true);
        m_uploadSpin->setDisabled(true);
    }
    connect(this, SIGNAL(accepted()), SLOT(save()));
}

TransferSettingsDialog::~TransferSettingsDialog()
{
}

void TransferSettingsDialog::save()
{
    if (m_downloadCheck->isChecked())
        m_transfer->setVisibleDownloadLimit(m_downloadSpin->value());
    else
        m_transfer->setVisibleDownloadLimit(0);

    if (m_uploadCheck->isChecked())
        m_transfer->setVisibleUploadLimit(m_uploadSpin->value());
    else
        m_transfer->setVisibleUploadLimit(0);
}

#include "transfersettingsdialog.moc"
 
