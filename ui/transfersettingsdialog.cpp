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
#include <KLineEdit>
#include <QSortFilterProxyModel>

TransferSettingsDialog::TransferSettingsDialog(QWidget *parent, TransferHandler *transfer)
  : KGetSaveSizeDialog("TransferSettingsDialog", parent),
    m_transfer(transfer),
    m_model(m_transfer->fileModel()),
    m_proxy(0)
{
    setCaption(i18n("Transfer Settings for %1", m_transfer->source().fileName()));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);
    ui.ktitlewidget->setPixmap(SmallIcon("preferences-other"));
    ui.downloadSpin->setValue(m_transfer->downloadLimit(Transfer::VisibleSpeedLimit));
    ui.uploadSpin->setValue(m_transfer->uploadLimit(Transfer::VisibleSpeedLimit));
    ui.ratioSpin->setValue(m_transfer->maximumShareRatio());
    ui.destination->setUrl(m_transfer->directory().pathOrUrl());
    ui.destination->lineEdit()->setReadOnly(true);
    ui.rename->setIcon(KIcon("edit-rename"));
    ui.mirrors->setIcon(KIcon("download"));
    ui.signature->setIcon(KIcon("application-pgp-signature"));
    ui.verification->setIcon(KIcon("document-decrypt"));

    if (m_model)
    {
        m_model->watchCheckState();
        m_proxy = new QSortFilterProxyModel(this);
        m_proxy->setSourceModel(m_model);
        ui.treeView->setModel(m_proxy);
        ui.treeView->sortByColumn(0, Qt::AscendingOrder);

        QByteArray loadedState = QByteArray::fromBase64(Settings::transferSettingsHeaderState().toAscii());
        if (!loadedState.isEmpty()) {
            ui.treeView->header()->restoreState(loadedState);
        } else {
            ui.treeView->header()->resizeSection(0, ui.treeView->header()->defaultSectionSize() * 3);
        }
    }

    updateCapabilities();

    connect(m_transfer, SIGNAL(capabilitiesChanged()), this, SLOT(updateCapabilities()));
    connect(this, SIGNAL(accepted()), SLOT(save()));
    connect(this, SIGNAL(finished()), this, SLOT(slotFinished()));
    connect(ui.treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
    connect(ui.rename, SIGNAL(clicked(bool)), this, SLOT(slotRename()));
    connect(ui.mirrors, SIGNAL(clicked(bool)), this, SLOT(slotMirrors()));
    connect(ui.verification, SIGNAL(clicked(bool)), this, SLOT(slotVerification()));
    connect(ui.signature, SIGNAL(clicked(bool)), this, SLOT(slotSignature()));
}

TransferSettingsDialog::~TransferSettingsDialog()
{
    if (m_model) {
        Settings::setTransferSettingsHeaderState(ui.treeView->header()->saveState().toBase64());
    }
}

QSize TransferSettingsDialog::sizeHint() const
{
    QSize sh = KDialog::sizeHint();
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

void TransferSettingsDialog::slotSignature()
{
    const QModelIndex index = m_proxy->mapToSource(ui.treeView->selectionModel()->selectedIndexes().first());

    SignatureDlg *signature = new SignatureDlg(m_transfer, m_model->getUrl(index), this);
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
    KUrl oldDirectory = m_transfer->directory();
    KUrl newDirectory = ui.destination->url();
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

#include "transfersettingsdialog.moc"
