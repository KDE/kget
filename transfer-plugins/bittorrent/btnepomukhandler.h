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

#ifndef BTNEPOMUKHANDLER_H
#define BTNEPOMUKHANDLER_H

#include "core/nepomukhandler.h"
#include <Nepomuk/Resource>

class BtNepomukHandler : public NepomukHandler
{
    Q_OBJECT
    public:
        BtNepomukHandler(Transfer *transfer);
        ~BtNepomukHandler();

        QStringList tags() const;
        int rating() const;

        /**
         * Set the destinations of the files that should be downloaded
         * @NOTE Use this method when changing the destination of the torrent,
         * or when the user decides to (not) download some files inside the
         * torrent
         * @param destinations a list of the destinations of the files being downloaded
         */
        void setDestinations(const QList<KUrl> &destinations);

        /**
         * from NepomukHandler, empty as BtNepomukHandler does use setDestinations
         */
        void setNewDestination(const KUrl &newDestination);

    public slots:
        void setRating(int rating);
        void addTag(const QString &newTag);
        void addTags(const QStringList &newTags);
        void removeTag(const QString &oldTag);
        void saveFileProperties();
        void postDeleteEvent();

    private:
        QList<KUrl> m_newDestinations;
        QHash<KUrl, Nepomuk::Resource> m_resources;
        Nepomuk::Resource m_tempResource;
};

#endif //BTNEPOMUKHANDLER_H
