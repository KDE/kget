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
                    Scheduler * scheduler, const QUrl & src, const QUrl & dest,
                    const QDomElement * e = nullptr);

        bool repair(const QUrl &file = QUrl()) override;

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        virtual bool setDirectory(const QUrl &newDirectory) override;

        virtual void init() override;
        virtual void deinit(Transfer::DeleteOptions options) override;

        QHash<QUrl, QPair<bool, int> > availableMirrors(const QUrl &file) const override;
        void setAvailableMirrors(const QUrl &file, const QHash<QUrl, QPair<bool, int> > &mirrors) override;

        /**
         * @param file for which to get the verifier
         * @return Verifier that allows you to add checksums manually verify a file etc.
         */
        virtual Verifier *verifier(const QUrl &file = QUrl()) override;

        /**
         * @param file for which to get the signature
         * @return Signature that allows you to add signatures and verify them
         */
        virtual Signature *signature(const QUrl &file = QUrl()) override;

        FileModel *fileModel() override;

    public slots:
        bool setNewDestination(const QUrl &newDestination);
        // --- Job virtual functions ---
        void start() override;
        void stop() override;

        void save(const QDomElement &element) override;
        void load(const QDomElement *e) override;
        void slotChecksumFound(QString type, QString checksum);

    private:
        void createJob();

    private slots:
        void slotDataSourceFactoryChange(Transfer::ChangesFlags change);
        void slotUpdateCapabilities();
        void slotSearchUrls(const QList<QUrl> &urls);
        void slotRename(const QUrl &oldUrl, const QUrl &newUrl);
        void slotVerified(bool isVerified);
        void slotStatResult(KJob * kioJob);

    private:
        bool m_movingFile;
        bool m_searchStarted;
        bool m_verificationSearch;
        DataSourceFactory *m_dataSourceFactory;
        FileModel *m_fileModel;
};

#endif
