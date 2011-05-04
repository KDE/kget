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

#include "signaturethread.h"
#include "signature_p.h"

#include <KDebug>

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

bool SignatureThread::isValid() const
{
#ifdef HAVE_QGPGME
    return true;
#else //HAVE_QGPGME
    return false;
#endif //HAVE_QGPGME
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
#ifdef HAVE_QGPGME
    while (!m_abort && m_dest.count()) {
        m_mutex.lock();
        const KUrl dest = m_dest.takeFirst();
        const QByteArray sig = m_sig.takeFirst();
        m_mutex.unlock();

        GpgME::VerificationResult result = SignaturePrivate::verify(dest, sig);

        if (!m_abort) {
            emit verified(result);
        }
    }
#else //HAVE_QGPGME
    kWarning(5001) << "No QGPGME support.";
#endif //HAVE_QGPGME
}
