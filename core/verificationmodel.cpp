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

#include "verificationmodel.h"
#include "verifier.h"

#include <QtCore/QStringList>

#include <KDebug>
#include <KIcon>
#include <KLocale>

struct VerificationModelPrivate
{
    VerificationModelPrivate()
    {
    }

    ~VerificationModelPrivate()
    {
    }

    QStringList types;
    QStringList checksums;
    QList<int> verificationStatus;
};

VerificationModel::VerificationModel(QObject *parent)
  : QAbstractTableModel(parent),
    d(new VerificationModelPrivate)
{
}

QVariant VerificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if ((index.column() == VerificationModel::Type) && (role == Qt::DisplayRole)) {
        return d->types.at(index.row());
    } else if ((index.column() == VerificationModel::Checksum) && (role == Qt::DisplayRole)) {
        return d->checksums.at(index.row());
    } else if (index.column() == VerificationModel::Verified) {
        const int status = d->verificationStatus.at(index.row());
        if (role == Qt::DecorationRole) {
            switch (status) {
                case Verifier::Verified:
                    return KIcon("dialog-ok");
                case Verifier::NotVerified:
                    return KIcon("dialog-close");
                case Verifier::NoResult:
                default:
                    return KIcon();
            }
        } else if (role == Qt::EditRole) {
            return status;
        }
    }

    return QVariant();
}

Qt::ItemFlags VerificationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == VerificationModel::Type)
    {
        flags |= Qt::ItemIsEditable;
    }
    else if (index.column() == VerificationModel::Checksum)
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool VerificationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= d->types.count()) {
        return false;
    }

    if ((index.column() == VerificationModel::Type) && role == Qt::EditRole) {
        const QString type = value.toString();
        if (Verifier::supportedVerficationTypes().contains(type) && !d->types.contains(type)) {
            d->types[index.row()] = type;
            emit dataChanged(index, index);
            return true;
        }
    } else if ((index.column() == VerificationModel::Checksum) && role == Qt::EditRole) {
        const QModelIndex typeIndex = index.sibling(index.row(), VerificationModel::Type);
        const QString type = typeIndex.data().toString();
        const QString checksum = value.toString();
        if (Verifier::isChecksum(type, checksum)) {
            d->checksums[index.row()] = checksum;
            emit dataChanged(index, index);
            return true;
        }
    } else if (index.column() == VerificationModel::Verified && role == Qt::EditRole) {
        d->verificationStatus[index.row()] = value.toInt();
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

int VerificationModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return d->types.length();
}

int VerificationModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 3;
}

QVariant VerificationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole)) {
        return QVariant();
    }

    if (section == VerificationModel::Type) {
        return i18nc("the type of the hash, e.g. MD5", "Type");
    } else if (section == VerificationModel::Checksum) {
        return i18nc("the used hash for verification", "Hash");
    } else if (section == VerificationModel::Verified) {
        return i18nc("verification-result of a file, can be true/false", "Verified");
    }

    return QVariant();
}

bool VerificationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row < 0) || (count < 1) || (row + count > rowCount())) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    while (count) {
        d->types.removeAt(row);
        d->checksums.removeAt(row);
        d->verificationStatus.removeAt(row);
        --count;
    }
    endRemoveRows();

    return true;
}

void VerificationModel::addChecksum(const QString &type, const QString &checksum, int verified)
{
    if (!Verifier::isChecksum(type, checksum)) {
        kWarning(5001) << "Could not add checksum.\nType:" << type << "\nChecksum:" << checksum;
        return;
    }

    //if the hashtype already exists in the model, then replace it
    int position = d->types.indexOf(type);
    if (position > -1) {
        d->checksums[position] = checksum;
        const QModelIndex index = this->index(position, VerificationModel::Checksum, QModelIndex());
        emit dataChanged(index, index);
        return;
    }

    int rows = rowCount();
    beginInsertRows(QModelIndex(), rows, rows);
    d->types.append(type);
    d->checksums.append(checksum.toLower());
    d->verificationStatus.append(verified);
    endInsertRows();
}

void VerificationModel::addChecksums(const QHash<QString, QString> &checksums)
{
    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = checksums.constEnd();
    for (it = checksums.constBegin(); it != itEnd; ++it) {
        addChecksum(it.key(), it.value());
    }
}

void VerificationModel::setVerificationStatus(const QString &type, int verified)
{
    const int position = d->types.indexOf(type);
    if (position > -1) {
        d->verificationStatus[position] = verified;
        const QModelIndex index = this->index(position, VerificationModel::Verified, QModelIndex());
        emit dataChanged(index, index);
    }
}
