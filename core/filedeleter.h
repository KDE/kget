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

#ifndef KGET_FILE_DELETER_H
#define KGET_FILE_DELETER_H

#include "kget_export.h"

class KJob;
class KUrl;
class QObject;

/**
 * The FileDeleter is a wrapper around KIO ensuring that always
 * just one job is started for deleting a file.
 * Thus deleteFile can be called mutliple times safely and all callees
 * are informed once the file is actually deleted.
 */
class KGET_EXPORT FileDeleter
{
    public:
        FileDeleter();
        ~FileDeleter();

        /**
         * Starts the deletion of dest and emits KJob::finished once done.
         * You can safely call this method multiple times for the same destination.
         * @param dest destination to delete
         * @param receiver receiver of the finished signal
         * @param method method the finished signal should be connected to, thus
         *        informing you of the result
         * @return the KJob that has been created
         * @note only use the returned job to create connections yourself, not to modify it!
         */
        static KJob *deleteFile(const KUrl &dest, QObject *receiver = 0, const char *method = 0);

        /**
         * @return true if dest is being deleted
         */
        static bool isFileBeingDeleted(const KUrl &dest);

    private:
        class Private;
        Private *d;
};

#endif
