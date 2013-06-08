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
#include <Nepomuk2/DataManagement>
#include <Nepomuk2/Tag>
#include <Nepomuk2/Variant>
#include <Soprano/Vocabulary/NAO>

NepomukController::NepomukController(QObject *parent)
  : QThread(parent)
{
}

NepomukController::~NepomukController()
{
    wait();
}

void NepomukController::setProperty(const QList<QUrl> &uris, QPair<QUrl, Nepomuk2::Variant> &property, const QUrl &uriType)
{
    setProperties(uris, QList<QPair<QUrl, Nepomuk2::Variant> >() << property, uriType);
}

void NepomukController::setProperties(const QList<QUrl> &uris, const QList<QPair<QUrl, Nepomuk2::Variant> > &properties, const QUrl &uriType)
{
    if (uris.isEmpty() || properties.isEmpty()) {
        return;
    }

    for(QList<QPair<QUrl, Nepomuk2::Variant> >::ConstIterator i = properties.constBegin(); i < properties.constEnd(); i++) {
        Nepomuk2::addProperty(uris, i->first, QVariantList() << i->second.variant());
    }
}

void NepomukController::addTags(const QList<QUrl> &uris, const QList<Nepomuk2::Tag> &tags, const QUrl &uriType)
{
    QVariantList tagUris;

    if (uris.isEmpty() || tags.isEmpty()) {
        return;
    }

    foreach(Nepomuk2::Tag tag, tags) {
        tagUris.push_back(tag.uri());
    }

    Nepomuk2::addProperty(uris, Soprano::Vocabulary::NAO::hasTag(), tagUris);
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
                Nepomuk2::Resource resource(uri, Nepomuk2::Vocabulary::NFO::FileDataObject());
                resource.remove();
            }
        }
    }
}
