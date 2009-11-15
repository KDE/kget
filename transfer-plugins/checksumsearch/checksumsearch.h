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

#ifndef CHECKSUMSEARCH_H
#define CHECKSUMSEARCH_H

#include <QtCore/QObject>

#include <KIO/Job>
#include <KUrl>

class ChecksumSearch : public QObject
{
    Q_OBJECT

    public:
        ChecksumSearch(const QList<KUrl> &srcs, const QString &fileName, const QStringList &types, QObject *parent = 0);
        ~ChecksumSearch();

        /**
         * Used to define in whiche way the url should be changed to try and find
         * Checksums
         */
        enum UrlChangeMode
        {
            kg_Append,     ///Appends the QString to the Url --> "http://test.com/aFile.zip"; ".md5" "http://test.com/aFile.zip.md5"
            kg_ReplaceFile,     ///Replaces the file of the Url with QString --> "http://test.com/aFile.zip"; "MD5SUMS" "http://test.com/MD5SUMS"
            kg_ReplaceEnding   ///Only replaces the file ending of the Url with QString --> "http://test.com/aFile.zip"; "-CHECKSUM" "http://test.com/aFile-CHECKSUM"
        };

        /**
         * Returns the available mode-names. The order of the Stringlist is the same as in the enum
         */
        static QStringList urlChangeModes() {return URLCHANGEMODES;}

        /**
         * Returns a modified url according to the parameters
         * @param src the url to modify
         * @param change the string containing the change e.g. ".md5"
         * @param mode the mode of the change e.g. Append
         */
        static KUrl createUrl(const KUrl &src, const QString &change, UrlChangeMode mode);

    Q_SIGNALS:
        void data(QString type, QString checksum);

    private slots:
        void slotResult(KJob *job);
        void slotData(KIO::Job *job, const QByteArray &data);

    private:
        /**
         * Creates a download
         */
        void createDownload();

        /**
         * Parses the download
         */
        void parseDownload();

        /**
         * Parses the download when no type has been specified
         */
        void parseDownloadEmpty();

    private:
        KIO::TransferJob *m_copyJob;
        KUrl m_src;
        QList<KUrl> m_srcs;
        QString m_fileName;
        QString m_type;
        QStringList m_types;
        QByteArray m_dataBA;
        QString m_data;
        bool m_isEmpty;
        static const QStringList URLCHANGEMODES;
};

#endif
