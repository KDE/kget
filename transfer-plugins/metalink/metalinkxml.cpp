/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 Manolo Valdes <nolis71cu@gmail.com>
   Copyright (C) 2009 Matthias Fuchs <mat69@gmx.net>
   Copyright (C) 2012 Aish Raj Dahal <dahalaishraj@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "metalinkxml.h"
#include "fileselectiondlg.h"
#include "metalinksettings.h"

#include "core/kget.h"
#include "core/transfergroup.h"
#include "core/download.h"
#include "core/transferdatasource.h"
#include "core/filemodel.h"
#include "core/urlchecker.h"
#include "core/verifier.h"
#include "core/signature.h"

#include "kget_debug.h"

#include <algorithm>

#include <kwidgetsaddons_version.h>
#include <KConfigGroup>
#include <KIO/DeleteJob>
#include <KIO/RenameDialog>
#include <KLocalizedString>
#include <KMessageBox>

#include <QDialog>
#include <QDir>
#include <QFile>
#include <QDomElement>
#include <QStandardPaths>

MetalinkXml::MetalinkXml(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const QUrl & source, const QUrl & dest,
                         const QDomElement * e)
    : AbstractMetalink(parent, factory, scheduler, source, dest, e)

{
}

MetalinkXml::~MetalinkXml()
{
}

void MetalinkXml::start()
{
    qCDebug(KGET_DEBUG) << "metalinkxml::start";

    if (!m_ready)
    {
        if (m_localMetalinkLocation.isValid() && metalinkInit()) {
            startMetalink();
        } else {
            downloadMetalink();
        }
    }
    else
    {
        startMetalink();
    }
}

void MetalinkXml::downloadMetalink()
{
    m_metalinkJustDownloaded = true;

    setStatus(Job::Running, i18n("Downloading Metalink File...."), "document-save");
    setTransferChange(Tc_Status, true);
    // make sure that the DataLocation directory exists (earlier this used to be handled by KStandardDirs)
    if (!QFileInfo::exists(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))) {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }
    const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/metalinks/") + m_source.fileName();
    auto *download = new Download(m_source, QUrl::fromLocalFile(path));
    connect(download, &Download::finishedSuccessfully, this, &MetalinkXml::metalinkInit);
}

bool MetalinkXml::metalinkInit(const QUrl &src, const QByteArray &data)
{
    qCDebug(KGET_DEBUG) << "MetalinkXml::metalinkInit";

    if (!src.isEmpty()) {
        m_localMetalinkLocation = src;
    }

    //use the downloaded metalink-file data directly if possible
    if (!data.isEmpty()) {
        KGetMetalink::HandleMetalink::load(data, &m_metalink);
    }

    //try to parse the locally stored metalink-file
    if (!m_metalink.isValid() && m_localMetalinkLocation.isValid()) {
        KGetMetalink::HandleMetalink::load(m_localMetalinkLocation, &m_metalink);
    }

    if (!m_metalink.isValid()) {
        qCCritical(KGET_DEBUG) << "Unknown error when trying to load the .metalink-file. Metalink is not valid.";
        setStatus(Job::Aborted);
        setTransferChange(Tc_Status, true);
        return false;
    }

    //offers a dialog to download the newest version of a dynamic metalink
     if ((m_source.isLocalFile() || !m_metalinkJustDownloaded) &&
         m_metalink.dynamic && (UrlChecker::checkSource(m_metalink.origin) == UrlChecker::NoError)) {
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        if (KMessageBox::questionTwoActions(nullptr,
#else
        if (KMessageBox::questionYesNo(nullptr,
#endif
                                       i18n("A newer version of this Metalink might exist, do you want to download it?"),
                                       i18n("Redownload Metalink"),
                                       KGuiItem(i18nc("@action:button", "Download Again"), QStringLiteral("view-refresh")),
                                       KGuiItem(i18nc("@action:button", "Ignore"), QStringLiteral("dialog-cancel")))
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
            == KMessageBox::PrimaryAction) {
#else
            == KMessageBox::Yes) {
#endif
            m_localMetalinkLocation.clear();
            m_source = m_metalink.origin;
            downloadMetalink();
            return false;
        }
    }

    QList<KGetMetalink::File>::const_iterator it;
    QList<KGetMetalink::File>::const_iterator itEnd = m_metalink.files.files.constEnd();
    m_totalSize = 0;
    KIO::fileoffset_t segSize = 500 * 1024;//TODO use config here!
    const QUrl tempDest = QUrl(m_dest.adjusted(QUrl::RemoveFilename));
    QUrl dest;
    for (it = m_metalink.files.files.constBegin(); it != itEnd ; ++it)
    {
        dest = tempDest;
        dest.setPath(tempDest.path() + (*it).name);

        QList<KGetMetalink::Url> urlList = (*it).resources.urls;
        //sort the urls according to their priority (highest first)
        std::sort(urlList.begin(), urlList.end(), [](const KGetMetalink::Url &a, const KGetMetalink::Url &b) { return b < a; });

        KIO::filesize_t fileSize = (*it).size;
        m_totalSize += fileSize;

        //create a DataSourceFactory for each separate file
        auto *dataFactory = new DataSourceFactory(this, dest, fileSize, segSize);
        dataFactory->setMaxMirrorsUsed(MetalinkSettings::mirrorsPerFile());

//TODO compare available file size (<size>) with the sizes of the server while downloading?

        connect(dataFactory, &DataSourceFactory::capabilitiesChanged, this, &MetalinkXml::slotUpdateCapabilities);
        connect(dataFactory, &DataSourceFactory::dataSourceFactoryChange, this, &MetalinkXml::slotDataSourceFactoryChange);
        connect(dataFactory->verifier(), &Verifier::verified, this, &MetalinkXml::slotVerified);
        connect(dataFactory->signature(), &Signature::verified, this, &MetalinkXml::slotSignatureVerified);
        connect(dataFactory, &DataSourceFactory::log, this, &Transfer::setLog);

        //add the DataSources
        for (int i = 0; i < urlList.size(); ++i)
        {
            const QUrl url = urlList[i].url;
            if (url.isValid())
            {
                dataFactory->addMirror(url, MetalinkSettings::connectionsPerUrl());
            }
        }
        //no datasource has been created, so remove the datasource factory
        if (dataFactory->mirrors().isEmpty())
        {
            delete dataFactory;
        }
        else
        {
            dataFactory->verifier()->addChecksums((*it).verification.hashes);

            foreach (const KGetMetalink::Pieces &pieces, (*it).verification.pieces) {
                dataFactory->verifier()->addPartialChecksums(pieces.type, pieces.length, pieces.hashes);
            }

            const QHash <QString, QString> signatures = (*it).verification.signatures;
            QHash<QString, QString>::const_iterator it;
            QHash<QString, QString>::const_iterator itEnd = signatures.constEnd();
            for (it = signatures.constBegin(); it != itEnd; ++it) {
                if (it.key().toLower() == "pgp") {
                    dataFactory->signature()->setAsciiDetachedSignature(*it);
                }
            }

            m_dataSourceFactory[dataFactory->dest()] = dataFactory;
        }
    }

    if ((m_metalink.files.files.size() == 1) &&   m_dataSourceFactory.size())
    {
        m_dest = dest;
    }

    if (!m_dataSourceFactory.size()) {
        //TODO make this via log in the future + do not display the KMessageBox
        qCWarning(KGET_DEBUG) << "Download of" << m_source << "failed, no working URLs were found.";
        KMessageBox::error(nullptr, i18n("Download failed, no working URLs were found."), i18n("Error"));
        setStatus(Job::Aborted);
        setTransferChange(Tc_Status, true);
        return false;
    }

    m_ready = !m_dataSourceFactory.isEmpty();
    slotUpdateCapabilities();

    //the metalink-file has just been downloaded, so ask the user to choose the files that
    // should be downloaded
    /* TODO this portion seems not to be working. Need to ask boom1992 */
    if (m_metalinkJustDownloaded) {
        QDialog *dialog = new FileSelectionDlg(fileModel());
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::finished, this, &MetalinkXml::fileDlgFinished);

        dialog->show();
    }

    return true;
}

void MetalinkXml::startMetalink()
{
    if (m_ready)
    {
        foreach (DataSourceFactory *factory, m_dataSourceFactory)
        {
            //specified number of files is downloaded simultaneously
            if (m_currentFiles < MetalinkSettings::simultaneousFiles())
            {
                const int status = factory->status();
                //only start factories that should be downloaded
                if (factory->doDownload() &&
                    (status != Job::Finished) &&
                    (status != Job::FinishedKeepAlive) &&
                    (status != Job::Running))
                {
                    ++m_currentFiles;
                    factory->start();
                }
            }
            else
            {
                break;
            }
        }
    }
}

void MetalinkXml::deinit(Transfer::DeleteOptions options)
{
    foreach (DataSourceFactory *factory, m_dataSourceFactory) {
        if (options & Transfer::DeleteFiles) {
            factory->deinit();
        }
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)

    //FIXME does that mean, that the metalink file is always removed, even if
    //downloaded by the user?
    if ((options & Transfer::DeleteTemporaryFiles) && m_localMetalinkLocation.isLocalFile())
    {
        KIO::Job *del = KIO::del(m_localMetalinkLocation, KIO::HideProgressInfo);
        if (!del->exec()) {
            qCDebug(KGET_DEBUG) << "Could not delete " << m_localMetalinkLocation.path();
        }
    }

}

void MetalinkXml::load(const QDomElement *element)
{
    Transfer::load(element);

    if (!element)
    {
        return;
    }

    const QDomElement e = *element;
    m_localMetalinkLocation = QUrl(e.attribute("LocalMetalinkLocation"));
    QDomNodeList factories = e.firstChildElement("factories").elementsByTagName("factory");

    //no stored information found, stop right here
    if (!factories.count())
    {
        return;
    }

    while (factories.count())
    {
        QDomDocument doc;
        QDomElement factory = doc.createElement("factories");
        factory.appendChild(factories.item(0).toElement());
        doc.appendChild(factory);

        auto *file = new DataSourceFactory(this);
        file->load(&factory);
        connect(file, &DataSourceFactory::capabilitiesChanged, this, &MetalinkXml::slotUpdateCapabilities);
        connect(file, &DataSourceFactory::dataSourceFactoryChange, this, &MetalinkXml::slotDataSourceFactoryChange);
        m_dataSourceFactory[file->dest()] = file;
        connect(file->verifier(), &Verifier::verified, this, &MetalinkXml::slotVerified);
        connect(file->signature(), &Signature::verified, this, &MetalinkXml::slotSignatureVerified);
        connect(file, &DataSourceFactory::log, this, &Transfer::setLog);

        //start the DataSourceFactories that were Started when KGet was closed
        if (file->status() == Job::Running) {
            if (m_currentFiles < MetalinkSettings::simultaneousFiles()) {
                ++m_currentFiles;
                file->start();
            } else {
                //enough simultaneous files already, so increase the number and set file to stop --> that will decrease the number again
                file->stop();
            }
        }
    }
    m_ready = !m_dataSourceFactory.isEmpty();
    slotUpdateCapabilities();
}

void MetalinkXml::save(const QDomElement &element)
{
    Transfer::save(element);

    QDomElement e = element;
    e.setAttribute("LocalMetalinkLocation", m_localMetalinkLocation.url());

    foreach (DataSourceFactory *factory, m_dataSourceFactory)
    {
        factory->save(e);
    }
}


