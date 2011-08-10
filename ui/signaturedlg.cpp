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

#include "signaturedlg.h"

#include "core/kget.h"
#include "core/filemodel.h"
#include "core/transferhandler.h"
#include "core/signature.h"

#ifdef HAVE_QGPGME
#include <gpgme++/context.h>
#include <gpgme++/key.h>
#endif

#include <QtGui/QLayoutItem>

#include <KFileDialog>
#include <KLocale>

const QStringList SignatureDlg::OWNERTRUST = QStringList() << i18nc("trust level", "Unknown") << i18nc("trust level", "Undefined") << i18nc("trust level", "Never") << i18nc("trust level", "Marginal") << i18nc("trust level", "Full") << i18nc("trust level", "Ultimate");

SignatureDlg::SignatureDlg(TransferHandler *transfer, const KUrl &dest, QWidget *parent, Qt::WFlags flags)
  : KGetSaveSizeDialog("SignatureDlg", parent, flags),
    m_signature(transfer->signature(dest)),
    m_fileModel(transfer->fileModel())
{
    setCaption(i18nc("Signature here is meant in cryptographic terms, so the signature of a file.", "Signature of %1.", dest.fileName()));
    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    ui.loadSignature->setIcon(KIcon("document-open"));
    ui.verify->setIcon(KIcon("document-encrypt"));
    setMainWidget(widget);

    ui.information->setCloseButtonVisible(false);
    ui.information->setWordWrap(true);
    if (m_signature) {
        connect(ui.loadSignature, SIGNAL(clicked(bool)), this, SLOT(loadSignatureClicked()));
        connect(ui.verify, SIGNAL(clicked()), this, SLOT(verifyClicked()));
        connect(ui.signature, SIGNAL(textChanged()), this, SLOT(textChanged()));
        connect(m_signature, SIGNAL(verified(int)), this, SLOT(updateData()));

        if (m_fileModel) {
            m_file = m_fileModel->index(dest, FileItem::File);
            connect(m_fileModel, SIGNAL(fileFinished(KUrl)), this, SLOT(fileFinished(KUrl)));
        }

        updateData();
        updateButtons();
    } else {
        ui.information->setMessageType(KMessageWidget::Warning);
        ui.information->setText(i18n("This option is not supported for the current transfer."));
        ui.sigGroup->hide();
        ui.keyGroup->hide();
    }
}

void SignatureDlg::fileFinished(const KUrl &file)
{
    if (m_fileModel && (m_fileModel->getUrl(m_file) == file)) {
        updateButtons();
    }
}

void SignatureDlg::textChanged()
{
    if (m_signature) {
        m_signature->setAsciiDetatchedSignature(ui.signature->toPlainText());

        clearData();
        updateButtons();
    }
}

void SignatureDlg::loadSignatureClicked()
{
    const KUrl url = KFileDialog::getOpenUrl(KGet::generalDestDir(), "*.asc|" + i18n("Detached OpenPGP ASCII signature (*.asc)") + '\n' +
                                             "*.sig|" + i18n("Detached OpenPGP binary signature (*.sig)"), this, i18n("Load Signature File"));
    if (url.isEmpty()) {
        return;
    }

    const bool isAsciiSig = url.fileName().endsWith("asc");
    clearData();
    handleWidgets(isAsciiSig);
    ui.signature->clear();

    QFile file(url.path());
    if (!file.open(QIODevice::ReadOnly)) {
        kWarning(5001) << "Could not open file" << url;
        return;
    }
    if (file.size() > 1 * 1024) {
        kWarning(5001) << "File is larger than 1 KiB, which is not supported.";
        return;
    }

    const QByteArray data = file.readAll();
    if (isAsciiSig) {
        ui.signature->setText(data);
    } else if (m_signature) {
        m_signature->setSignature(data, Signature::BinaryDetached);
        clearData();
        updateButtons();
    }
}

void SignatureDlg::updateButtons()
{
    bool enableVerify = m_signature && m_signature->isVerifyable();
    if (!m_fileModel || !m_fileModel->downloadFinished(m_fileModel->getUrl(m_file))) {
        enableVerify = false;
    }
    ui.verify->setEnabled(enableVerify);
}

void SignatureDlg::updateData()
{
    if (!m_signature) {
        return;
    }

    const QString fingerprintString = m_signature->fingerprint();
    const QByteArray signature = m_signature->signature();

    QStringList information;

    bool problem = false;
    bool error = false;
    if (signature.isEmpty()) {
        information << i18n("You need to define a signature.");
        problem = true;
    }
    if (fingerprintString.isEmpty()) {
        information << i18n("No fingerprint could be found, check if the signature is correct or verify the download.");//TODO get fingerprint from signature!
        problem = true;
    }

    ui.fingerprint->setText(fingerprintString);

    const bool isAsciiSig = (m_signature->type() != Signature::BinaryDetached);
    handleWidgets(isAsciiSig);
    if (isAsciiSig) {
        ui.signature->blockSignals(true);
        ui.signature->setText(signature);
        ui.signature->blockSignals(false);
    }

    ui.keyGroup->setVisible(!signature.isEmpty());

    const int iconSize = KIconLoader::global()->currentSize(KIconLoader::Small);

#ifdef HAVE_QGPGME
    if (!fingerprintString.isEmpty()) {
        GpgME::initializeLibrary();
        GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
        QScopedPointer<GpgME::Context> context(GpgME::Context::createForProtocol(GpgME::OpenPGP));
        if (err) {
            kDebug(5001) << "OpenPGP not supported!";
        } else if (!context.data()) {
                kDebug(5001) << "Could not create context.";
        } else {
            QByteArray fingerprint = fingerprintString.toAscii();
            const GpgME::Key key = context->key(fingerprint.constData(), err);
            if (err || key.isNull() || !key.numUserIDs() || !key.numSubkeys()) {
                kDebug(5001) << "There was an error while loading the key:" << err;
            } else {
                if (key.isRevoked()) {
                    information << i18n("The key has been revoked.");
                    problem = true;
                }
                if (key.isDisabled()) {
                    information << i18n("The key is disabled.");
                    problem = true;
                }
                if (key.isInvalid()) {
                    information << i18n("The key is invalid.");
                    error = true;
                }
                ui.expirationIcon->clear();
                if (key.isExpired()) {
                    information << i18n("The key is expired.");
                    ui.expirationIcon->setPixmap(KIcon("dialog-warning").pixmap(iconSize));
                    ui.expirationIcon->show();
                    problem = true;
                } else {
                    ui.expirationIcon->hide();
                }


                //handle the trust of the key
                const GpgME::Key::OwnerTrust ownerTrust = key.ownerTrust();
                ui.trust->setText(OWNERTRUST.value(ownerTrust));

                switch (ownerTrust) {
                    case GpgME::Key::Never:
                        information.prepend(i18n("The key is not to be trusted."));
                        ui.trustIcon->setPixmap(KIcon("dialog-error").pixmap(iconSize));
                        error = true;
                        break;
                    case GpgME::Key::Marginal:
                        information.prepend(i18n("The key is to be trusted marginally."));
                        ui.trustIcon->setPixmap(KIcon("dialog-warning").pixmap(iconSize));
                        problem = true;
                        break;
                    case GpgME::Key::Full:
                        ui.trustIcon->setPixmap(KIcon("dialog-ok").pixmap(iconSize));
                        break;
                    case GpgME::Key::Ultimate:
                        ui.trustIcon->setPixmap(KIcon("dialog-ok").pixmap(iconSize));
                        break;
                    case GpgME::Key::Unknown:
                    case GpgME::Key::Undefined:
                    default:
                        information.prepend(i18n("Trust level of the key is unclear."));
                        ui.trustIcon->setPixmap(KIcon("dialog-warning").pixmap(iconSize));
                        problem = true;
                        break;
                }

                //issuer, issuer mail and comment
                if (key.numUserIDs()) {
                    const GpgME::UserID userID = key.userID(0);
                    ui.issuer->setText(userID.name());
                    ui.comment->setText(userID.comment());
                    ui.email->setText(userID.email());
                }

                //key creation/expiration-time
                if (key.numSubkeys()) {
                    const GpgME::Subkey subKey = key.subkey(0);

                    time_t creation = subKey.creationTime();
                    QDateTime creationTime;
                    creationTime.setTime_t(creation);
                    ui.creation->setText(creationTime.toString());

                    time_t expiration = subKey.expirationTime();
                    if (expiration) {
                        QDateTime expirationTime;
                        expirationTime.setTime_t(expiration);
                        ui.expiration->setText(expirationTime.toString());
                    } else {
                        ui.expiration->setText(i18n("Unlimited"));
                    }
                }
            }
        }
    }
#endif //HAVE_QGPGME

    const Signature::VerificationStatus verificationStatus = m_signature->status();

    //display the verification status
    ui.verificationIcon->hide();
    switch (verificationStatus) {
        case Signature::Verified:
        case Signature::VerifiedInformation:
        case Signature::VerifiedWarning:
            ui.verificationIcon->setPixmap(KIcon("dialog-ok").pixmap(iconSize));
            ui.verificationIcon->show();
            ui.verified->setText(i18nc("pgp signature is verified", "Verified"));
            break;
        case Signature::NotVerified:
            ui.verified->setText(i18nc("pgp signature is not verified", "Failed"));
            ui.verificationIcon->setPixmap(KIcon("dialog-error").pixmap(iconSize));
            ui.verificationIcon->show();
            information.prepend(i18n("Caution: Verification failed. Either you entered the wrong signature, or the data has been modified."));
            error = true;
            break;
        case Signature::NotWorked://TODO downloading state? --> currently downloading
            ui.verified->clear();//TODO
            information.prepend(i18n("Verification not possible. Check the entered data, whether gpg-agent is running, or whether you have an Internet connection (for retrieving keys.)"));
            problem = true;
            break;
        case Signature::NoResult:
            ui.verified->clear();//TODO
            break;
    }

    if (verificationStatus == Signature::VerifiedWarning) {
        problem = true;
    }

#ifndef HAVE_QGPGME
    ui.sigGroup->hide();
    ui.keyGroup->hide();
    ui.verify->hide();
    information.clear();
    information << i18n("Feature is not supported, as KGet is not compiled with QPGME support.");
    resize(350, 200);
#endif //HAVE_QPGME

    //TODO more messages, e.g. from result etc.
    //TODO enter more error cases

    //change the icon of the titlewidget
    if (error) {
        ui.information->setMessageType(KMessageWidget::Error);
    } else if (problem) {
        ui.information->setMessageType(KMessageWidget::Warning);
    } else {
        if (verificationStatus != Signature::Verified) {
            ui.information->setMessageType(KMessageWidget::Information);
        }
    }

    const QString text = information.join(" ");
    ui.information->setVisible(!text.isEmpty());
    ui.information->setText(text);
}

void SignatureDlg::verifyClicked()
{
    clearData();

    m_signature->verify();
}

void SignatureDlg::clearData()
{
    ui.verified->clear();
    ui.verificationIcon->clear();
    ui.issuer->clear();
    ui.email->clear();
    ui.comment->clear();
    ui.creation->clear();
    ui.expiration->clear();
    ui.expirationIcon->hide();
    ui.trust->clear();
    ui.trustIcon->clear();
    ui.fingerprint->clear();
}

void SignatureDlg::handleWidgets(bool isAsciiSig)
{
    ui.asciiLabel->setVisible(isAsciiSig);
    ui.signature->setVisible(isAsciiSig);
    ui.binaryLabel->setVisible(!isAsciiSig);
    QLayoutItem *item = ui.verticalLayout_2->itemAt(ui.verticalLayout_2->count() - 1);
    QSpacerItem *spacer = item->spacerItem();
    if (isAsciiSig) {
        if (spacer) {
            ui.verticalLayout_2->removeItem(item);
            delete item;
        }
    } else if (!spacer) {
        ui.verticalLayout_2->addStretch(1);
    }
}

#include "signaturedlg.moc"
