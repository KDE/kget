/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef METALINK_H
#define METALINK_H

#include <KIO/Job>

#include "core/datasourcefactory.h"
#include "core/transfer.h"

#include "ui/metalinkcreator/metalinker.h"

#ifdef HAVE_NEPOMUK
class MetaNepomukHandler;
#endif //HAVE_NEPOMUK

class Metalink : public Transfer
{
    Q_OBJECT

    public:
        Metalink(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                    const QDomElement * e = 0);

        ~Metalink();

        void init();

        void save(const QDomElement &element);
        void load(const QDomElement *e);

        /**
         * Reimplemented to return a time based on the average of the last three speeds
         */
        int remainingTime() const;

        bool repair(const KUrl &file = KUrl());

        /**
         * Move the download to the new destination
         * @param newDirectory is a directory where the download should be stored
         * @returns true if newDestination can be used
         */
        virtual bool setDirectory(const KUrl &newDirectory);

        QHash<KUrl, QPair<bool, int> > availableMirrors(const KUrl &file) const;
        void setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors);

        /**
         * @param file for which to get the verifier
         * @return Verifier that allows you to add checksums manually verify a file etc.
         */
        virtual Verifier *verifier(const KUrl &file);

        FileModel *fileModel();

    public Q_SLOTS:
        // --- Job virtual functions ---
        void start();
        void stop();

        void deinit();

        bool isResumable() const;

    private Q_SLOTS:
        void metalinkInit(const KUrl &url = KUrl(), const QByteArray &data = QByteArray());
        void filesSelected();
        void totalSizeChanged(KIO::filesize_t size);
        void processedSizeChanged();
        void speedChanged();
        void slotStatus(Job::Status status);
        void slotRename(const KUrl &oldUrl, const KUrl &newUrl);
        void slotVerified(bool isVerified);

    private :
        void startMetalink();
        QList<KUrl> files() const;

    private:
        FileModel *m_fileModel;
        int m_currentFiles;
        KUrl m_localMetalinkLocation;
        KGetMetalink::Metalink m_metalink;
        QHash<KUrl, DataSourceFactory*> m_dataSourceFactory;
        bool m_ready;
        int m_speedCount;
        int m_tempAverageSpeed;
        mutable int m_averageSpeed;
#ifdef HAVE_NEPOMUK
        MetaNepomukHandler *m_nepHandler;
#endif //HAVE_NEPOMUK
};

#endif
