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

#ifndef VERIFICATION_MODEL_H
#define VERIFICATION_MODEL_H

#include "../kget_export.h"

#include <QtCore/QAbstractTableModel>

class VerificationModelPrivate;

class KGET_EXPORT VerificationModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        VerificationModel(QObject *parent = 0);

        enum dataType
        {
            Type,
            Checksum,
            Verified
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
         * @param verified if the file has been verified using this checksum
         */
        void addChecksum(const QString &type, const QString &checksum, int verified = 0);

        /**
         * Add multiple checksums that will later be used in the verification process
         * @note only one checksum per type can be added (one MD5, one SHA1 etc.),
         * the newer overwrites the older and a checksum can only be added if it is
         * supported by the verifier
         * @param checksums <type, checksum>
         */
        void addChecksums(const QHash<QString, QString> &checksums);

        /**
         * Sets the verificationStatus for type
         */
        void setVerificationStatus(const QString &type, int verified);

    private:
        VerificationModelPrivate *d;

        friend class VerificationModelPrivate;
};

#endif
