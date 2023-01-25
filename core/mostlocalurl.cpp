/***************************************************************************
 *   Copyright (C) 2011 Matthias Fuchs <mat69@gmx.net>                     *
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

#include "mostlocalurl.h"
#include "kget.h"
#include "plugin/transferfactory.h"

#include "kget_debug.h"
#include <KIO/StatJob>
#include <QDebug>

QUrl mostLocalUrl(const QUrl &url)
{
    qCDebug(KGET_DEBUG);
    if (!url.isValid()) {
        qCDebug(KGET_DEBUG) << "Invalid URL: " << qUtf8Printable(url.toString());
        return url;
    }

    const QString protocol = url.scheme();
    if (protocol.isEmpty())
        return url;

    foreach (TransferFactory *factory, KGet::factories()) {
        if (factory->addsProtocols().contains(protocol)) {
            return url;
        }
    }

    qCDebug(KGET_DEBUG) << "Trying to find the most local URL for:" << url;
    KIO::StatJob *job = KIO::mostLocalUrl(url, KIO::HideProgressInfo);
    if (job->exec()) {
        return job->mostLocalUrl();
    };
    return url;
}

MostLocalUrlJob *mostLocalUrlJob(const QUrl &url)
{
    return new MostLocalUrlJob(url);
}

MostLocalUrlJob::MostLocalUrlJob(const QUrl &url)
    : KIO::Job()
    , m_url(url)
{
}

QUrl MostLocalUrlJob::url()
{
    return m_url;
}

QUrl MostLocalUrlJob::mostLocalUrl() const
{
    return m_mostLocalUrl;
}

void MostLocalUrlJob::start()
{
    bool startJob = true;
    const QString protocol = m_url.scheme();
    foreach (TransferFactory *factory, KGet::factories()) {
        if (factory->addsProtocols().contains(protocol)) {
            startJob = false;
            break;
        }
    }

    if (startJob) {
        qCDebug(KGET_DEBUG) << "Starting KIO::mostLocalUrl for:" << m_url;
        KIO::Job *job = KIO::mostLocalUrl(m_url, KIO::HideProgressInfo);
        addSubjob(job);
    } else {
        m_mostLocalUrl = m_url;
        emitResult();
    }
}

void MostLocalUrlJob::slotResult(KJob *job)
{
    if (job->error()) {
        qCWarning(KGET_DEBUG) << "Error" << job->error() << "happened for:" << m_url;
        m_mostLocalUrl = m_url;
    } else {
        m_mostLocalUrl = static_cast<KIO::StatJob *>(job)->mostLocalUrl();
    }
    qCDebug(KGET_DEBUG) << "Setting mostLocalUrl to" << m_mostLocalUrl;
    emitResult();
}
