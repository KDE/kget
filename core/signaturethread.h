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

#ifndef SIGNATURE_THREAD_H
#define SIGNATURE_THREAD_H

#include <KUrl>

#include <QtCore/QMutex>
#include <QtCore/QThread>

#ifdef HAVE_QGPGME
#include <gpgme++/verificationresult.h>
#endif //HAVE_QGPGME

class SignatureThread : public QThread
{
    Q_OBJECT

    public:
        SignatureThread(QObject *parent = 0);
        ~SignatureThread();

        /**
         * @return true if the thread is valid, i.e. has QGPGME support
         */
        bool isValid() const;
        void verify(const KUrl &dest, const QByteArray &sig);

#ifdef HAVE_QGPGME
    signals:
        /**
         * Emitted when the verification of a file finishes, connect to this signal
         * if you do the verification for one file only and do not want to bother with
         * file and type
         */
        void verified(const GpgME::VerificationResult &result);
#endif //HAVE_QGPGME

    protected:
        void run();

    private:
        QMutex m_mutex;
        bool m_abort;
        QList<KUrl> m_dest;
        QList<QByteArray> m_sig;
};

#endif
