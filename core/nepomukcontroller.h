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

#ifndef NEPOMUKCONTROLLER
#define NEPOMUKCONTROLLER

#include "../kget_export.h"

#include <Nepomuk/Vocabulary/NFO>

#include <QtCore/QMutex>
#include <QtCore/QThread>

#include <KUrl>

namespace Nepomuk {
    class Variant;
    class Tag;
}

/**
 * @class NepomukConroller works with Nepomuk, it can be used to set properties
 * on resources or to delete them. All of this is done using Nepomuk::MassUpdateJob,
 * only removeResource run in its own thread, since that does not exist in MassUpdateJob
 */
class KGET_EXPORT NepomukController : public QThread
{
    Q_OBJECT

    public:
        explicit NepomukController(QObject *parent = 0);
        ~NepomukController();

        void setProperty(const QList<KUrl> &uris, QPair<QUrl, Nepomuk::Variant> &property, const QUrl &uriType = Nepomuk::Vocabulary::NFO::FileDataObject());
        void setProperties(const QList<KUrl> &uris, const QList<QPair<QUrl, Nepomuk::Variant> > &properties, const QUrl &uriType = Nepomuk::Vocabulary::NFO::FileDataObject());
        void addTags(const QList<KUrl> &uris, const QList<Nepomuk::Tag> &tags, const QUrl &uriType = Nepomuk::Vocabulary::NFO::FileDataObject());

        /**
         * Removes the resources associated with uris
         * @note This assumes that uris points to local files and only removes the resource if those
         * files do not exist anymore
         */
        void removeResource(const QList<KUrl> &uris);

    protected:
        void run();

    private:
        bool continueToRun();

    private:
        QMutex m_mutex;
        QList<KUrl> m_uris;
};
#endif
