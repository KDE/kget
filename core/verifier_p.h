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

#ifndef VERIFIER_P_H
#define VERIFIER_P_H

class PartialChecksums;
class Verifier;

#include "verifier.h"
#include "verificationthread.h"

struct VerifierPrivate
{
    VerifierPrivate(Verifier *verifier)
      : q(verifier),
        model(0)
    {
    }

    ~VerifierPrivate();

    static QString calculatePartialChecksum(QFile *file, const QString &type, KIO::fileoffset_t startOffset, int pieceLength, KIO::filesize_t fileSize = 0, bool *abortPtr = 0);
    QStringList orderChecksumTypes(Verifier::ChecksumStrength strength) const;


    Verifier *q;

    QString dBusObjectPath;
    VerificationModel *model;
    KUrl dest;
    Verifier::VerificationStatus status;

    QHash<QString, PartialChecksums*> partialSums;

    mutable VerificationThread thread;

    static const QStringList SUPPORTED;
    static const QString MD5;
    static const int DIGGESTLENGTH[];
    static const int MD5LENGTH;
    static const int PARTSIZE;
};

#endif
