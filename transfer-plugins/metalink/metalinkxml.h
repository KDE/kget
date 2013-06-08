/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2012 Aish Raj Dahal <dahalaishraj@gmail.com>

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
#include "abstractmetalink.h"

#include "ui/metalinkcreator/metalinker.h"


class MetalinkXml : public AbstractMetalink
{
    Q_OBJECT

    public:
        MetalinkXml(TransferGroup * parent, TransferFactory * factory,
                    Scheduler * scheduler, const KUrl & src, const KUrl & dest,
                    const QDomElement * e = 0);

        ~MetalinkXml();

        void save(const QDomElement &element);
        void load(const QDomElement *e);

    public Q_SLOTS:
        // --- Job virtual functions ---
        void start();
        void stop();

        void deinit(Transfer::DeleteOptions options);

    protected Q_SLOTS:
        /**
         * @return true if initialising worked
         * @note false does not mean that an error happened, it could mean, that the user
         * decided to update the metalink
         */
        bool metalinkInit(const KUrl &url = KUrl(), const QByteArray &data = QByteArray());

    protected :
        /**
         * @note downloads the metalink file, then starts the download
         */
        void downloadMetalink();
        void startMetalink();
        void untickAllFiles();

    private:
        bool m_metalinkJustDownloaded;
        KUrl m_localMetalinkLocation;
        KGetMetalink::Metalink m_metalink;

};

#endif
