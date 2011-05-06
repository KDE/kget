/**************************************************************************
*   Copyright (C) 2009-2011 Matthias Fuchs <mat69@gmx.net>                *
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

#include "signature_p.h"
#include "keydownloader.h"
#include "settings.h"

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <QtXml/QDomElement>

#ifdef HAVE_QGPGME
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <qgpgme/dataprovider.h>

#include <QtCore/QFile>
#endif

#ifdef HAVE_QGPGME
K_GLOBAL_STATIC(KeyDownloader, signatureDownloader)
#endif //HAVE_QGPGME

SignaturePrivate::SignaturePrivate(Signature *signature)
  : q(signature),
    type(Signature::NoType),
    status(Signature::NoResult),
    verifyTried(false),
    sigSummary(0),
    error(0)
{
}

SignaturePrivate::~SignaturePrivate()
{
}

void SignaturePrivate::signatureDownloaded()
{
    if (verifyTried) {
        kDebug(5001) << "Rerun verification.";
        q->verify();
    }
}

#ifdef HAVE_QGPGME
GpgME::VerificationResult SignaturePrivate::verify(const KUrl &dest, const QByteArray &sig)
{
    GpgME::VerificationResult result;
    if (!QFile::exists(dest.pathOrUrl()) || sig.isEmpty()) {
        return result;
    }

    GpgME::initializeLibrary();
    GpgME::Error error = GpgME::checkEngine(GpgME::OpenPGP);
    if (error) {
        kDebug(5001) << "OpenPGP not supported!";
        return result;
    }

    QScopedPointer<GpgME::Context> context(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (!context.data()) {
        kDebug(5001) << "Could not create context.";
        return result;
    }

    boost::shared_ptr<QFile> qFile(new QFile(dest.pathOrUrl()));
    qFile->open(QIODevice::ReadOnly);
    QGpgME::QIODeviceDataProvider *file = new QGpgME::QIODeviceDataProvider(qFile);
    GpgME::Data dFile(file);

    QGpgME::QByteArrayDataProvider signatureBA(sig);
    GpgME::Data signature(&signatureBA);

    return context->verifyDetachedSignature(signature, dFile);
}
#endif //HAVE_QGPGME


Signature::Signature(const KUrl &dest, QObject *object)
  : QObject(object),
    d(new SignaturePrivate(this))
{
    d->dest = dest;
#ifdef HAVE_QGPGME
    qRegisterMetaType<GpgME::VerificationResult>("GpgME::VerificationResult");
    connect(&d->thread, SIGNAL(verified(GpgME::VerificationResult)), this, SLOT(slotVerified(GpgME::VerificationResult)));
#endif //HAVE_QGPGME
}

Signature::~Signature()
{
    delete d;
}

KUrl Signature::destination() const
{
    return d->dest;
}

void Signature::setDestination(const KUrl &destination)
{
    d->dest = destination;
}

Signature::VerificationStatus Signature::status() const
{
    return d->status;
}

#ifdef HAVE_QGPGME
GpgME::VerificationResult Signature::verificationResult()
{
    return d->verificationResult;
}
#endif //HAVE_QGPGME

QByteArray Signature::signature()
{
    return d->signature;
}

void Signature::setAsciiDetatchedSignature(const QString &signature)
{
    setSignature(signature.toAscii(), AsciiDetached);
}

void Signature::setSignature(const QByteArray &signature, SignatureType type)
{
    if ((signature == d->signature) && (type == d->type)) {
        return;
    }

    d->type = type;
    d->signature = signature;

    d->fingerprint.clear();
    d->error = 0;
    d->sigSummary = 0;
    d->status = Signature::NoResult;

#ifdef HAVE_QGPGME
    d->verificationResult = GpgME::VerificationResult();
#endif //HAVE_QGPGME

    emit verified(d->status);//FIXME
}

Signature::SignatureType Signature::type() const
{
    return d->type;
}

QString Signature::fingerprint()
{
    return d->fingerprint;
}

void Signature::downloadKey(QString fingerprint) // krazy:exclude=passbyvalue
{
#ifdef HAVE_QGPGME
    kDebug(5001) << "Downloading key:" << fingerprint;
    signatureDownloader->downloadKey(fingerprint, this);
#else
    Q_UNUSED(fingerprint)
#endif //HAVE_QGPGME
}

bool Signature::isVerifyable()
{
#ifdef HAVE_QGPGME
    return QFile::exists(d->dest.pathOrUrl()) && !d->signature.isEmpty();
#else
    return false;
#endif //HAVE_QGPGME
}

void Signature::verify()
{
#ifdef HAVE_QGPGME
    d->thread.verify(d->dest, d->signature);
#endif //HAVE_QGPGME
}

#ifdef HAVE_QGPGME
void Signature::slotVerified(const GpgME::VerificationResult &result)
{
    d->verificationResult = result;
    d->status = Signature::NotWorked;

    if (!d->verificationResult.numSignatures()) {
        kDebug(5001) << "No signatures\n";
        emit verified(d->status);
        return;
    }

    GpgME::Signature signature = d->verificationResult.signature(0);
    d->sigSummary = signature.summary();
    d->error = signature.status().code();
    d->fingerprint = signature.fingerprint();

    kDebug(5001) << "Fingerprint:" << d->fingerprint;
    kDebug(5001) << "Signature summary:" << d->sigSummary;
    kDebug(5001) << "Error code:" << d->error;

    if (d->sigSummary & GpgME::Signature::KeyMissing) {
        kDebug(5001) << "Public key missing.";
        if (Settings::signatureAutomaticDownloading() ||
            (KMessageBox::warningYesNoCancel(0,
             i18n("The key to verify the signature is missing, do you want to download it?")) == KMessageBox::Yes)) {
            d->verifyTried = true;
            downloadKey(d->fingerprint);
            emit verified(d->status);
            return;
        }
    }

    if (!signature.status()) {
        if (d->sigSummary & GpgME::Signature::Valid) {
            d->status = Signature::Verified;
        } else if ((d->sigSummary & GpgME::Signature::Green) || (d->sigSummary == 0)) {
            d->status = Signature::VerifiedInformation;
        }
    } else if (signature.status()) {
        if ((d->sigSummary & GpgME::Signature::KeyExpired) || (d->sigSummary & GpgME::Signature::KeyRevoked)) {
            d->status = Signature::VerifiedWarning;
        }
        if (d->sigSummary & GpgME::Signature::Red) {//TODO handle more cases!
            d->status = Signature::NotVerified;
            //TODO handle that dialog better in 4.5
            KMessageBox::error(0,
                            i18n("The signature could not be verified for %1. See transfer settings for more information.", d->dest.fileName()),
                            i18n("Signature not verified"));
        }
    }

    emit verified(d->status);
}
#endif //HAVE_QGPGME

void Signature::save(const QDomElement &element)
{
    QDomElement e = element;

    QDomElement verification = e.ownerDocument().createElement("signature");
    verification.setAttribute("status", d->status);
    verification.setAttribute("sigStatus", d->sigSummary);
    verification.setAttribute("error", d->error);
    verification.setAttribute("fingerprint", d->fingerprint);
    verification.setAttribute("type", d->type);
    QDomText value;
    switch (d->type) {
        case NoType:
        case AsciiDetached:
            value = e.ownerDocument().createTextNode(d->signature);
            break;
        case BinaryDetached:
            value = e.ownerDocument().createTextNode(d->signature.toBase64());
            break;
    }
    verification.appendChild(value);


    e.appendChild(verification);
}

void Signature::load(const QDomElement &e)
{
    QDomElement verification = e.firstChildElement("signature");
    d->status = static_cast<VerificationStatus>(verification.attribute("status").toInt());
    d->sigSummary = verification.attribute("sigStatus").toInt();
    d->error = verification.attribute("error").toInt();
    d->fingerprint = verification.attribute("fingerprint");
    d->type = static_cast<SignatureType>(verification.attribute("type").toInt());
    switch (d->type) {
        case NoType:
        case AsciiDetached:
            d->signature = verification.text().toAscii();
            break;
        case BinaryDetached:
            d->signature = QByteArray::fromBase64(verification.text().toAscii());
    }
}

#include "signature.moc"
