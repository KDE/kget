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

#ifndef VERIFICATION_THREAD_H
#define  VERIFICATION_THREAD_H

#include <kio/global.h>
#include <KUrl>

#include <QtCore/QMutex>
#include <QtCore/QThread>

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

        void brokenPieces(const QList<KIO::fileoffset_t> &offsets, KIO::filesize_t length);

    protected:
        void run();

    private:
        QMutex m_mutex;
        bool m_abort;
        QStringList m_types;
        QStringList m_checksums;
        QList<KUrl> m_files;
        KIO::filesize_t m_length;
        WorkType m_type;
};

#endif
