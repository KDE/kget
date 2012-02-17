/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef TRANSFER_MULTISEGKIO_H
#define TRANSFER_MULTISEGKIO_H

#include <KIO/Job>

#include "core/transfer.h"
#include "core/datasourcefactory.h"

/**
 * This transfer uses multiple segments to download a file
 */

class DataSourceFactory;
class FileModel;

class TransferMultiSegKio : public Transfer
{
    Q_OBJECT

    public:
        TransferMultiSegKio(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                    const QDomElement * e = 0);

        bool repair(const KUrl &file = KUrl());

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        virtual bool setDirectory(const KUrl &newDirectory);

        virtual void init();
        virtual void deinit(Transfer::DeleteOptions options);

        QHash<KUrl, QPair<bool, int> > availableMirrors(const KUrl &file) const;
        void setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors);

        /**
         * @param file for which to get the verifier
         * @return Verifier that allows you to add checksums manually verify a file etc.
         */
        virtual Verifier *verifier(const KUrl &file = KUrl());

        /**
         * @param file for which to get the signature
         * @return Signature that allows you to add signatures and verify them
         */
        virtual Signature *signature(const KUrl &file = KUrl());

        FileModel *fileModel();

    public slots:
        bool setNewDestination(const KUrl &newDestination);
        // --- Job virtual functions ---
        void start();
        void stop();

        void save(const QDomElement &element);
        void load(const QDomElement *e);
        void slotChecksumFound(QString type, QString checksum);

    private:
        void createJob();

    private slots:
        void slotDataSourceFactoryChange(Transfer::ChangesFlags change);
        void slotUpdateCapabilities();
        void slotSearchUrls(const QList<KUrl> &urls);
        void slotRename(const KUrl &oldUrl, const KUrl &newUrl);
        void slotVerified(bool isVerified);

    private:
        bool m_movingFile;
        bool m_searchStarted;
        bool m_verificationSearch;
        DataSourceFactory *m_dataSourceFactory;
        FileModel *m_fileModel;
};

#endif
