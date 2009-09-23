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

#include <QtCore/QFile>
#include <QtGui/QStandardItemModel>

#include <KCodecs>
#include <KComboBox>
#include <KDebug>
#include <KLocale>
#include <KLineEdit>
//TODO use mutable to make some methods const?
const QStringList Verifier::SUPPORTED = (QStringList() << "sha512" << "sha384" << "sha256" << "ripmed160" << "sha1" << "md5" << "md4");
const int Verifier::DIGGESTLENGTH[] = {128, 96, 64, 40, 40, 32, 32};
const int Verifier::MD5LENGTH = 32;
const int Verifier::PARTSIZE = 512 * 1024;

#ifdef HAVE_QCA2
static QCA::Initializer s_qcaInit;
#endif //HAVE_QCA2

static const QString s_md5 = QString("md5");

VerificationThread::VerificationThread(QObject *parent)
  : QThread(parent),
    abort(false),
    m_length(0),
    m_type(Nothing)
{
}

VerificationThread::~VerificationThread()
{
    mutex.lock();
    abort = true;
    mutex.unlock();

    wait();
}

void VerificationThread::verifiy(const QString &type, const QString &checksum, const KUrl &file)
{
    QMutexLocker locker(&mutex);
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
    QMutexLocker locker(&mutex);
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
    mutex.lock();
    bool run = m_files.count();
    mutex.unlock();

    while (run && !abort)
    {
        mutex.lock();
        const QString type = m_types.takeFirst();
        const QString checksum = m_checksums.takeFirst();
        const KUrl url = m_files.takeFirst();
        mutex.unlock();

        if (type.isEmpty() || checksum.isEmpty())
        {
            mutex.lock();
            run = m_files.count();
            mutex.unlock();
            continue;
        }

        const QString hash = Verifier::checksum(url, type, &abort);
        kDebug() << "Type:" << type << "Calculated checksum:" << hash << "Entered checksum:" << checksum;
        const bool fileVerified = (hash == checksum);

        if (abort)
        {
            return;
        }

        mutex.lock();
        if (!abort)
        {
            emit verified(type, fileVerified, url);
            emit verified(fileVerified);
        }
        run = m_files.count();
        mutex.unlock();
    }
}

void VerificationThread::doBrokenPieces()
{
    mutex.lock();
    const QString type = m_types.takeFirst();
    const QStringList checksums = m_checksums;
    m_checksums.clear();
    const KUrl url = m_files.takeFirst();
    const KIO::filesize_t length = m_length;
    mutex.unlock();

    QList<QPair<KIO::fileoffset_t, KIO::filesize_t> > broken;

    if (QFile::exists(url.pathOrUrl()))
    {
        QFile file(url.pathOrUrl());
        if (!file.open(QIODevice::ReadOnly))
        {
            emit brokenPieces(broken);
            return;
        }

        const KIO::filesize_t fileSize = file.size();
        if (!length || !fileSize)
        {
            emit brokenPieces(broken);
            return;
        }

        const QStringList fileChecksums = Verifier::partialChecksums(url, type, length, &abort).checksums();
        if (abort)
        {
            emit brokenPieces(broken);
            return;
        }

        if (fileChecksums.size() != checksums.size())
        {
            kDebug(5001) << "Number of checksums differs!";
            emit brokenPieces(broken);
            return;
        }

        for (int i = 0; i < checksums.size(); ++i)
        {
            if (fileChecksums.at(i) != checksums.at(i))
            {
                const int brokenStart = length * i;
                kDebug(5001) << url << "broken segment" << i << "start" << brokenStart << "length" << length;
                broken.append(QPair<KIO::fileoffset_t, KIO::filesize_t>(brokenStart, length));
            }
        }
    }

    emit brokenPieces(broken);
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
    if (index.isValid() && editor)
    {
        if (index.column() == VerificationModel::Type)
        {
            KComboBox *hashTypes = static_cast<KComboBox*>(editor);
            const QString hashType = index.model()->data(index).toString();
            hashTypes->setCurrentItem(hashType);
        }
        else if (index.column() == VerificationModel::Checksum)
        {
            KLineEdit *line = static_cast<KLineEdit*>(editor);
            const QString checksum = index.model()->data(index).toString();
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
    if (!index.isValid() || (role != Qt::DisplayRole))
    {
        return QVariant();
    }

    if (index.column() == Type)
    {
        return m_types.at(index.row());
    }
    else if (index.column() == Checksum)
    {
        return m_checksums.at(index.row());
    }

    return QVariant();
}

Qt::ItemFlags VerificationModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() && (index.row() >= rowCount()))
    {
        return 0;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.row() == VerificationModel::Type)
    {
        flags |= Qt::ItemIsEditable;
    }
    else if (index.row() == VerificationModel::Checksum)
    {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool VerificationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_types.count())
    {
        return false;
    }

    if ((index.column() == VerificationModel::Type) && role == Qt::EditRole)
    {
        const QString type = value.toString();
        if (Verifier::supportedVerficationTypes().contains(type) && !m_types.contains(type))
        {
            m_types[index.row()] = type;
            emit dataChanged(index, index);
            return true;
        }
    }
    else if ((index.column() == VerificationModel::Checksum) && role == Qt::EditRole)
    {
        const QModelIndex typeIndex = index.model()->index(index.row(), VerificationModel::Type);
        const QString type = index.model()->data(typeIndex).toString();
        const QString checksum = value.toString();
        if (Verifier::isChecksum(type, checksum))
        {

            m_checksums[index.row()] = checksum;
            emit dataChanged(index, index);
            return true;
        }
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

    return 2;
}

QVariant VerificationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation != Qt::Horizontal) || (role != Qt::DisplayRole))
    {
        return QVariant();
    }

    if (section == Type)
    {
        return i18nc("the type of the hash, e.g. MD5", "Type");
    }
    else if (section == Checksum)
    {
        return i18nc("the used hash for verification", "Hash");
    }

    return QVariant();
}

bool VerificationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || (row < 0) || (count < 1) || (row + count > rowCount()))
    {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    while (count)
    {
        m_types.removeAt(row);
        m_checksums.removeAt(row);
        --count;
    }
    endRemoveRows();

    return true;
}

void VerificationModel::addChecksum(const QString &type, const QString &checksum)
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
        if (position > -1)
        {
            m_checksums[position] = checksum;
            QModelIndex index = this->index(position, Checksum, QModelIndex());
            emit dataChanged(index, index);
            return;
        }

        int rows = rowCount();
        beginInsertRows(QModelIndex(), rows, rows);
        m_types.append(type);
        m_checksums.append(checksum);
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

Verifier::Verifier(const KUrl &dest)
  : m_dest(dest),
    m_status(NoResult)
{
    qRegisterMetaType<KIO::filesize_t>("KIO::filesize_t");
    qRegisterMetaType<KIO::fileoffset_t>("KIO::fileoffset_t");
    qRegisterMetaType<QList<QPair<KIO::fileoffset_t,KIO::filesize_t> > >("QList<QPair<KIO::fileoffset_t,KIO::filesize_t> >");

    m_model = new VerificationModel();
    connect(&m_thread, SIGNAL(verified(bool)), this, SLOT(changeStatus(bool)));
    connect(&m_thread, SIGNAL(brokenPieces(QList<QPair<KIO::fileoffset_t,KIO::filesize_t> >)), this, SIGNAL(brokenPieces(QList<QPair<KIO::fileoffset_t,KIO::filesize_t> >)));
}

Verifier::~Verifier()
{
    delete m_model;
    qDeleteAll(m_partialSums.begin(), m_partialSums.end());
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
#ifdef HAVE_QCA2
    if (QCA::isSupported(type.toLatin1()))
    {
        return DIGGESTLENGTH[SUPPORTED.indexOf(type)];
    }
#endif //HAVE_QCA2

    if (type == s_md5)
    {
        return MD5LENGTH;
    }

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
    if (QFile::exists(m_dest.pathOrUrl()) && m_model->rowCount())
    {
        return true;
    }

    return false;
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

QPair<QString, QString> Verifier::bestChecksum() const
{
    QPair<QString, QString> pair;

    //check if there is at least one entry
    QModelIndex index = m_model->index(0, 0);
    if (!index.isValid())
    {
        return pair;
    }

#ifdef HAVE_QCA2
    const QStringList supported = supportedVerficationTypes();
    for (int i = 0; i < supported.count(); ++i)
    {
        QModelIndexList indexList = m_model->match(index, Qt::DisplayRole, supported.at(i));
        //choose the "best" verification type, if it is supported by QCA
        if (!indexList.isEmpty())
        {
            QModelIndex match = m_model->index(indexList.first().row(), VerificationModel::Checksum);
            pair.first = supported.at(i);
            pair.second = match.data().toString();
            return pair;
        }
    }
#endif //HAVE_QCA2

    //use md5 provided by KMD5 as a fallback
    QModelIndexList indexList = m_model->match(index, 0, s_md5);
    if (!indexList.isEmpty())
    {
        QModelIndex match = m_model->index(indexList.first().row(), VerificationModel::Checksum);
        pair.first = s_md5;
        pair.second = match.data().toString();
        return pair;
    }

    return pair;
}

QPair<QString, PartialChecksums*> Verifier::bestPartialChecksums() const
{
    QPair<QString, PartialChecksums*> pair;
    QString type;
    PartialChecksums *checksum = 0;

    const QStringList supported = supportedVerficationTypes();
    #ifdef HAVE_QCA2
    for (int i = 0; i < supported.size(); ++i)
    {
        if (m_partialSums.contains(supported.at(i)))
        {
            type = supported.at(i);
        }
    }
    #else //NO QCA2
    if (m_partialSums.contains(s_md5))
    {
        type = s_md5;
    }
    #endif //HAVE_QCA2

    if (m_partialSums.contains(type))
    {
        checksum =  m_partialSums[type];
    }

    return QPair<QString, PartialChecksums*>(type, checksum);
}

void Verifier::changeStatus(bool isVerified)
{
    kDebug(5001) << "Verified:" << isVerified;
    m_status = isVerified ? Verified : NotVerified;
    emit verified(isVerified);
}

void Verifier::verify(const QModelIndex &index)
{
    int row = -1;
    if (index.isValid())
    {
        row = index.row();
    }

    QString type;
    QString checksum;

    if (row == -1)
    {
        QPair<QString, QString> pair = bestChecksum();
        type = pair.first;
        checksum = pair.second;
    }
    else if ((row >= 0) && (row < m_model->rowCount()))
    {
        type = m_model->index(row, VerificationModel::Type).data().toString();
        checksum = m_model->index(row, VerificationModel::Checksum).data().toString();
    }

    m_thread.verifiy(type, checksum, m_dest);
}

void Verifier::brokenPieces() const
{
    QPair<QString, PartialChecksums*> pair = bestPartialChecksums();
    QList<QString> checksums;
    KIO::filesize_t length = 0;
    if (pair.second)
    {
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
    if (type == s_md5)
    {
        KMD5 hash;
        hash.update(file);
        QString final = QString(hash.hexDigest());
        file.close();
        return final;
    }

    return QString();
}

PartialChecksums Verifier::partialChecksums(const KUrl &dest, const QString &type, KIO::filesize_t length, bool *abortPtr)
{
    QList<QString> checksums;

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
        hash.update(data);
    }

    //now read the rest
    if (dataRest)
    {
        if (!file->seek(startOffset + PARTSIZE * k))
        {
            return QString();
        }

        QByteArray data = file->read(dataRest);
        hash.update(data);
    }

#ifdef HAVE_QCA2
    return QString(QCA::arrayToHex(hash.final().toByteArray()));
#else //NO QCA2
    return QString(hash.hexDigest());
#endif //HAVE_QCA2
}

void Verifier::addPartialChecksums(const QString &type, KIO::filesize_t length, const QList<QString> &checksums)
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
        if (!type.isEmpty() && !value.isEmpty())
        {
            m_model->addChecksum(type, value);
        }
    }

    QDomNodeList const piecesList = verification.elementsByTagName("pieces");

    for (uint i = 0; i < piecesList.length(); ++i)
    {
        QDomElement pieces = piecesList.at(i).toElement();

        const QString type = pieces.attribute("type");
        const KIO::filesize_t length = pieces.attribute("length").toULongLong();
        QList<QString> partialChecksums;

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
