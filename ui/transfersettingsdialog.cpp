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
    setCaption(i18n("Transfer Settings for %1", m_transfer->source().fileName()));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);
    ui.downloadSpin->setValue(m_transfer->downloadLimit(Transfer::VisibleSpeedLimit));
    ui.uploadSpin->setValue(m_transfer->uploadLimit(Transfer::VisibleSpeedLimit));
    ui.ratioSpin->setValue(m_transfer->maximumShareRatio());
    ui.downloadCheck->setChecked(ui.downloadSpin->value() != 0);
    ui.uploadCheck->setChecked(ui.uploadSpin->value() != 0);
    ui.ratioCheck->setChecked(ui.ratioSpin->value() != 0);

    if (!transfer->supportsSpeedLimits())
    {
        ui.downloadCheck->setDisabled(true);
        ui.downloadSpin->setDisabled(true);
        ui.uploadCheck->setDisabled(true);
        ui.uploadSpin->setDisabled(true);
        ui.ratioCheck->setDisabled(true);
        ui.ratioSpin->setDisabled(true);
    }
    connect(this, SIGNAL(accepted()), SLOT(save()));
}

TransferSettingsDialog::~TransferSettingsDialog()
{
}

void TransferSettingsDialog::save()
{//TODO: Set to -1 when no limit
    if (ui.downloadCheck->isChecked())
        m_transfer->setDownloadLimit(ui.downloadSpin->value(), Transfer::VisibleSpeedLimit);
    else
        m_transfer->setDownloadLimit(0, Transfer::VisibleSpeedLimit);

    if (ui.uploadCheck->isChecked())
        m_transfer->setUploadLimit(ui.uploadSpin->value(), Transfer::VisibleSpeedLimit);
    else
        m_transfer->setUploadLimit(0, Transfer::VisibleSpeedLimit);

    if (ui.ratioCheck->isChecked())
        m_transfer->setMaximumShareRatio(ui.ratioSpin->value());
    else
        m_transfer->setMaximumShareRatio(0);
}

#include "transfersettingsdialog.moc"
