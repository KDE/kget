/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2012 Aish Raj Dahal <dahalaishraj@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/


#ifndef METALINKHTTP_H
#define METALINKHTTP_H

#include <KIO/Job>

#include "core/datasourcefactory.h"
#include "core/transfer.h"
#include "transfer-plugins/metalink/abstractmetalink.h"

#include "ui/metalinkcreator/metalinker.h"


class MetalinkHttp : public AbstractMetalink
{
    Q_OBJECT

    public:
    MetalinkHttp(TransferGroup * parent, TransferFactory * factory,
                Scheduler * scheduler, const QUrl & src, const QUrl & dest,
                KGetMetalink::MetalinkHttpParser *httpParser, const QDomElement * e = nullptr );
    ~MetalinkHttp() override;

    public Q_SLOTS:

    // --- Job virtual functions ---
        void start() override;
        void save(const QDomElement &element) override;
        void load(const QDomElement *element) override;
        void deinit(Transfer::DeleteOptions options) override;
        void slotSignatureVerified() override;

    private Q_SLOTS:
        /**
        * @return true if initialising worked
        * @note false does not mean that an error happened, it could mean, that the user
        * decided to update the metalink
        */
        bool metalinkHttpInit();

        /**
         * @note sets the signatures in the headers to the signature reader
         */

        void setSignature(QUrl & dest, QByteArray & data, DataSourceFactory* dataFactory);

        /**
         * @note sets the Instance Digests in the headers to the vlaues as per
         * the Instance digest in header
         */

        void setDigests();
        /**
         * @note sets the links in the headers to the vlaues as per
         * the Link in header
         */
        void setLinks();

    private:
        QUrl m_signatureUrl;
        QUrl m_metalinkxmlUrl ;
        void startMetalink() override;
        KGetMetalink::MetalinkHttpParser *m_httpparser;
        QList<KGetMetalink::HttpLinkHeader> m_linkheaderList;
        QHash<QString, QString> m_DigestList;

        /**
         * Needed to change the cases of hash type to be compatible with the Metalink XML types
         * @param Standard hash algorithm value in QString
         */
        static QString adaptDigestType(const QString& );
};

#endif //METALINKHTTP_H
