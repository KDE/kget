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

#ifndef KGET_SIGNATURE_P_H
#define KGET_SIGNATURE_P_H

#include "signature.h"
#include "signaturethread.h"

struct SignaturePrivate
{
    SignaturePrivate(Signature *signature);
    ~SignaturePrivate();

    /**
     * Starts verification again if a verification was tried before but aborted
     * because of missing keys
     */
    void signatureDownloaded();

#ifdef HAVE_QGPGME
    /**
     * Verifies a signature
     */
    static GpgME::VerificationResult verify(const KUrl &dest, const QByteArray &sig);
#endif //HAVE_QGPGME

    Signature *q;

    Signature::SignatureType type;
    Signature::VerificationStatus status;
    bool verifyTried;
    int sigSummary;
    int error;
    SignatureThread thread;
    KUrl dest;
    QByteArray signature;
    QString fingerprint;
#ifdef HAVE_QGPGME
    GpgME::VerificationResult verificationResult;
#endif //HAVE_QGPGME
};

#endif
