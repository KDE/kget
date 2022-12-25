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

#include "abstractmetalink.h"
#include "core/datasourcefactory.h"
#include "core/transfer.h"

#include "ui/metalinkcreator/metalinker.h"

class MetalinkXml : public AbstractMetalink
{
    Q_OBJECT

public:
    MetalinkXml(TransferGroup *parent, TransferFactory *factory, Scheduler *scheduler, const QUrl &src, const QUrl &dest, const QDomElement *e = nullptr);

    ~MetalinkXml() override;

    void save(const QDomElement &element) override;
    void load(const QDomElement *e) override;

public Q_SLOTS:
    // --- Job virtual functions ---
    void start() override;

    void deinit(Transfer::DeleteOptions options) override;

protected Q_SLOTS:
    /**
     * @return true if initialising worked
     * @note false does not mean that an error happened, it could mean, that the user
     * decided to update the metalink
     */
    bool metalinkInit(const QUrl &url = QUrl(), const QByteArray &data = QByteArray());

protected:
    /**
     * @note downloads the metalink file, then starts the download
     */
    void downloadMetalink();
    void startMetalink() override;
    void untickAllFiles();

private:
    bool m_metalinkJustDownloaded;
    QUrl m_localMetalinkLocation;
    KGetMetalink::Metalink m_metalink;
};

#endif
