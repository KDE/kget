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

#ifndef CHECKSUMSEARCHTRANSFERDATASOURCE_H
#define CHECKSUMSEARCHTRANSFERDATASOURCE_H

#include "core/transferdatasource.h"

#include <KIO/Job>

class ChecksumSearch;
class ChecksumSearchTransferDataSource;

/**
 * ChecksumSearchController downloads the baseUrl of files one want to get checksums for
 * e.g. www.example.com/directory/file.zip the baseUrl would be www.example.com/directory/
 * the result is cached so not downloaded multiple times
 */
class ChecksumSearchController : public QObject
{
    Q_OBJECT
    public:
        ChecksumSearchController(QObject *parent = 0);
        ~ChecksumSearchController();

        /**
         * Registers a search, downloads baseUrl if that has not be downloaded and then
         * calls gotBaseUrl for any ChecksumSearchTransferDataSource that was registered
         * for this baseUrl
         * @param search ChecksumSearchTransferDataSource to register to baseUrl
         * @param baseUrl that is being downloaded
         */
        void registerSearch(ChecksumSearchTransferDataSource *search, const KUrl &baseUrl);

        /**
         * Unregisters a search, do that e.g. if the search gets destroyed
         * @param search ChecksumSearchTransferDataSource to unregister to baseUrl
         * @param baseUrl can be empty, in that case search is unregistered for any url
         */
        void unregisterSearch(ChecksumSearchTransferDataSource *search, const KUrl &baseUrl = KUrl());

    private slots:
        void slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries);
        void slotResult(KJob *job);

    private:
        QMultiHash<KUrl, ChecksumSearchTransferDataSource*> m_searches;
        QHash<KUrl, KUrl> m_finished;
        QHash<KJob*, QPair<KUrl, KUrl> > m_jobs;
};

class ChecksumSearchTransferDataSource : public TransferDataSource
{
    Q_OBJECT
    public:
        ChecksumSearchTransferDataSource(const KUrl &srcUrl, QObject *parent);
        virtual ~ChecksumSearchTransferDataSource();

        void start();
        void stop();
        void addSegments(const QPair<KIO::fileoffset_t, KIO::fileoffset_t> &segmentSize, const QPair<int, int> &segmentRange);

    private:
        void gotBaseUrl(const KUrl &urlToFile);

    private:
        KUrl m_src;
        static ChecksumSearchController s_controller;

    friend class ChecksumSearchController;
};

#endif
