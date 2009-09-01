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

#include "checksumsearchtransferdatasource.h"
#include "checksumsearch.h"
#include "checksumsearchsettings.h"

#include <KDebug>

ChecksumSearchTransferDataSource::ChecksumSearchTransferDataSource(const KUrl &srcUrl)
  : TransferDataSource(srcUrl, 0)
{
}

ChecksumSearchTransferDataSource::~ChecksumSearchTransferDataSource()
{
    qDeleteAll(m_jobs);
}

void ChecksumSearchTransferDataSource::start()
{
    kDebug(5001);

    if (m_jobs.isEmpty())
    {
        QStringList changes = ChecksumSearchSettings::self()->searchStrings();
        QList<int> modes = ChecksumSearchSettings::self()->urlChangeModeList();
        QStringList types = ChecksumSearchSettings::self()->checksumTypeList();

        for (int i = 0; i < changes.size(); ++i)
        {
            const ChecksumSearch::UrlChangeMode mode = static_cast<ChecksumSearch::UrlChangeMode>(modes.at(i));
            const KUrl source = ChecksumSearch::createUrl(m_sourceUrl, changes.at(i), mode);

            if (!m_jobs.contains(source))
            {
                ChecksumSearch *search = new ChecksumSearch(source, m_sourceUrl.fileName(), types.at(i));

                connect(search, SIGNAL(finished(KUrl)), this, SLOT(slotFinished(KUrl)));
                connect(search, SIGNAL(data(QString, QString)), this, SIGNAL(data(QString,QString)));

                m_jobs[source] = search;
            }
        }
    }
}

void ChecksumSearchTransferDataSource::stop()
{
    kDebug(5001);
}

void ChecksumSearchTransferDataSource::addSegment(const KIO::fileoffset_t offset, const KIO::fileoffset_t bytes, int segmentNum)
{
    Q_UNUSED(offset);
    Q_UNUSED(bytes);
    Q_UNUSED(segmentNum);
    kDebug(5001);
}

void ChecksumSearchTransferDataSource::slotFinished(const KUrl &src)
{
    ChecksumSearch *search = m_jobs[src];
    m_jobs.remove(src);
    delete search;
}

#include "checksumsearchtransferdatasource.moc"
