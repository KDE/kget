/***************************************************************************
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
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

#ifndef DBUSVERIFIERWRAPPER_H
#define DBUSVERIFIERWRAPPER_H

#include <QDBusVariant>
#include <kio/global.h>

class Verifier;

class DBusVerifierWrapper : public QObject
{
    Q_OBJECT
    public:
        DBusVerifierWrapper(Verifier *parent);
        ~DBusVerifierWrapper();

    public slots:
        /**
         * @return the dest url
         */
        QString destination() const;

        /**
         * Adds a checksum to the transfer
         */
        void addChecksum(const QString &type, const QString &hash);

        /**
         * Add partial checksums that can be used as repairinformation
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksums
         * @param length the length of each piece
         * @param checksums the checksums, first entry is piece number 0
         */
        void addPartialChecksums(const QString &type, qulonglong length, const QStringList &checksums);

        bool isVerifyable() const;

        void verify();

        /**
         * Call this method after calling verify() with a negative result, it will
         * emit a list of the broken pieces, if PartialChecksums were defined,
         * otherwise and in case of any error an empty list will be emitted
         */
        void brokenPieces() const;

    signals:
        /**
         * Emitted when the verification of a file finishes
         */
        void verified(bool verified);

        /**
         * Emitted when brokenPiecesThreaded finishes, the list can be empty
         * @param offsets of the broken pieces, they are the beginning
         * @param length of broken pieces
         */
        void brokenPieces(const QStringList &offsets, qulonglong length);

    private slots:
        void slotBrokenPieces(const QList<KIO::fileoffset_t> &offsets, KIO::filesize_t length);

    private:
        Verifier *m_verifier;
};

#endif
