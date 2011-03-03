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

#include "verifier.h"
#include "../dbus/dbusverifierwrapper.h"
#include "verifieradaptor.h"
#include "settings.h"

#ifdef HAVE_QGPGME
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/importresult.h>
#include <gpgme++/key.h>
#include <qgpgme/dataprovider.h>
#endif

#include <QtCore/QFile>
#include <QtCore/QScopedPointer>
#include <QtGui/QStandardItemModel>

#include <KCodecs>
#include <KComboBox>
#include <KDebug>
#include <KIO/Job>
#include <KLocale>
#include <KLineEdit>
#include <KMessageBox>

#ifdef HAVE_QCA2
#include <QtCrypto>
#endif

#ifdef HAVE_QGPGME
K_GLOBAL_STATIC(KeyDownloader, signatureDownloader)
#endif //HAVE_QGPGME

//TODO use mutable to make some methods const?
const QStringList Verifier::SUPPORTED = (QStringList() << "sha512" << "sha384" << "sha256" << "ripmed160" << "sha1" << "md5" << "md4");
const int Verifier::DIGGESTLENGTH[] = {128, 96, 64, 40, 40, 32, 32};
const int Verifier::MD5LENGTH = 32;
const int Verifier::PARTSIZE = 500 * 1024;

static const QString s_md5 = QString("md5");

VerificationThread::VerificationThread(QObject *parent)
  : QThread(parent),
    m_abort(false),
    m_length(0),
    m_type(Nothing)
{
}

VerificationThread::~VerificationThread()
{
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();

    wait();
}

void VerificationThread::verifiy(const QString &type, const QString &checksum, const KUrl &file)
{
    QMutexLocker locker(&m_mutex);
    m_types.append(type);
    m_checksums.append(checksum);
    m_files.append(file);
    m_type = Verify;

    if (!isRunning())
    {
        start();
    }
}

void VerificationThread::findBrokenPieces(const QString &type, const QList<QString> checksums, KIO::filesize_t length, const KUrl &file)
{
    QMutexLocker locker(&m_mutex);
    m_types.clear();
    m_types.append(type);
    m_checksums = checksums;
    m_files.clear();
    m_files.append(file);
    m_length = length;
    m_type = BrokenPieces;

    if (!isRunning())
    {
        start();
    }
}

void VerificationThread::run()
{
    if (m_type == Nothing)
    {
        return;
    }

    if (m_type == Verify)
    {
        doVerify();
    }
    else if (m_type == BrokenPieces)
    {
        doBrokenPieces();
    }
}

void VerificationThread::doVerify()
{
    m_mutex.lock();
    bool run = m_files.count();
    m_mutex.unlock();

    while (run && !m_abort)
    {
        m_mutex.lock();
        const QString type = m_types.takeFirst();
        const QString checksum = m_checksums.takeFirst();
        const KUrl url = m_files.takeFirst();
        m_mutex.unlock();

        if (type.isEmpty() || checksum.isEmpty())
        {
            m_mutex.lock();
            run = m_files.count();
            m_mutex.unlock();
            continue;
        }

        const QString hash = Verifier::checksum(url, type, &m_abort);
        kDebug() << "Type:" << type << "Calculated checksum:" << hash << "Entered checksum:" << checksum;
        const bool fileVerified = (hash == checksum);

        if (m_abort)
        {
            return;
        }

        m_mutex.lock();
        if (!m_abort)
        {
            emit verified(type, fileVerified, url);
            emit verified(fileVerified);
        }
        run = m_files.count();
        m_mutex.unlock();
    }
}

void VerificationThread::doBrokenPieces()
{
    m_mutex.lock();
    const QString type = m_types.takeFirst();
    const QStringList checksums = m_checksums;
    m_checksums.clear();
    const KUrl url = m_files.takeFirst();
    const KIO::filesize_t length = m_length;
    m_mutex.unlock();

    QList<KIO::fileoffset_t> broken;

    if (QFile::exists(url.pathOrUrl()))
    {
        QFile file(url.pathOrUrl());
        if (!file.open(QIODevice::ReadOnly))
        {
            emit brokenPieces(broken, length);
            return;
        }

        const KIO::filesize_t fileSize = file.size();
        if (!length || !fileSize)
        {
            emit brokenPieces(broken, length);
            return;
        }

        const QStringList fileChecksums = Verifier::partialChecksums(url, type, length, &m_abort).checksums();
        if (m_abort)
        {
            emit brokenPieces(broken, length);
            return;
        }

        if (fileChecksums.size() != checksums.size())
        {
            kDebug(5001) << "Number of checksums differs!";
            emit brokenPieces(broken, length);
            return;
        }

        for (int i = 0; i < checksums.size(); ++i)
        {
            if (fileChecksums.at(i) != checksums.at(i))
            {
                const int brokenStart = length * i;
                kDebug(5001) << url << "broken segment" << i << "start" << brokenStart << "length" << length;
                broken.append(brokenStart);
            }
        }
    }

    emit brokenPieces(broken, length);
}

VerificationDelegate::VerificationDelegate(QObject *parent)
  : QStyledItemDelegate(parent),
    m_hashTypes(Verifier::supportedVerficationTypes())
{
    m_hashTypes.sort();
}

QWidget *VerificationDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (index.isValid())
    {
        if (index.column() == VerificationModel::Type)
        {
            if (m_hashTypes.count())
            {
                KComboBox *hashTypes = new KComboBox(parent);
                hashTypes->addItems(m_hashTypes);

                return hashTypes;
            }
        }
        else if (index.column() == VerificationModel::Checksum)
        {
            KLineEdit *line = new KLineEdit(parent);

            return line;
        }
    }

    return 0;
}

void VerificationDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.isValid() && editor) {
        if (index.column() == VerificationModel::Type) {
            KComboBox *hashTypes = static_cast<KComboBox*>(editor);
            const QString hashType = index.data().toString();
            hashTypes->setCurrentItem(hashType);
        } else if (index.column() == VerificationModel::Checksum) {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            const QString checksum = index.data().toString();
            line->setText(checksum);
        }
    }
}

void VerificationDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.isValid() && editor && model)
    {
        if (index.column() == VerificationModel::Type)
        {
            KComboBox *hashTypes = static_cast<KComboBox*>(editor);
            model->setData(index, hashTypes->currentText());
        }
        else if (index.column() == VerificationModel::Checksum)
        {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            model->setData(index, line->text());
        }
    }
}

void VerificationDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QSize VerificationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //make the sizeHint a little bit nicer to have more beautiful editors
    QSize hint;
    hint.setWidth(QStyledItemDelegate::sizeHint(option, index).width());
    hint.setHeight(option.fontMetrics.height() + 7);
    return hint;
}


VerificationModel::VerificationModel(QObject *parent)
  : QAbstractTableModel(parent)
{
}

QVariant VerificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if ((index.column() == VerificationModel::Type) && (role == Qt::DisplayRole)) {
        return m_types.at(index.row());
    } else if ((index.column() == VerificationModel::Checksum) && (role == Qt::DisplayRole)) {
        return m_checksums.at(index.row());
    } else if (index.column() == VerificationModel::Verified) {
        const int status = m_verificationStatus.at(index.row());
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
    if (!index.isValid() || index.row() >= m_types.count()) {
        return false;
    }

    if ((index.column() == VerificationModel::Type) && role == Qt::EditRole) {
        const QString type = value.toString();
        if (Verifier::supportedVerficationTypes().contains(type) && !m_types.contains(type)) {
            m_types[index.row()] = type;
            emit dataChanged(index, index);
            return true;
        }
    } else if ((index.column() == VerificationModel::Checksum) && role == Qt::EditRole) {
        const QModelIndex typeIndex = index.sibling(index.row(), VerificationModel::Type);
        const QString type = typeIndex.data().toString();
        const QString checksum = value.toString();
        if (Verifier::isChecksum(type, checksum)) {
            m_checksums[index.row()] = checksum;
            emit dataChanged(index, index);
            return true;
        }
    } else if (index.column() == VerificationModel::Verified && role == Qt::EditRole) {
        m_verificationStatus[index.row()] = value.toInt();
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

int VerificationModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_types.length();
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
        m_types.removeAt(row);
        m_checksums.removeAt(row);
        m_verificationStatus.removeAt(row);
        --count;
    }
    endRemoveRows();

    return true;
}

void VerificationModel::addChecksum(const QString &type, const QString &checksum, int verified)
{
    if (!type.isEmpty() && !checksum.isEmpty())
    {
        //check if the type is supported and if the diggestLength matches
        bool works = false;

#ifdef HAVE_QCA2
        if (QCA::isSupported(type.toLatin1()) && Verifier::isChecksum(type, checksum))
        {
            works = true;
        }
#endif //HAVE_QCA2

        if ((type == s_md5) && Verifier::isChecksum(type, checksum))
        {
            works = true;
        }

        if (!works)
        {
            return;
        }


        type.toLower();

        //if the hashtype already exists in the model, then replace it
        int position = m_types.indexOf(type);
        if (position > -1) {
            m_checksums[position] = checksum;
            const QModelIndex index = this->index(position, VerificationModel::Checksum, QModelIndex());
            emit dataChanged(index, index);
            return;
        }

        int rows = rowCount();
        beginInsertRows(QModelIndex(), rows, rows);
        m_types.append(type);
        m_checksums.append(checksum);
        m_verificationStatus.append(verified);
        endInsertRows();
    }
}

void VerificationModel::addChecksums(const QHash<QString, QString> &checksums)
{
    QHash<QString, QString>::const_iterator it;
    QHash<QString, QString>::const_iterator itEnd = checksums.constEnd();
    for (it = checksums.constBegin(); it != itEnd; ++it)
    {
        addChecksum(it.key(), it.value());
    }
}

void VerificationModel::setVerificationStatus(const QString &type, int verified)
{
    const int position = m_types.indexOf(type);
    if (position > -1) {
        m_verificationStatus[position] = verified;
        const QModelIndex index = this->index(position, VerificationModel::Verified, QModelIndex());
        emit dataChanged(index, index);
    }
}


Verifier::Verifier(const KUrl &dest, QObject *parent)
  : QObject(parent),
    m_dest(dest),
    m_status(NoResult)
{
    static int dBusObjIdx = 0;
    m_dBusObjectPath = "/KGet/Verifiers/" + QString::number(dBusObjIdx++);

    DBusVerifierWrapper *wrapper = new DBusVerifierWrapper(this);
    new VerifierAdaptor(wrapper);
    QDBusConnection::sessionBus().registerObject(m_dBusObjectPath, wrapper);

    qRegisterMetaType<KIO::filesize_t>("KIO::filesize_t");
    qRegisterMetaType<KIO::fileoffset_t>("KIO::fileoffset_t");
    qRegisterMetaType<QList<KIO::fileoffset_t> >("QList<KIO::fileoffset_t>");

    m_model = new VerificationModel();
    connect(&m_thread, SIGNAL(verified(QString,bool,KUrl)), this, SLOT(changeStatus(QString,bool)));
    connect(&m_thread, SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)), this, SIGNAL(brokenPieces(QList<KIO::fileoffset_t>,KIO::filesize_t)));
}

Verifier::~Verifier()
{
    delete m_model;
    qDeleteAll(m_partialSums.begin(), m_partialSums.end());
}

QString Verifier::dBusObjectPath() const
{
    return m_dBusObjectPath;
}

VerificationModel *Verifier::model()
{
    return m_model;
}

QStringList Verifier::supportedVerficationTypes()
{
    QStringList supported;
#ifdef HAVE_QCA2
    QStringList supportedTypes = QCA::Hash::supportedTypes();
    for (int i = 0; i < SUPPORTED.count(); ++i)
    {
        if (supportedTypes.contains(SUPPORTED.at(i)))
        {
            supported << SUPPORTED.at(i);
        }
    }
#endif //HAVE_QCA2

    if (!supported.contains(s_md5))
    {
        supported << s_md5;
    }

    return supported;

}

int Verifier::diggestLength(const QString &type)
{
    if (type == s_md5)
    {
        return MD5LENGTH;
    }

#ifdef HAVE_QCA2
    if (QCA::isSupported(type.toLatin1()))
    {
        return DIGGESTLENGTH[SUPPORTED.indexOf(type)];
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

bool Verifier::isVerifyable() const
{
    return QFile::exists(m_dest.pathOrUrl()) && m_model->rowCount();
}

bool Verifier::isVerifyable(const QModelIndex &index) const
{
    int row = -1;
    if (index.isValid())
    {
        row = index.row();
    }
    if (QFile::exists(m_dest.pathOrUrl()) && (row >= 0) && (row < m_model->rowCount()))
    {
        return true;
    }
    return false;
}

QStringList Verifier::orderChecksumTypes(Verifier::ChecksumStrength strength) const
{
    QStringList checksumTypes;
    if (strength == Weak) {
        for (int i = SUPPORTED.count() - 1; i >= 0; --i) {
            checksumTypes.append(SUPPORTED.at(i));
        }
        checksumTypes.move(0, 1); //md4 second position
    } else if (strength == Strong) {
        for (int i = SUPPORTED.count() - 1; i >= 0; --i) {
            checksumTypes.append(SUPPORTED.at(i));
        }
        checksumTypes.move(1, checksumTypes.count() - 1); //md5 second last position
        checksumTypes.move(0, checksumTypes.count() - 1); //md4 last position
    } else if (strength == Strongest) {
        checksumTypes = SUPPORTED;
    }

    return checksumTypes;
}

QPair<QString, QString> Verifier::availableChecksum(Verifier::ChecksumStrength strength) const
{
    QPair<QString, QString> pair;

    //check if there is at least one entry
    QModelIndex index = m_model->index(0, 0);
    if (!index.isValid())
    {
        return pair;
    }

    const QStringList available = supportedVerficationTypes();
    const QStringList supported = orderChecksumTypes(strength);
    for (int i = 0; i < supported.count(); ++i) {
        QModelIndexList indexList = m_model->match(index, Qt::DisplayRole, supported.at(i));
        if (!indexList.isEmpty() && available.contains(supported.at(i))) {
            QModelIndex match = m_model->index(indexList.first().row(), VerificationModel::Checksum);
            pair.first = supported.at(i);
            pair.second = match.data().toString();
            break;
        }
    }

    return pair;
}

QPair<QString, PartialChecksums*> Verifier::availablePartialChecksum(Verifier::ChecksumStrength strength) const
{
    QPair<QString, PartialChecksums*> pair;
    QString type;
    PartialChecksums *checksum = 0;

    const QStringList available = supportedVerficationTypes();
    const QStringList supported = orderChecksumTypes(strength);
    for (int i = 0; i < supported.size(); ++i) {
        if (m_partialSums.contains(supported.at(i)) && available.contains(supported.at(i))) {
            type = supported.at(i);
            checksum =  m_partialSums[type];
            break;
        }
    }

    return QPair<QString, PartialChecksums*>(type, checksum);
}

void Verifier::changeStatus(const QString &type, bool isVerified)
{
    kDebug(5001) << "Verified:" << isVerified;
    m_status = isVerified ? Verifier::Verified : Verifier::NotVerified;
    m_model->setVerificationStatus(type, m_status);
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
        QPair<QString, QString> pair = availableChecksum(static_cast<Verifier::ChecksumStrength>(Settings::checksumStrength()));
        type = pair.first;
        checksum = pair.second;
    } else if ((row >= 0) && (row < m_model->rowCount())) {
        type = m_model->index(row, VerificationModel::Type).data().toString();
        checksum = m_model->index(row, VerificationModel::Checksum).data().toString();
    }

    m_thread.verifiy(type, checksum, m_dest);
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
    m_thread.findBrokenPieces(pair.first, checksums, length, m_dest);
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

    if (type == s_md5) {
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
        length = PARTSIZE;
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
        QString hash = calculatePartialChecksum(&file, type, length * i, length, fileSize, abortPtr);
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

QString Verifier::calculatePartialChecksum(QFile *file, const QString &type, KIO::fileoffset_t startOffset, int pieceLength, KIO::filesize_t fileSize, bool *abortPtr)
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
    const bool useMd5 = (type == s_md5);
#else //NO QCA2
    if (type != s_md5)
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

void Verifier::addPartialChecksums(const QString &type, KIO::filesize_t length, const QStringList &checksums)
{
    if (!m_partialSums.contains(type) && length && !checksums.isEmpty())
    {
        m_partialSums[type] = new PartialChecksums(length, checksums);
    }
}

KIO::filesize_t Verifier::partialChunkLength() const
{
    QStringList::const_iterator it;
    QStringList::const_iterator itEnd = SUPPORTED.constEnd();
    for (it = SUPPORTED.constBegin(); it != itEnd; ++it)
    {
        if (m_partialSums.contains(*it))
        {
            return m_partialSums[*it]->length();
        }
    }

    return 0;
}

void Verifier::save(const QDomElement &element)
{
    QDomElement e = element;
    e.setAttribute("verificationStatus", m_status);

    QDomElement verification = e.ownerDocument().createElement("verification");
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QDomElement hash = e.ownerDocument().createElement("hash");
        hash.setAttribute("type", m_model->index(i, VerificationModel::Type).data().toString());
        hash.setAttribute("verified", m_model->index(i, VerificationModel::Verified).data(Qt::EditRole).toInt());
        QDomText value = e.ownerDocument().createTextNode(m_model->index(i, VerificationModel::Checksum).data().toString());
        hash.appendChild(value);
        verification.appendChild(hash);
    }

    QHash<QString, PartialChecksums*>::const_iterator it;
    QHash<QString, PartialChecksums*>::const_iterator itEnd = m_partialSums.constEnd();
    for (it = m_partialSums.constBegin(); it != itEnd; ++it)
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
                m_status = NoResult;
                break;
            case NotVerified:
                m_status = NotVerified;
                break;
            case Verified:
                m_status = Verified;
                break;
            default:
                m_status = NotVerified;
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
            m_model->addChecksum(type, value, verificationStatus);
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

Signature::Signature(const KUrl &dest, QObject *object)
  : QObject(object),
    m_dest(dest),
    m_verifyTried(false),
    m_status(NoResult),
    m_sigSummary(0),
    m_error(0)
{
#ifdef HAVE_QGPGME
    m_thread = new SignatureThread(this);

    qRegisterMetaType<GpgME::VerificationResult>("GpgME::VerificationResult");
    connect(m_thread, SIGNAL(verified(GpgME::VerificationResult)), this, SLOT(slotVerified(GpgME::VerificationResult)));
#endif //HAVE_QGPGME
}

KUrl Signature::destination() const
{
    return m_dest;
}

void Signature::setDestination(const KUrl &destination)
{
    m_dest = destination;
}

Signature::VerificationStatus Signature::status() const
{
    return m_status;
}

#ifdef HAVE_QGPGME
GpgME::VerificationResult Signature::verificationResult()
{
    return m_verificationResult;
}
#endif //HAVE_QGPGME

QString Signature::signature()
{
    return QString(m_signature);
}

void Signature::setSignature(const QString &signature)
{
    setSignature(signature.toAscii());
}

void Signature::setSignature(const QByteArray &signature)
{
    m_signature = signature;

    m_fingerprint.clear();
    m_error = 0;
    m_sigSummary = 0;
    m_status = Signature::NoResult;

#ifdef HAVE_QGPGME
    m_verificationResult = GpgME::VerificationResult();
#endif //HAVE_QGPGME

    emit verified(m_status);//FIXME
}

QString Signature::fingerprint()
{
    return m_fingerprint;
}

void Signature::downloadKey(QString fingerprint) // krazy:exclude=passbyvalue
{
#ifdef HAVE_QGPGME
    signatureDownloader->downloadKey(fingerprint, this);
#else
    Q_UNUSED(fingerprint)
#endif //HAVE_QGPGME
}

#ifdef HAVE_QGPGME
void Signature::signatureDownloaded()
{
    if (m_verifyTried) {
        kDebug(5001) << "Rerun verification.";
        verify();
    }
}
#endif //HAVE_QGPGME

bool Signature::isVerifyable()
{
#ifdef HAVE_QGPGME
    return QFile::exists(m_dest.pathOrUrl()) && !m_signature.isEmpty();
#else
    return false;
#endif //HAVE_QGPGME
}

#ifdef HAVE_QGPGME
GpgME::VerificationResult Signature::verify(const KUrl &dest, const QByteArray &sig)
{
    GpgME::VerificationResult result;
    if (!QFile::exists(dest.pathOrUrl()) || sig.isEmpty()) {
        return result;
    }

    GpgME::initializeLibrary();
    GpgME::Error error = GpgME::checkEngine(GpgME::OpenPGP);
    if (error) {
        kDebug(5001) << "OpenPGP not supported!";
        return result;
    }

    QScopedPointer<GpgME::Context> context(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (!context.data()) {
        kDebug(5001) << "Could not create context.";
        return result;
    }

    boost::shared_ptr<QFile> qFile(new QFile(dest.pathOrUrl()));
    qFile->open(QIODevice::ReadOnly);
    QGpgME::QIODeviceDataProvider *file = new QGpgME::QIODeviceDataProvider(qFile);
    GpgME::Data dFile(file);

    QGpgME::QByteArrayDataProvider signatureBA(sig);
    GpgME::Data signature(&signatureBA);

    return context->verifyDetachedSignature(signature, dFile);
}
#endif //HAVE_QGPGME

void Signature::verify()
{
#ifdef HAVE_QGPGME
    m_thread->verify(m_dest, m_signature);
#endif //HAVE_QGPGME
}

#ifdef HAVE_QGPGME
void Signature::slotVerified(const GpgME::VerificationResult &result)
{
    m_verificationResult = result;
    m_status = Signature::NotWorked;

    if (!m_verificationResult.numSignatures()) {
        kDebug(5001) << "No signatures\n";
        emit verified(m_status);
        return;
    }

    GpgME::Signature signature = m_verificationResult.signature(0);
    m_sigSummary = signature.summary();
    m_error = signature.status().code();
    m_fingerprint = signature.fingerprint();

    if (m_sigSummary & GpgME::Signature::KeyMissing) {
        if (Settings::signatureAutomaticDownloading() ||
            (KMessageBox::warningYesNoCancel(0,
             i18n("The key to verify the signature is missing, do you want to download it?")) == KMessageBox::Yes)) {
            m_verifyTried = true;
            emit verified(m_status);
            downloadKey(m_fingerprint);
            return;
        }
    }

    if (!signature.status()) {
        if (m_sigSummary == GpgME::Signature::Valid) {
            m_status = Signature::Verified;
        } else if ((m_sigSummary & GpgME::Signature::Green) || (m_sigSummary == 0)) {
            m_status = Signature::VerifiedInformation;
        }
    } else if (signature.status() && (m_sigSummary & GpgME::Signature::Red)) {//TODO handle more cases!
        m_status = Signature::NotVerified;
        //TODO handle that dialog better in 4.5
        KMessageBox::error(0,
                           i18n("The signature could not be verified for %1. See transfer settings for more information.", m_dest.fileName()),
                           i18n("Signature not verified"));
    }

    emit verified(m_status);
}
#endif //HAVE_QGPGME

void Signature::save(const QDomElement &element)
{
    QDomElement e = element;

    QDomElement verification = e.ownerDocument().createElement("signature");
    verification.setAttribute("status", m_status);
    verification.setAttribute("sigStatus", m_sigSummary);
    verification.setAttribute("error", m_error);
    verification.setAttribute("fingerprint", m_fingerprint);
    QDomText value = e.ownerDocument().createTextNode(m_signature);
    verification.appendChild(value);

    e.appendChild(verification);
}

void Signature::load(const QDomElement &e)
{
    QDomElement verification = e.firstChildElement("signature");
    m_status = static_cast<VerificationStatus>(verification.attribute("status").toInt());
    m_sigSummary = verification.attribute("sigStatus").toInt();
    m_error = verification.attribute("error").toInt();
    m_fingerprint = verification.attribute("fingerprint");
    m_signature = verification.text().toAscii();
}

#ifdef HAVE_QGPGME
SignatureThread::SignatureThread(QObject *parent)
  : QThread(parent),
    m_abort(false)
{
}

SignatureThread::~SignatureThread()
{
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();

    wait();
}

void SignatureThread::verify(const KUrl &dest, const QByteArray &sig)
{
    QMutexLocker locker(&m_mutex);
    m_dest.append(dest);
    m_sig.append(sig);

    if (!isRunning()) {
        start();
    }
}

void SignatureThread::run()
{
    while (!m_abort && m_dest.count()) {
        m_mutex.lock();
        const KUrl dest = m_dest.takeFirst();
        const QByteArray sig = m_sig.takeFirst();
        m_mutex.unlock();

        GpgME::VerificationResult result = Signature::verify(dest, sig);

        if (!m_abort) {
            emit verified(result);
        }
    }
}
#endif //HAVE_QGPGME

#ifdef HAVE_QGPGME
KeyDownloader::KeyDownloader(QObject *parent)
  : QObject(parent)
{
}

void KeyDownloader::downloadKey(QString fingerprint, Signature *sig)
{
    downloadKey(fingerprint, sig, false);
}

void KeyDownloader::downloadKey(QString fingerprint, Signature *sig, bool mirrorFailed)
{
    if (fingerprint.isEmpty() || (!sig && !mirrorFailed)) {
        return;
    }

    if (!fingerprint.startsWith(QLatin1String("0x"))) {
        fingerprint = "0x" + fingerprint;
    }

    if (m_downloading.contains(fingerprint) && !mirrorFailed) {
        if (!m_downloading.contains(fingerprint, sig)) {
            m_downloading.insert(fingerprint, sig);
        }
    } else {
        const QStringList servers = Settings::signatureKeyServers();
        if (!servers.count()) {
            KMessageBox::error(0,
                               i18n("No server for downloading keys is specified in settings. Downloading aborted."),
                               i18n("No key server"));
            return;
        }

        QString mirror;
        if (mirrorFailed) {
            const QStringList failedMirrors = m_triedMirrors.values(fingerprint);
            for (int i = 0; i < servers.count(); ++i) {
                if (!m_triedMirrors.contains(fingerprint, servers.at(i))) {
                    mirror = servers.at(i);
                    break;
                }
            }
        } else {
             mirror = servers.first();
        }

        if (mirror.isEmpty()) {
            KMessageBox::error(0,
                               i18n("No useful key server found, key not downloaded. Add more servers to the settings or restart KGet and retry downloading."),
                               i18n("No key server"));
           return;
        }

        m_triedMirrors.insert(fingerprint, mirror);
        if (!mirrorFailed) {
            m_downloading.insert(fingerprint, sig);
        }

        KUrl url(mirror);
        url.addPath("pks/lookup");
        url.setQuery("op=get&options=mr&search=" + fingerprint);
        url.setPort(11371);

        kDebug(5001) << "Dowloading:" << url;

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        m_jobs[job] = fingerprint;
        connect(job, SIGNAL(finished(KJob*)), this, SLOT(slotDownloaded(KJob*)));
    }
}

void KeyDownloader::slotDownloaded(KJob *job)
{
    if (!m_jobs.contains(job)) {
        return;
    }

    const QString fingerprint = m_jobs[job];
    KIO::StoredTransferJob *transferJob = static_cast<KIO::StoredTransferJob*>(job);

    if (transferJob->isErrorPage()) {
        kDebug(5001) << "Mirror did not work, try another one.";
        downloadKey(fingerprint, 0, true);
        return;
    }


    QByteArray data = transferJob->data();
    if (data.isEmpty()) {
        kDebug(5001) << "Downloaded data is empty.";
        downloadKey(fingerprint, 0, true);
        return;
    }

    const int indexStart = data.indexOf("<pre>");
    const int indexEnd = data.indexOf("</pre>", indexStart);
    if ((indexStart == -1) || (indexEnd == -1)) {
        kDebug(5001) << "Could not find a key.";
        downloadKey(fingerprint, 0, true);
        return;
    }

    data = data.mid(indexStart + 6, indexEnd - indexStart - 6);

    GpgME::initializeLibrary();
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err) {
        kDebug(5001) << "Problem checking the engine.";
        return;
    }

    QScopedPointer<GpgME::Context> context(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (!context.data()) {
        kDebug(5001) << "Could not create context.";
        return;
    }

    QGpgME::QByteArrayDataProvider keyBA(data);
    GpgME::Data key(&keyBA);
    GpgME::ImportResult importResult = context->importKeys(key);
    err = importResult.error();
    if (err) {
        kDebug(5001) << "Error while importing key.";;
        return;
    }

    kDebug(5001) << "Key downloaded, notifying requesters.";

    QList<Signature*> sigs = m_downloading.values(fingerprint);
    foreach (Signature *sig, sigs) {
        sig->signatureDownloaded();
    }
    m_downloading.remove(fingerprint);
}
#endif //HAVE_QGPGME

#include "verifier.moc"
