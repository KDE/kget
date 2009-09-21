/* This file is part of the KDE project

   Copyright (C) 2008 Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/
#include "transfersettingsdialog.h"
#include "mirror/mirrorsettings.h"
#include "renamefile.h"
#include "verificationdialog.h"

#include "core/transferhandler.h"
#include "core/filemodel.h"

#include <KMessageBox>
#include <KLineEdit>
#include <QSortFilterProxyModel>

TransferSettingsDialog::TransferSettingsDialog(QWidget *parent, TransferHandler *transfer)
  : KDialog(parent),
    m_transfer(transfer),
    m_model(m_transfer->fileModel()),
    m_proxy(0)
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
    ui.kUrlRequester->setUrl(m_transfer->directory().pathOrUrl());
    ui.kUrlRequester->lineEdit()->setReadOnly(true);
    ui.kUrlRequester->setMode(KFile::Directory | KFile::ExistingOnly);
    ui.mirrors->setEnabled(false);
    ui.rename->setIcon(KIcon("edit-rename"));
    ui.rename->setEnabled(false);
    ui.verification->setEnabled(false);

    if (m_model)
    {
        m_model->watchCheckState();
        m_proxy = new QSortFilterProxyModel(this);
        m_proxy->setSourceModel(m_model);
        ui.treeView->setModel(m_proxy);
        ui.treeView->sortByColumn(0, Qt::AscendingOrder);
    }

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
    connect(this, SIGNAL(finished()), this, SLOT(slotFinished()));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
    connect(ui.rename, SIGNAL(clicked(bool)), this, SLOT(slotRename()));
    connect(ui.mirrors, SIGNAL(clicked(bool)), this, SLOT(slotMirrors()));
    connect(ui.verification, SIGNAL(clicked(bool)), this, SLOT(slotVerification()));
}

TransferSettingsDialog::~TransferSettingsDialog()
{
}

void TransferSettingsDialog::slotMirrors()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
    KDialog *mirrors = new MirrorSettings(this, m_transfer, m_model->getUrl(index));
    mirrors->setAttribute(Qt::WA_DeleteOnClose);
    mirrors->show();
}

void TransferSettingsDialog::slotRename()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
    RenameFile *renameDlg = new RenameFile(m_model, index, this);
    renameDlg->setAttribute(Qt::WA_DeleteOnClose);
    renameDlg->show();
}

void TransferSettingsDialog::slotVerification()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
    KDialog *verification = new VerificationDialog(this, m_transfer, m_model->getUrl(index));
    verification->setAttribute(Qt::WA_DeleteOnClose);
    verification->show();
}

void TransferSettingsDialog::slotSelectionChanged()
{
    bool enabled = false;
    //only enable rename when one item is selected and when this item is a file
    if (ui.treeView->selectionModel()->selectedRows().count() == 1)
    {
        const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
        if (index.isValid() && !(static_cast<FileItem*>(index.internalPointer()))->childCount())
        {
            enabled = true;
        }
    }
    ui.mirrors->setEnabled(enabled);
    ui.rename->setEnabled(enabled);
    ui.verification->setEnabled(enabled);
}

void TransferSettingsDialog::save()
{//TODO: Set to -1 when no limit
    KUrl oldDirectory = m_transfer->directory();
    KUrl newDirectory = ui.kUrlRequester->url();
    if ((oldDirectory != newDirectory) && !m_transfer->setDirectory(newDirectory))
    {
        KMessageBox::error(this, i18n("Changing the destination did not work, the destination stays unmodified."), i18n("Destination unmodified"));
    }

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

void TransferSettingsDialog::slotFinished()
{
    if (m_model)
    {
        m_model->stopWatchCheckState();
    }
}

#include "transfersettingsdialog.moc"
