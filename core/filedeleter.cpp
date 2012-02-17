/**************************************************************************
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

#include "filedeleter.h"
#include "filedeleter_p.h"

K_GLOBAL_STATIC(FileDeleter, fileDeleter)

FileDeleter::Private::Private()
  : QObject(0)
{
}

FileDeleter::Private::~Private()
{
}

bool FileDeleter::Private::isFileBeingDeleted(const KUrl &dest) const
{
    return m_jobs.contains(dest);
}

KJob *FileDeleter::Private::deleteFile(const KUrl &dest, QObject *receiver, const char *method)
{
    QHash<KUrl, KJob*>::iterator it = m_jobs.find(dest);
    if (it == m_jobs.end()) {
        KJob *job = KIO::del(dest, KIO::HideProgressInfo);
        it = m_jobs.insert(dest, job);
        connect(*it, SIGNAL(result(KJob*)), this, SLOT(slotResult(KJob*)));
    }

    if (receiver && method) {
        //make sure that there is just one connection
        disconnect(*it, SIGNAL(result(KJob*)), receiver, method);
        connect(*it, SIGNAL(result(KJob*)), receiver, method);
    }

    return *it;
}

void FileDeleter::Private::slotResult(KJob *job)
{
    KIO::DeleteJob *deleteJob = static_cast<KIO::DeleteJob*>(job);
    m_jobs.remove(deleteJob->urls().first());
}

FileDeleter::FileDeleter()
  : d(new Private)
{
}

FileDeleter::~FileDeleter()
{
    delete d;
}

KJob *FileDeleter::deleteFile(const KUrl &dest, QObject *receiver, const char *method)
{
    return fileDeleter->d->deleteFile(dest, receiver, method);
}

bool FileDeleter::isFileBeingDeleted(const KUrl &dest)
{
    return fileDeleter->d->isFileBeingDeleted(dest);
}

#include "filedeleter.moc"
#include "filedeleter_p.moc"
