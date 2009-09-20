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

#ifndef VERIFIER_H
#define VERIFIER_H

#include <KIO/Job>
#include <KUrl>

#include <QtCore/QAbstractTableModel>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtGui/QStyledItemDelegate>
#include <QtXml/QDomElement>

#ifdef HAVE_QCA2
#include <QtCrypto>
#endif

#include "kget_export.h"

class QFile;
class QStandardItemModel;
class TransferHandler;

class VerificationThread : public QThread
{
    Q_OBJECT

    public:
        VerificationThread(QObject *parent = 0);
        ~VerificationThread();

        void verifiy(const QString &type, const QString &checksum, const KUrl &file);

        void findBrokenPieces(const QString &type, const QList<QString> checksums, KIO::filesize_t length, const KUrl &file);

    private:
        enum WorkType
        {
            Nothing,
            Verify,
            BrokenPieces
        };

        void doVerify();
        void doBrokenPieces();

    signals:
        /**
         * Emitted when the verification of a file finishes, connect to this signal
         * if you do the verification for one file only and do not want to bother with
         * file and type
         */
        void verified(bool verified);

        void verified(const QString &type, bool verified, const KUrl &file);

        void brokenPieces(QList<QPair<KIO::fileoffset_t, KIO::filesize_t> >);

    protected:
        void run();

    private:
        QMutex mutex;
        bool abort;
        QStringList m_types;
        QStringList m_checksums;
        QList<KUrl> m_files;
        KIO::filesize_t m_length;
        WorkType m_type;
};

class KGET_EXPORT VerificationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        VerificationDelegate(QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        QStringList m_hashTypes;
};

class KGET_EXPORT VerificationModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        VerificationModel(QObject *parent = 0);

        enum dataType
        {
            Type,
            Checksum
        };

        QVariant data(const QModelIndex &index, int role) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
        int rowCount(const QModelIndex &parent = QModelIndex()) const;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

        /**
         * Add a checksum that is later used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksum
         * @param checksum the checksum
         */
        void addChecksum(const QString &type, const QString &checksum);

        /**
         * Add multiple checksums that will later be used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param checksums <type, checksum>
         */
        void addChecksums(const QHash<QString, QString> &checksums);

    private:
        QStringList m_types;
        QStringList m_checksums;
};

class KGET_EXPORT PartialChecksums
{
    public:
        PartialChecksums()
          : m_length(0)
        {
        }

        PartialChecksums(KIO::filesize_t len, const QList<QString> &sums)
          : m_length(len), m_checksums(sums)
        {
        }

        bool isValid() const {return (length() && m_checksums.count());}

        KIO::filesize_t length() const {return m_length;}
        void setLength(KIO::filesize_t length) {m_length = length;}

        QList<QString> checksums() const {return m_checksums;}
        void setChecksums(const QList<QString> &checksums) {m_checksums = checksums;}

    private:
        KIO::filesize_t m_length;
        QList<QString> m_checksums;
};

class KGET_EXPORT Verifier : public QObject
{
    Q_OBJECT

    public:
        Verifier(const KUrl &m_dest);
        ~Verifier();

        enum VerificationStatus
        {
            NoResult, //either not tried, or not enough information
            NotVerified,
            Verified
        };

        KUrl destination() const {return m_dest;}
        void setDestination(const KUrl &destination) {m_dest = destination;}//TODO handle the case when m_thread is working, while the file gets moved

        VerificationStatus status() const {return m_status;}

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
         * Creates the checksum type of the file dest
         */
        static QString checksum(const KUrl &dest, const QString &type);

        /**
         * Create partial checksums of type for file dest
         * @note the length of the partial checksum (if not defined = 0) is not less than 512 kb
         * and there won't be more partial checksums than 101
         */
        static PartialChecksums partialChecksums(const KUrl &dest, const QString &type, KIO::filesize_t length = 0);

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
         * @param row of the model should be checked, if not defined the "best" checksum
         * available will be used
         */
        void verify(const QModelIndex &index = QModelIndex());

        /**
         * Call this method after calling verify() with a negative result, it will
         * emit a list of the broken pieces, if PartialChecksums were defined,
         * otherwise and in case of any error an empty list will be emitted
         */
        void brokenPieces() const;

        /**
         * Add partial checksums that can be used as repairinformation
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param type the type of the checksums
         * @param length the length of each piece
         * @param checksums the checksums, first entry is piece number 0
         */
        void addPartialChecksums(const QString &type, KIO::filesize_t length, const QList<QString> &checksums);

        /**
         * Returns the length of the "best" partialChecksums
         */
        KIO::filesize_t partialChunkLength() const;

        /**
         * Returns the "best" (strongest) stored checksum-type with the checksum
         */
        QPair<QString, QString> bestChecksum() const;

        /**
         * Returns the "best" (strongest) stored partial checksum-type with the checksum
         */
        QPair<QString, PartialChecksums*> bestPartialChecksums() const;

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
         * Emitted when brokenPiecesThreaded finishes, the list can be empty
         */
        void brokenPieces(QList<QPair<KIO::fileoffset_t, KIO::filesize_t> >);

    private slots:
        void changeStatus(bool verified);

    private:
        static QString calculatePartialChecksum(QFile *file, const QString &type, KIO::fileoffset_t startOffset, int pieceLength, KIO::filesize_t fileSize = 0);

    private:
        VerificationModel *m_model;
        KUrl m_dest;
        VerificationStatus m_status;

        QHash<QString, PartialChecksums*> m_partialSums;

        mutable VerificationThread m_thread;

        static const QStringList SUPPORTED;
        static const int DIGGESTLENGTH[];
        static const int MD5LENGTH;
        static const int PARTSIZE;
};

#endif //VERIFIER_H
