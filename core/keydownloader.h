/**************************************************************************
*   Copyright (C) 2009-2011 Matthias Fuchs <mat69@gmx.net>                *
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

#ifndef KEY_DOWNLOADER_H
#define KEY_DOWNLOADER_H

#include <QtCore/QMultiHash>
#include <QtCore/QObject>

class KJob;
class Signature;

/**
 * @class KeyDownloader
 *
 * @short Class to download Keys
 */
class KeyDownloader : public QObject
{
    Q_OBJECT

    public:
        KeyDownloader(QObject *parent = 0);

        /**
         * @return true if KeyDownloader is valid, i.e. has QPGME support
         */
        bool isValid() const;

        /**
         * Searches for a key with fignerprint
         * @param fingerprint the fingerprint of the key that is searched
         * @param sig Signature to notify of successful downloads
         */
        void downloadKey(QString fingerprint, Signature *sig);

    private slots:
        /**
         * Parses the downloaded data and if it is a key tries to add it to GnuPG,
         * if it is not a key try a different server.
         */
        void slotDownloaded(KJob *job);

    private:
        /**
         * Searches for a key with fignerprint
         * @param fingerprint the fingerprint of the key that is searched
         * @param sig Signature to notify of successful downloads
         * @param mirrorFailed if true another mirror will be tried
         */
        void downloadKey(QString fingerprint, Signature *sig, bool mirrorFailed);

    private:
        QMultiHash<QString, Signature*> m_downloading;
        QHash<KJob*, QString> m_jobs;
        QMultiHash<QString, QString> m_triedMirrors;
};

#endif
