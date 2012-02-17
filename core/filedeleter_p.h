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

#ifndef FILEDELETER_P_H
#define FILEDELETER_P_H

#include <KIO/DeleteJob>
#include <KUrl>

#include <QtCore/QHash>
#include <QtCore/QObject>

class FileDeleter::Private : QObject
{
    Q_OBJECT
    public:
        Private();
        ~Private();

        KJob *deleteFile(const KUrl &dest, QObject *receiver, const char *method);

        bool isFileBeingDeleted(const KUrl &dest) const;

    public Q_SLOTS:
        void slotResult(KJob *job);

    private:
        QHash<KUrl, KJob*> m_jobs;
};

#endif // FILEDELETER_P_H
