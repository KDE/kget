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

#ifndef VERIFIER_H
#define VERIFIER_H

#include <kio/global.h>
#include <KUrl>

#include <QtCore/QModelIndex>
#include <QtCore/QHash>
#include <QtCore/QStringList>

#include "../kget_export.h"

class QDomElement;
class QFile;
class TransferHandler;
class VerificationModel;
class VerifierPrivate;
typedef QPair<QString, QString> Checksum;


class KGET_EXPORT PartialChecksums
{
    public:
        PartialChecksums()
          : m_length(0)
        {
        }

        PartialChecksums(KIO::filesize_t len, const QStringList &sums)
          : m_length(len), m_checksums(sums)
        {
        }

        bool isValid() const {return (length() && m_checksums.count());}

        KIO::filesize_t length() const {return m_length;}
        void setLength(KIO::filesize_t length) {m_length = length;}

        QStringList checksums() const {return m_checksums;}
        void setChecksums(const QStringList &checksums) {m_checksums = checksums;}

    private:
        KIO::filesize_t m_length;
        QStringList m_checksums;
};

Q_DECLARE_METATYPE(PartialChecksums)

class KGET_EXPORT Verifier : public QObject
{
    Q_OBJECT

    public:
        explicit Verifier(const KUrl &dest, QObject *parent = 0);
        ~Verifier();

        enum VerificationStatus
        {
            NoResult, //either not tried, or not enough information
            NotVerified,
            Verified
        };

        enum ChecksumStrength
        {
            Weak,
            Strong,
            Strongest
        };

        /**
         * @returns the object path that will be shown in the DBUS interface
         */
        QString dBusObjectPath() const;

        KUrl destination() const;
        void setDestination(const KUrl &destination);//TODO handle the case when m_thread is working, while the file gets moved

        VerificationStatus status() const;

        /**
         * Returns the supported verification types
         * @return the supported verification types (e.g. MD5, SHA1 ...)
         */
        static QStringList supportedVerficationTypes();

        /**
         * Returns the diggest length of type
         * @param type the checksum type for which to get the diggest length
         * @return the length the diggest should have
         */
        static int diggestLength(const QString &type);

        /**
         * Tries to check if the checksum is a checksum and if it is supported
         * it compares the diggestLength and checks if there are only alphanumerics in checksum
         * @param type of the checksum
         * @param checksum the checksum you want to check
         */
        static bool isChecksum(const QString &type, const QString &checksum);

        /**
         * Cleans the checksum type, that it should match the official name, i.e. upper case
         * e.g. SHA-1 instead of sha1
         */
        static QString cleanChecksumType(const QString &type);

        /**
         * Creates the checksum type of the file dest
         * @param abortPtr makes it possible to abort the calculation of the checksum from another thread
         */
        static QString checksum(const KUrl &dest, const QString &type, bool *abortPtr);

        /**
         * Create partial checksums of type for file dest
         * @param abortPtr makes it possible to abort the calculation of the checksums from another thread
         * @note the length of the partial checksum (if not defined = 0) is not less than 512 kb
         * and there won't be more partial checksums than 101
         */
        static PartialChecksums partialChecksums(const KUrl &dest, const QString &type, KIO::filesize_t length = 0, bool *abortPtr = 0);

        /**
         * @note only call verify() when this function returns true
         * @return true if the downloaded file exists and a supported checksum is set
         */
        bool isVerifyable() const;

        /**
         * Convenience function if only a row of the model should be checked
         * @note only call verify() when this function returns true
         * @param row the row in the model of the checksum
         * @return true if the downloaded file exists and a supported checksum is set
         */
        bool isVerifyable(const QModelIndex &index) const;

        /**
         * Call this method if you want to verify() in its own thread, then signals with
         * the result are emitted
         * @param row of the model should be checked, if not defined the a checkum defined by
         * Verifier::ChecksumStrength will be used
         */
        void verify(const QModelIndex &index = QModelIndex());

        /**
         * Call this method after calling verify() with a negative result, it will
         * emit a list of the broken pieces, if PartialChecksums were defined,
         * otherwise and in case of any error an empty list will be emitted
         */
        void brokenPieces() const;

        /**
         * Add a checksum that is later used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksum
         * @param checksum the checksum
         * @param verified if the file has been verified using this checksum
         * @note uses VerificationModel internally
         * @see VerificationModel
         */
        void addChecksum(const QString &type, const QString &checksum, int verified = 0);

        /**
         * Add multiple checksums that will later be used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param checksums <type, checksum>
         * @note uses VerificationModel internally
         * @see VerificationModel
         */
        void addChecksums(const QHash<QString, QString> &checksums);

        /**
         * Add partial checksums that can be used as repairinformation
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksums
         * @param length the length of each piece
         * @param checksums the checksums, first entry is piece number 0
         */
        void addPartialChecksums(const QString &type, KIO::filesize_t length, const QStringList &checksums);

        /**
         * Returns the length of the "best" partialChecksums
         */
        KIO::filesize_t partialChunkLength() const;

        /**
         * Returns a checksum and a type
         * @param strength the strength the checksum-type should have;
         * weak is md5 > md4 (md5 preferred), strong is sha1 > ripmed160 >
         * sha256 > sha384 > sha512 (sha1 preferred), strongest is sha512 >
         * sha384 > sha256 .... < (sha512 preferred)
         * If the category does not match then any checksum is taken
         */
        Checksum availableChecksum(ChecksumStrength strength) const;

        /**
         * Returns all set checksums
         */
        QList<Checksum> availableChecksums() const;

        /**
         * Returns a PartialChecksum and a type
         * @param strength the strength the checksum-type should have;
         * weak is md5 > md4 > ... (md5 preferred), strong is sha1 > ripmed160 >
         * sha256 > sha384 > sha512 > ... (sha1 preferred), strongest is sha512 >
         * sha384 > sha256 ... (sha512 preferred)
         * If the category does not match then any checksum is taken
         */
        QPair<QString, PartialChecksums*> availablePartialChecksum(Verifier::ChecksumStrength strength) const;

        /**
         * @return the model that stores the hash-types and checksums
         */
        VerificationModel *model();

        void save(const QDomElement &element);
        void load(const QDomElement &e);

    signals:
        /**
         * Emitted when the verification of a file finishes
         */
        void verified(bool verified);

        /**
         * Emitted when brokenPiecesThreaded finishes, the list can be empty, while length will be always set
         */
       void brokenPieces(const QList<KIO::fileoffset_t> &offsets, KIO::filesize_t length);

    private slots:
        void changeStatus(const QString &type, bool verified);

    private:
        VerifierPrivate *const d;

        friend class VerifierPrivate;
};

#endif //VERIFIER_H
