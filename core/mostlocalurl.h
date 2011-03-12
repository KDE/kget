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

#ifndef KGET_MOSTLOCALURL_H
#define KGET_MOSTLOCALURL_H

#include "kget_export.h"
#include <KIO/Job>
#include <KUrl>

/**
 * NOTE this implementation does KIO::mostLocalUrl on any url whose protocol
 * is not supported by a TransferFactory.
 * I.e. it is assumed that an e.g. ftp url does _not_ point to a local file.
 *
 * The reason for that is to avoid connecting to external urls, which could
 * cause problems with websites that only allow one connection, or with
 * servers you connected already, reaching their maximum connections.
 * See #264452
 */

class MostLocalUrlJob;

/**
 * Synchronous
 */
KGET_EXPORT KUrl mostLocalUrl(const KUrl &url);

/**
 * Asynchronous
 */
KGET_EXPORT MostLocalUrlJob *mostLocalUrlJob(const KUrl &url);

/**
 * Job for asynchronously getting the most local url, do not use directly, but use
 * mostLocalUrlJob instead
 */
class KGET_EXPORT MostLocalUrlJob : public KIO::Job
{
    Q_OBJECT
    public:
        MostLocalUrlJob(const KUrl &url);

        virtual void start();
        KUrl url();

        /**
         * Call this in the slot connected to result.
         */
        KUrl mostLocalUrl() const;

    protected:
        virtual void slotResult(KJob *job);

    private:
        KUrl m_url;
        KUrl m_mostLocalUrl;
};

#endif
