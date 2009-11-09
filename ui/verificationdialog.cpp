/***************************************************************************
*   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "verificationdialog.h"

#include <QtGui/QStandardItemModel>

#include <KLocale>
#include <KMessageBox>

#include "core/filemodel.h"
#include "core/transferhandler.h"
#include "core/verifier.h"

VerificationAddDlg::VerificationAddDlg(VerificationModel *model, QWidget *parent, Qt::WFlags flags)
  : KDialog(parent, flags),
    m_model(model)
{
    setCaption(i18n("Add checksum"));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    QStringList supportedTypes = Verifier::supportedVerficationTypes();
    supportedTypes.sort();
    ui.hashTypes->addItems(supportedTypes);
    ui.successLabel->hide();

    setButtons(KDialog::Yes | KDialog::User1 | KDialog::Cancel);
    setButtonGuiItem(KDialog::Yes, KStandardGuiItem::add());
    setButtonGuiItem(KDialog::User1, KGuiItem(i18nc("Adds the item and reopens the dialog to add a further item", "Add more"), KIcon("list-add")));
    showButton(KDialog::Yes, true);
    showButton(KDialog::User1, true);

    updateButton();

    connect(ui.newHash, SIGNAL(userTextChanged(const QString &)), this, SLOT(updateButton()));
    connect(ui.hashTypes, SIGNAL(currentIndexChanged(int)), this, SLOT(updateButton()));
    connect(this, SIGNAL(yesClicked()), this, SLOT(addChecksum()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(addMore()));
}

void VerificationAddDlg::updateButton()
{
    const QString type = ui.hashTypes->currentText();
    const QString hash = ui.newHash->text();
    const bool enabled = Verifier::isChecksum(type, hash);

    enableButton(KDialog::Yes, enabled);
    enableButton(KDialog::User1, enabled);
}

void VerificationAddDlg::addChecksum()
{
    if (m_model)
    {
        m_model->addChecksum(ui.hashTypes->currentText(), ui.newHash->text());
    }
}

void VerificationAddDlg::addMore()
{
    addChecksum();
    ui.successLabel->setText(i18n("%1 %2 has been successfully added.", ui.hashTypes->currentText(), ui.newHash->text()));
    ui.newHash->clear();
    ui.successLabel->show();
    ui.hashTypes->setFocus();
}

VerificationDialog::VerificationDialog(QWidget *parent, TransferHandler *transfer, const KUrl &file)
  : KDialog(parent),
    m_transfer(transfer),
    m_file(file),
    m_verifier(transfer->verifier(m_file)),
    m_model(0),
    m_fileModel(0)
{
    if (m_verifier)
    {
        m_model = m_verifier->model();
        connect(m_verifier, SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
    }

    setCaption(i18n("Transfer Verification for %1", m_file.fileName()));
    showButtonSeparator(true);
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);
    ui.add->setGuiItem(KStandardGuiItem::add());
    ui.remove->setGuiItem(KStandardGuiItem::remove());
    ui.verifying->hide();

    if (m_model)
    {
        ui.usedHashes->setModel(m_model);
        ui.usedHashes->setItemDelegate(new VerificationDelegate(this));
        m_fileModel = m_transfer->fileModel();

        if (m_fileModel) {
            connect(m_fileModel, SIGNAL(fileFinished(KUrl)), this, SLOT(fileFinished(KUrl)));
        }

        updateButtons();

        connect(m_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(updateButtons()));
        connect(ui.usedHashes, SIGNAL(clicked(const QModelIndex&)), this, SLOT(updateButtons()));
        connect(ui.add, SIGNAL(pressed()), this, SLOT(addPressed()));
        connect(ui.remove, SIGNAL(pressed()), this, SLOT(removePressed()));
        connect(ui.verify, SIGNAL(pressed()), this, SLOT(verifyPressed()));
    }

    setButtons(KDialog::Close);
}

void VerificationDialog::fileFinished(const KUrl &file)
{
    if (m_file == file) {
        updateButtons();
    }
}

void VerificationDialog::updateButtons()
{
    ui.remove->setEnabled(m_model && ui.usedHashes->selectionModel()->selectedIndexes().count());

    //check if the download finished and if the selected indexes are verifyable
    bool verifyEnabled = false;
    if (m_fileModel && m_fileModel->downloadFinished(m_file))
    {
        const QModelIndexList indexes = ui.usedHashes->selectionModel()->selectedRows();
        if (indexes.count())
        {
            foreach (const QModelIndex &index, indexes)
            {
                verifyEnabled = m_verifier->isVerifyable(index);
                if (!verifyEnabled)
                {
                    break;
                }
            }
        }
    }
    ui.verify->setEnabled(verifyEnabled);
}

void VerificationDialog::removePressed()
{
    const QModelIndexList indexes = ui.usedHashes->selectionModel()->selectedRows();
    foreach (const QModelIndex &index, indexes)
    {
        m_model->removeRow(index.row());
    }
    updateButtons();
}

void VerificationDialog::addPressed()
{
    VerificationAddDlg *dialog = new VerificationAddDlg(m_model, this);
    dialog->show();
}

void VerificationDialog::verifyPressed()
{
    const QModelIndex index = ui.usedHashes->selectionModel()->selectedRows().first();
    if (index.isValid())
    {
         m_verifier->verify(index);
         ui.progressBar->setMaximum(0);
         ui.verifying->show();
    }
}

void VerificationDialog::slotVerified(bool verified)
{
    ui.progressBar->setMaximum(1);
    ui.verifying->hide();

    if (verified)
    {
        KMessageBox::information(this,
                                 i18n("%1 was successfully verified.", m_file.fileName()),
                                 i18n("Verification successful"));
    }
}

#include "verificationdialog.moc"
