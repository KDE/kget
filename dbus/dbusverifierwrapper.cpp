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

#include "dbusverifierwrapper.h"
#include "core/verifier.h"
#include "core/verificationmodel.h"

DBusVerifierWrapper::DBusVerifierWrapper(Verifier *verifier)
  : QObject(verifier),
    m_verifier(verifier)
{
    connect(m_verifier, SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)), this, SLOT(slotBrokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)));
    connect(m_verifier, SIGNAL(verified(bool)), this, SIGNAL(verified(bool)));
}

DBusVerifierWrapper::~DBusVerifierWrapper()
{
}

QString DBusVerifierWrapper::destination() const
{
    return m_verifier->destination().pathOrUrl();
}

void DBusVerifierWrapper::addChecksum(const QString &type, const QString &hash)
{
    m_verifier->model()->addChecksum(type, hash);
}

void DBusVerifierWrapper::addPartialChecksums(const QString &type, qulonglong length, const QStringList &checksums)
{
    m_verifier->addPartialChecksums(type, length, checksums);
}

bool DBusVerifierWrapper::isVerifyable() const
{
    return m_verifier->isVerifyable();
}

void DBusVerifierWrapper::verify()
{
    m_verifier->verify();
}

void DBusVerifierWrapper::brokenPieces() const
{
    m_verifier->brokenPieces();
}

void DBusVerifierWrapper::slotBrokenPieces(const QList<KIO::fileoffset_t> &offsets, KIO::filesize_t length)
{
    //FIXME seems to work correct though is not correctly received at TestTransfers or maybe wrong converted
//     QList<QVariant> broken;
//     for (int i = 0; i < brokenPieces.count(); ++i) {
//         broken << brokenPieces[i];
//     }
// 
//     QDBusVariant dbusBroken;
//     dbusBroken.setVariant(QVariant(broken));
//     emit this->brokenPieces(dbusBroken);

    QStringList broken;
    for (int i = 0; i < offsets.count(); ++i) {
        broken << QString::number(offsets[i]);
    }

    emit brokenPieces(broken, length);
}


#include "dbusverifierwrapper.moc"
