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

#include "verifier_p.h"
#include "verificationmodel.h"
#include "../dbus/dbusverifierwrapper.h"
#include "verifieradaptor.h"
#include "settings.h"

#include <QtCore/QFile>
#include <QtCore/QScopedPointer>
#include <QtXml/QDomElement>

#include <KCodecs>
#include <KDebug>

#ifdef HAVE_QCA2
#include <QtCrypto>
#endif

//TODO use mutable to make some methods const?
const QStringList VerifierPrivate::SUPPORTED = (QStringList() << "sha512" << "sha384" << "sha256" << "ripmed160" << "sha1" << "md5" << "md4");
const QString VerifierPrivate::MD5 = QString("md5");
const int VerifierPrivate::DIGGESTLENGTH[] = {128, 96, 64, 40, 40, 32, 32};
const int VerifierPrivate::MD5LENGTH = 32;
const int VerifierPrivate::PARTSIZE = 500 * 1024;

VerifierPrivate::~VerifierPrivate()
{
    delete model;
    qDeleteAll(partialSums.begin(), partialSums.end());
}

QString VerifierPrivate::calculatePartialChecksum(QFile *file, const QString &type, KIO::fileoffset_t startOffset, int pieceLength, KIO::filesize_t fileSize, bool *abortPtr)
{
    if (!file)
    {
        return QString();
    }

    if (!fileSize)
    {
        fileSize = file->size();
    }
    //longer than the file, so adapt it
    if (static_cast<KIO::fileoffset_t>(fileSize) < startOffset + pieceLength)
    {
        pieceLength = fileSize - startOffset;
    }

#ifdef HAVE_QCA2
    QCA::Hash hash(type);

    //it can be that QCA2 does not support md5, e.g. when Qt is compiled locally
    KMD5 md5Hash;
    const bool useMd5 = (type == MD5);
#else //NO QCA2
    if (type != MD5)
    {
        return QString();
    }
    KMD5 hash;
#endif //HAVE_QCA2

    //we only read 512kb each time, to save RAM
    int numData = pieceLength / PARTSIZE;
    KIO::fileoffset_t dataRest = pieceLength % PARTSIZE;

    if (!numData && !dataRest)
    {
        QString();
    }

    int k = 0;
    for (k = 0; k < numData; ++k)
    {
        if (!file->seek(startOffset + PARTSIZE * k))
        {
            return QString();
        }

        if (abortPtr && *abortPtr)
        {
            return QString();
        }

        QByteArray data = file->read(PARTSIZE);
#ifdef HAVE_QCA2
        if (useMd5) {
            md5Hash.update(data);
        } else {
            hash.update(data);
        }
#else //NO QCA2
        hash.update(data);
#endif //HAVE_QCA2
    }

    //now read the rest
    if (dataRest)
    {
        if (!file->seek(startOffset + PARTSIZE * k))
        {
            return QString();
        }

        QByteArray data = file->read(dataRest);
#ifdef HAVE_QCA2
        if (useMd5) {
            md5Hash.update(data);
        } else {
            hash.update(data);
        }
#else //NO QCA2
        hash.update(data);
#endif //HAVE_QCA2
    }

#ifdef HAVE_QCA2
    return (useMd5 ? QString(md5Hash.hexDigest()) : QString(QCA::arrayToHex(hash.final().toByteArray())));
#else //NO QCA2
    return QString(hash.hexDigest());
#endif //HAVE_QCA2
}

QStringList VerifierPrivate::orderChecksumTypes(Verifier::ChecksumStrength strength) const
{
    QStringList checksumTypes;
    if (strength == Verifier::Weak) {
        for (int i = SUPPORTED.count() - 1; i >= 0; --i) {
            checksumTypes.append(SUPPORTED.at(i));
        }
        checksumTypes.move(0, 1); //md4 second position
    } else if (strength == Verifier::Strong) {
        for (int i = SUPPORTED.count() - 1; i >= 0; --i) {
            checksumTypes.append(SUPPORTED.at(i));
        }
        checksumTypes.move(1, checksumTypes.count() - 1); //md5 second last position
        checksumTypes.move(0, checksumTypes.count() - 1); //md4 last position
    } else if (strength == Verifier::Strongest) {
        checksumTypes = SUPPORTED;
    }

    return checksumTypes;
}

Verifier::Verifier(const KUrl &dest, QObject *parent)
  : QObject(parent),
    d(new VerifierPrivate(this))
{
    d->dest = dest;
    d->status = NoResult;

    static int dBusObjIdx = 0;
    d->dBusObjectPath = "/KGet/Verifiers/" + QString::number(dBusObjIdx++);

    DBusVerifierWrapper *wrapper = new DBusVerifierWrapper(this);
    new VerifierAdaptor(wrapper);
    QDBusConnection::sessionBus().registerObject(d->dBusObjectPath, wrapper);

    qRegisterMetaType<KIO::filesize_t>("KIO::filesize_t");
    qRegisterMetaType<KIO::fileoffset_t>("KIO::fileoffset_t");
    qRegisterMetaType<QList<KIO::fileoffset_t> >("QList<KIO::fileoffset_t>");

    d->model = new VerificationModel();
    connect(&d->thread, SIGNAL(verified(QString,bool,KUrl)), this, SLOT(changeStatus(QString,bool)));
    connect(&d->thread, SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)), this, SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)));
}

Verifier::~Verifier()
{
    delete d;
}

QString Verifier::dBusObjectPath() const
{
    return d->dBusObjectPath;
}

KUrl Verifier::destination() const
{
    return d->dest;
}

void Verifier::setDestination(const KUrl &destination)
{
    d->dest = destination;
}

Verifier::VerificationStatus Verifier::status() const
{
    return d->status;
}

VerificationModel *Verifier::model()
{
    return d->model;
}

QStringList Verifier::supportedVerficationTypes()
{
    QStringList supported;
#ifdef HAVE_QCA2
    QStringList supportedTypes = QCA::Hash::supportedTypes();
    for (int i = 0; i < VerifierPrivate::SUPPORTED.count(); ++i)
    {
        if (supportedTypes.contains(VerifierPrivate::SUPPORTED.at(i)))
        {
            supported << VerifierPrivate::SUPPORTED.at(i);
        }
    }
#endif //HAVE_QCA2

    if (!supported.contains(VerifierPrivate::MD5))
    {
        supported << VerifierPrivate::MD5;
    }

    return supported;

}

int Verifier::diggestLength(const QString &type)
{
    if (type == VerifierPrivate::MD5)
    {
        return VerifierPrivate::MD5LENGTH;
    }

#ifdef HAVE_QCA2
    if (QCA::isSupported(type.toLatin1()))
    {
        return VerifierPrivate::DIGGESTLENGTH[VerifierPrivate::SUPPORTED.indexOf(type)];
    }
#endif //HAVE_QCA2

    return 0;
}

bool Verifier::isChecksum(const QString &type, const QString &checksum)
{
    const int length = diggestLength(type);
    const QString pattern = QString("[0-9a-z]{%1}").arg(length);
    //needs correct length and only word characters
    if (length && (checksum.length() == length) && checksum.toLower().contains(QRegExp(pattern)))
    {
        return true;
    }

    return false;
}

QString Verifier::cleanChecksumType(const QString &type)
{
    QString hashType = type.toUpper();
    if (hashType.contains(QRegExp("^SHA\\d+"))) {
        hashType.insert(3, '-');
    }

    return hashType;
}

bool Verifier::isVerifyable() const
{
    return QFile::exists(d->dest.pathOrUrl()) && d->model->rowCount();
}

bool Verifier::isVerifyable(const QModelIndex &index) const
{
    int row = -1;
    if (index.isValid())
    {
        row = index.row();
    }
    if (QFile::exists(d->dest.pathOrUrl()) && (row >= 0) && (row < d->model->rowCount()))
    {
        return true;
    }
    return false;
}

Checksum Verifier::availableChecksum(Verifier::ChecksumStrength strength) const
{
    Checksum pair;

    //check if there is at least one entry
    QModelIndex index = d->model->index(0, 0);
    if (!index.isValid())
    {
        return pair;
    }

    const QStringList available = supportedVerficationTypes();
    const QStringList supported = d->orderChecksumTypes(strength);
    for (int i = 0; i < supported.count(); ++i) {
        QModelIndexList indexList = d->model->match(index, Qt::DisplayRole, supported.at(i));
        if (!indexList.isEmpty() && available.contains(supported.at(i))) {
            QModelIndex match = d->model->index(indexList.first().row(), VerificationModel::Checksum);
            pair.first = supported.at(i);
            pair.second = match.data().toString();
            break;
        }
    }

    return pair;
}

QList<Checksum> Verifier::availableChecksums() const
{
    QList<Checksum> checksums;

    for (int i = 0; i < d->model->rowCount(); ++i) {
        const QString type = d->model->index(i, VerificationModel::Type).data().toString();
        const QString hash = d->model->index(i, VerificationModel::Checksum).data().toString();
        checksums << qMakePair(type, hash);
    }

    return checksums;
}

QPair<QString, PartialChecksums*> Verifier::availablePartialChecksum(Verifier::ChecksumStrength strength) const
{
    QPair<QString, PartialChecksums*> pair;
    QString type;
    PartialChecksums *checksum = 0;

    const QStringList available = supportedVerficationTypes();
    const QStringList supported = d->orderChecksumTypes(strength);
    for (int i = 0; i < supported.size(); ++i) {
        if (d->partialSums.contains(supported.at(i)) && available.contains(supported.at(i))) {
            type = supported.at(i);
            checksum =  d->partialSums[type];
            break;
        }
    }

    return QPair<QString, PartialChecksums*>(type, checksum);
}

void Verifier::changeStatus(const QString &type, bool isVerified)
{
    kDebug(5001) << "Verified:" << isVerified;
    d->status = isVerified ? Verifier::Verified : Verifier::NotVerified;
    d->model->setVerificationStatus(type, d->status);
    emit verified(isVerified);
}

void Verifier::verify(const QModelIndex &index)
{
    int row = -1;
    if (index.isValid()) {
        row = index.row();
    }

    QString type;
    QString checksum;

    if (row == -1) {
        Checksum pair = availableChecksum(static_cast<Verifier::ChecksumStrength>(Settings::checksumStrength()));
        type = pair.first;
        checksum = pair.second;
    } else if ((row >= 0) && (row < d->model->rowCount())) {
        type = d->model->index(row, VerificationModel::Type).data().toString();
        checksum = d->model->index(row, VerificationModel::Checksum).data().toString();
    }

    d->thread.verifiy(type, checksum, d->dest);
}

void Verifier::brokenPieces() const
{
    QPair<QString, PartialChecksums*> pair = availablePartialChecksum(static_cast<Verifier::ChecksumStrength>(Settings::checksumStrength()));
    QList<QString> checksums;
    KIO::filesize_t length = 0;
    if (pair.second) {
        checksums = pair.second->checksums();
        length = pair.second->length();
    }
    d->thread.findBrokenPieces(pair.first, checksums, length, d->dest);
}

QString Verifier::checksum(const KUrl &dest, const QString &type, bool *abortPtr)
{
    QStringList supported = supportedVerficationTypes();
    if (!supported.contains(type))
    {
        return QString();
    }

    QFile file(dest.pathOrUrl());
    if (!file.open(QIODevice::ReadOnly))
    {
        return QString();
    }

    if (type == VerifierPrivate::MD5) {
        KMD5 hash;
        hash.update(file);
        QString final = QString(hash.hexDigest());
        file.close();
        return final;
    }


#ifdef HAVE_QCA2
    QCA::Hash hash(type);

    //BEGIN taken from qca_basic.h and slightly adopted to allow abort
    char buffer[1024];
    int len;

    while ((len=file.read(reinterpret_cast<char*>(buffer), sizeof(buffer))) > 0)
    {
        hash.update(buffer, len);
        if (abortPtr && *abortPtr)
        {
            hash.final();
            file.close();
            return QString();
        }
    }
    //END

    QString final = QString(QCA::arrayToHex(hash.final().toByteArray()));
    file.close();
    return final;
#endif //HAVE_QCA2

    return QString();
}

PartialChecksums Verifier::partialChecksums(const KUrl &dest, const QString &type, KIO::filesize_t length, bool *abortPtr)
{
    QStringList checksums;

    QStringList supported = supportedVerficationTypes();
    if (!supported.contains(type))
    {
        return PartialChecksums();
    }

    QFile file(dest.pathOrUrl());
    if (!file.open(QIODevice::ReadOnly))
    {
        return PartialChecksums();
    }

    const KIO::filesize_t fileSize = file.size();
    if (!fileSize)
    {
        return PartialChecksums();
    }

    int numPieces = 0;

    //the piece length has been defined
    if (length)
    {
        numPieces = fileSize / length;
    }
    else
    {
        length = VerifierPrivate::PARTSIZE;
        numPieces = fileSize / length;
        if (numPieces > 100)
        {
            numPieces = 100;
            length = fileSize / numPieces;
        }
    }

    //there is a rest, so increase numPieces by one
    if (fileSize % length)
    {
        ++numPieces;
    }

    PartialChecksums partialChecksums;

    //create all the checksums for the pieces
    for (int i = 0; i < numPieces; ++i)
    {
        QString hash = VerifierPrivate::calculatePartialChecksum(&file, type, length * i, length, fileSize, abortPtr);
        if (hash.isEmpty())
        {
            file.close();
            return PartialChecksums();
        }
        checksums.append(hash);
    }

    partialChecksums.setLength(length);
    partialChecksums.setChecksums(checksums);
    file.close();
    return partialChecksums;
}

void Verifier::addChecksum(const QString &type, const QString &checksum, int verified)
{
    d->model->addChecksum(type, checksum, verified);
}

void Verifier::addChecksums(const QHash<QString, QString> &checksums)
{
    d->model->addChecksums(checksums);
}

void Verifier::addPartialChecksums(const QString &type, KIO::filesize_t length, const QStringList &checksums)
{
    if (!d->partialSums.contains(type) && length && !checksums.isEmpty())
    {
        d->partialSums[type] = new PartialChecksums(length, checksums);
    }
}

KIO::filesize_t Verifier::partialChunkLength() const
{
    QStringList::const_iterator it;
    QStringList::const_iterator itEnd = VerifierPrivate::SUPPORTED.constEnd();
    for (it = VerifierPrivate::SUPPORTED.constBegin(); it != itEnd; ++it)
    {
        if (d->partialSums.contains(*it))
        {
            return d->partialSums[*it]->length();
        }
    }

    return 0;
}

void Verifier::save(const QDomElement &element)
{
    QDomElement e = element;
    e.setAttribute("verificationStatus", d->status);

    QDomElement verification = e.ownerDocument().createElement("verification");
    for (int i = 0; i < d->model->rowCount(); ++i)
    {
        QDomElement hash = e.ownerDocument().createElement("hash");
        hash.setAttribute("type", d->model->index(i, VerificationModel::Type).data().toString());
        hash.setAttribute("verified", d->model->index(i, VerificationModel::Verified).data(Qt::EditRole).toInt());
        QDomText value = e.ownerDocument().createTextNode(d->model->index(i, VerificationModel::Checksum).data().toString());
        hash.appendChild(value);
        verification.appendChild(hash);
    }

    QHash<QString, PartialChecksums*>::const_iterator it;
    QHash<QString, PartialChecksums*>::const_iterator itEnd = d->partialSums.constEnd();
    for (it = d->partialSums.constBegin(); it != itEnd; ++it)
    {
        QDomElement pieces = e.ownerDocument().createElement("pieces");
        pieces.setAttribute("type", it.key());
        pieces.setAttribute("length", (*it)->length());
        QList<QString> checksums = (*it)->checksums();
        for (int i = 0; i < checksums.size(); ++i)
        {
            QDomElement hash = e.ownerDocument().createElement("hash");
            hash.setAttribute("piece", i);
            QDomText value = e.ownerDocument().createTextNode(checksums[i]);
            hash.appendChild(value);
            pieces.appendChild(hash);
        }
        verification.appendChild(pieces);
    }
    e.appendChild(verification);
}

void Verifier::load(const QDomElement &e)
{
    if (e.hasAttribute("verificationStatus"))
    {
        const int status = e.attribute("verificationStatus").toInt();
        switch (status)
        {
            case NoResult:
                d->status = NoResult;
                break;
            case NotVerified:
                d->status = NotVerified;
                break;
            case Verified:
                d->status = Verified;
                break;
            default:
                d->status = NotVerified;
                break;
        }
    }

    QDomElement verification = e.firstChildElement("verification");
    QDomNodeList const hashList = verification.elementsByTagName("hash");

    for (uint i = 0; i < hashList.length(); ++i)
    {
        const QDomElement hash = hashList.item(i).toElement();
        const QString value = hash.text();
        const QString type = hash.attribute("type");
        const int verificationStatus = hash.attribute("verified").toInt();
        if (!type.isEmpty() && !value.isEmpty())
        {
            d->model->addChecksum(type, value, verificationStatus);
        }
    }

    QDomNodeList const piecesList = verification.elementsByTagName("pieces");

    for (uint i = 0; i < piecesList.length(); ++i)
    {
        QDomElement pieces = piecesList.at(i).toElement();

        const QString type = pieces.attribute("type");
        const KIO::filesize_t length = pieces.attribute("length").toULongLong();
        QStringList partialChecksums;

        const QDomNodeList partialHashList = pieces.elementsByTagName("hash");
        for (int i = 0; i < partialHashList.size(); ++i)//TODO give this function the size of the file, to calculate how many hashs are needed as an additional check, do that check in addPartialChecksums?!
        {
            const QString hash = partialHashList.at(i).toElement().text();
            if (hash.isEmpty())
            {
                break;
            }
            partialChecksums.append(hash);
        }

        addPartialChecksums(type, length, partialChecksums);
    }
}

#include "verifier.moc"
