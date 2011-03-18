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

#include <KDebug>
#include <KIO/NetAccess>

KUrl mostLocalUrl(const KUrl &url)
{
    kDebug(5001);
    const QString protocol = url.protocol();
    foreach (TransferFactory *factory, KGet::factories()) {
        if (factory->addsProtocols().contains(protocol)) {
            return url;
        }
    }

    kDebug(5001) << "Starting KIO::NetAccess::mostLocalUrl for:" << url;
    return KIO::NetAccess::mostLocalUrl(url, 0);
}

MostLocalUrlJob *mostLocalUrlJob(const KUrl &url)
{
    return new MostLocalUrlJob(url);
}

MostLocalUrlJob::MostLocalUrlJob(const KUrl& url)
  : KIO::Job(),
    m_url(url)
{
}

KUrl MostLocalUrlJob::url()
{
    return m_url;
}

KUrl MostLocalUrlJob::mostLocalUrl() const
{
    return m_mostLocalUrl;
}

void MostLocalUrlJob::start()
{
    bool startJob = true;
    const QString protocol = m_url.protocol();
    foreach (TransferFactory *factory, KGet::factories()) {
        if (factory->addsProtocols().contains(protocol)) {
            startJob = false;
            break;
        }
    }

    if (startJob) {
        kDebug(5001) << "Starting KIO::mostLocalUrl for:" << m_url;
        KIO::Job *job = KIO::mostLocalUrl(m_url, KIO::HideProgressInfo);
        addSubjob(job);
    } else {
        m_mostLocalUrl = m_url;
        emitResult();
    }
}

void MostLocalUrlJob::slotResult(KJob* job)
{
    if (job->error()) {
        kWarning(5001) << "Error" << job->error() << "happened for:" << m_url;
        m_mostLocalUrl = m_url;
    } else {
        m_mostLocalUrl = static_cast<KIO::StatJob*>(job)->mostLocalUrl();
    }
    kDebug(5001) << "Setting mostLocalUrl to" << m_mostLocalUrl;
    emitResult();
}
