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

#ifndef KGET_SIGNATURE_H
#define KGET_SIGNATURE_H

#include "../kget_export.h"

#include <KUrl>

class QDomElement;

#ifdef HAVE_QGPGME
#include <gpgme++/verificationresult.h>
#endif //HAVE_QGPGME

/**
 * @class Signature
 *
 * @short Class to verify signatures
 */
class KGET_EXPORT Signature : public QObject
{
    Q_OBJECT

    friend class KeyDownloader;
    friend class SignatureThread;

//TODO also support verification and decryption of files that contain the signature?
    public:
        explicit Signature(const KUrl &dest, QObject *object = 0);
        ~Signature();

        enum SignatureType
        {
            NoType = 0,
            AsciiDetached, //.asc
            BinaryDetached //.sig
        };

        enum VerificationStatus
        {
            NoResult, //either not tried, or not enough information
            NotWorked, //something during verification failed
            NotVerified,
            Verified,
            VerifiedInformation, //verified, though the there is some additional information
            VerifiedWarning //verified, though there is a warning
        };

        KUrl destination() const;
        void setDestination(const KUrl &destination);

        VerificationStatus status() const;
#ifdef HAVE_QGPGME
        GpgME::VerificationResult verificationResult();
#endif //HAVE_QGPGME

        void downloadKey(QString fingerprint);
        QByteArray signature();
        void setAsciiDetatchedSignature(const QString &signature);
        void setSignature(const QByteArray &signature, SignatureType type);

        SignatureType type() const;

        /**
         * The fingerprint of the signature//TODO get even without verification??
         */
        QString fingerprint();
        bool isVerifyable();
        void verify();

        void save(const QDomElement &element);
        void load(const QDomElement &e);

    signals:
        void verified(int verificationStatus);

    private slots:
#ifdef HAVE_QGPGME
        void slotVerified(const GpgME::VerificationResult &result);
#endif //HAVE_QGPGME

    private:
        class SignaturePrivate *d;

        friend class SignaturePrivate;
};

#endif
