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

#include "verificationthread.h"
#include "verifier.h"

#include <KDebug>

#include <QtCore/QFile>

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
        kDebug(5001) << "Type:" << type << "Calculated checksum:" << hash << "Entered checksum:" << checksum;
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
