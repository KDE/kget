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
#include "signaturedlg.h"
#include "verificationdialog.h"
#include "settings.h"

#include "core/transferhandler.h"
#include "core/filemodel.h"
#include "core/verifier.h"

#include <KMessageBox>
#include <KLocalizedString>

#include <QLineEdit>
#include <QSortFilterProxyModel>

TransferSettingsDialog::TransferSettingsDialog(QWidget *parent, TransferHandler *transfer)
  : KGetSaveSizeDialog("TransferSettingsDialog", parent),
    m_transfer(transfer),
    m_model(m_transfer->fileModel())
{
    setWindowTitle(i18n("Transfer Settings for %1", m_transfer->source().fileName()));
    
    ui.setupUi(this);
    
    ui.ktitlewidget->setPixmap(QIcon::fromTheme("preferences-other").pixmap(16));
    ui.downloadSpin->setValue(m_transfer->downloadLimit(Transfer::VisibleSpeedLimit));
    ui.uploadSpin->setValue(m_transfer->uploadLimit(Transfer::VisibleSpeedLimit));
    ui.ratioSpin->setValue(m_transfer->maximumShareRatio());
    ui.destination->setUrl(m_transfer->directory());
    ui.destination->lineEdit()->setReadOnly(true);
    ui.rename->setIcon(QIcon::fromTheme("edit-rename"));
    ui.mirrors->setIcon(QIcon::fromTheme("download"));
    ui.signature->setIcon(QIcon::fromTheme("application-pgp-signature"));
    ui.verification->setIcon(QIcon::fromTheme("document-decrypt"));

    if (m_model)
    {
        m_model->watchCheckState();
        m_proxy = new QSortFilterProxyModel(this);
        m_proxy->setSourceModel(m_model);
        ui.treeView->setModel(m_proxy);
        ui.treeView->sortByColumn(0, Qt::AscendingOrder);

        QByteArray loadedState = QByteArray::fromBase64(Settings::transferSettingsHeaderState().toLatin1());
        if (!loadedState.isEmpty()) {
            ui.treeView->header()->restoreState(loadedState);
        } else {
            ui.treeView->header()->resizeSection(0, ui.treeView->header()->defaultSectionSize() * 3);
        }
    }

    updateCapabilities();

    connect(m_transfer, &TransferHandler::capabilitiesChanged, this, &TransferSettingsDialog::updateCapabilities);
    connect(this, &TransferSettingsDialog::accepted, this, &TransferSettingsDialog::save);
    connect(this, &TransferSettingsDialog::finished, this, &TransferSettingsDialog::slotFinished);
    connect(ui.treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TransferSettingsDialog::slotSelectionChanged);
    connect(ui.rename, &QPushButton::clicked, this, &TransferSettingsDialog::slotRename);
    connect(ui.mirrors, &QPushButton::clicked, this, &TransferSettingsDialog::slotMirrors);
    connect(ui.verification, &QPushButton::clicked, this, &TransferSettingsDialog::slotVerification);
    connect(ui.signature, &QPushButton::clicked, this, &TransferSettingsDialog::slotSignature);
    
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

TransferSettingsDialog::~TransferSettingsDialog()
{
    if (m_model) {
        Settings::setTransferSettingsHeaderState(ui.treeView->header()->saveState().toBase64());
    }
}

QSize TransferSettingsDialog::sizeHint() const
{
    QSize sh = QDialog::sizeHint();
    sh.setWidth(sh.width() * 1.7);
    return sh;
}

void TransferSettingsDialog::updateCapabilities()
{
    const int capabilities = m_transfer->capabilities();

    const bool supportsSpeedLimit = capabilities & Transfer::Cap_SpeedLimit;
    ui.labelDownload->setVisible(supportsSpeedLimit);
    ui.downloadSpin->setVisible(supportsSpeedLimit);
    ui.labelUpload->setVisible(supportsSpeedLimit);
    ui.uploadSpin->setVisible(supportsSpeedLimit);
    ui.labelShareRatio->setVisible(supportsSpeedLimit);
    ui.ratioSpin->setVisible(supportsSpeedLimit);

    ui.destination->setEnabled(capabilities & Transfer::Cap_Moving);
    ui.mirrors->setVisible(capabilities & Transfer::Cap_MultipleMirrors);
    ui.rename->setVisible(capabilities & Transfer::Cap_Renaming);
}

void TransferSettingsDialog::slotMirrors()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
    QDialog *mirrors = new MirrorSettings(this, m_transfer, m_model->getUrl(index));
    mirrors->setAttribute(Qt::WA_DeleteOnClose);
    mirrors->show();
}

void TransferSettingsDialog::slotRename()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
    auto *renameDlg = new RenameFile(m_model, index, this);
    renameDlg->setAttribute(Qt::WA_DeleteOnClose);
    renameDlg->show();
}

void TransferSettingsDialog::slotVerification()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());
    QDialog *verification = new VerificationDialog(this, m_transfer, m_model->getUrl(index));
    verification->setAttribute(Qt::WA_DeleteOnClose);
    verification->show();
}

void TransferSettingsDialog::slotSignature()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());

    auto *signature = new SignatureDlg(m_transfer, m_model->getUrl(index), this);
    signature->setAttribute(Qt::WA_DeleteOnClose);
    signature->show();
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
    ui.signature->setEnabled(enabled);
}

void TransferSettingsDialog::save()
{//TODO: Set to -1 when no limit
    QUrl oldDirectory = m_transfer->directory();
    QUrl newDirectory = ui.destination->url();
    if ((oldDirectory != newDirectory) && !m_transfer->setDirectory(newDirectory))
    {
        KMessageBox::error(this, i18n("Changing the destination did not work, the destination stays unmodified."), i18n("Destination unmodified"));
    }

    m_transfer->setDownloadLimit(ui.downloadSpin->value(), Transfer::VisibleSpeedLimit);
    m_transfer->setUploadLimit(ui.uploadSpin->value(), Transfer::VisibleSpeedLimit);
    m_transfer->setMaximumShareRatio(ui.ratioSpin->value());
}

void TransferSettingsDialog::slotFinished()
{
    if (m_model)
    {
        m_model->stopWatchCheckState();
    }
}


