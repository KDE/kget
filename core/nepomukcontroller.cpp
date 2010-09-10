/***************************************************************************
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
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

#include "nepomukcontroller.h"

#include <QtCore/QFile>
#include <Nepomuk/Tag>
#include <nepomuk/nepomukmassupdatejob.h>

NepomukController::NepomukController(QObject *parent)
  : QThread(parent)
{
}

NepomukController::~NepomukController()
{
    wait();
}

void NepomukController::setProperty(const QList<KUrl> &uris, QPair<QUrl, Nepomuk::Variant> &property, const QUrl &uriType)
{
    setProperties(uris, QList<QPair<QUrl, Nepomuk::Variant> >() << property, uriType);
}

void NepomukController::setProperties(const QList<KUrl> &uris, const QList<QPair<QUrl, Nepomuk::Variant> > &properties, const QUrl &uriType)
{
    if (uris.isEmpty() || properties.isEmpty()) {
        return;
    }

    QList<Nepomuk::Resource> resources;
    foreach (const KUrl &uri, uris) {
        resources << Nepomuk::Resource(uri, uriType);
    }
    Nepomuk::MassUpdateJob *job = new Nepomuk::MassUpdateJob(this);
    job->setResources(resources);
    job->setProperties(properties);
    job->start();
}

void NepomukController::addTags(const QList<KUrl> &uris, const QList<Nepomuk::Tag> &tags, const QUrl &uriType)
{
    if (uris.isEmpty() || tags.isEmpty()) {
        return;
    }

    QList<Nepomuk::Resource> resources;
    foreach (const KUrl &uri, uris) {
        resources << Nepomuk::Resource(uri, uriType);
    }

    Nepomuk::MassUpdateJob *job = Nepomuk::MassUpdateJob::tagResources(resources, tags);
    job->start();
}

void NepomukController::removeResource(const QList<KUrl> &uris)
{
    QMutexLocker locker(&m_mutex);
    m_uris << uris;

    if (!isRunning()) {
        start();
    }
}

bool NepomukController::continueToRun()
{
    QMutexLocker locker(&m_mutex);

    return !m_uris.isEmpty();
}


void NepomukController::run()
{
    while (continueToRun()) {
        m_mutex.lock();
        QList<KUrl> uris = m_uris;
        m_uris.clear();
        m_mutex.unlock();

        foreach (const KUrl &uri, uris) {
            if (!QFile::exists(uri.path())) {
                Nepomuk::Resource resource(uri, Nepomuk::Vocabulary::NFO::FileDataObject());
                resource.remove();
            }
        }
    }
}
