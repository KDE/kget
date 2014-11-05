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
#include <qdebug.h>

#ifdef HAVE_NEPOMUK
    #include "core/nepomukhandler.h"
    #include <Nepomuk2/Variant>
#endif //HAVE_NEPOMUK

#include <KIconLoader>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <KIO/RenameDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KDebug>
#include <KDialog>
#include <KStandardDirs>

#include <QFile>
#include <QDomElement>

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

    setStatus(Job::Running, i18n("Downloading Metalink File...."), SmallIcon("document-save"));
    setTransferChange(Tc_Status, true);
    Download *download = new Download(m_source, QString(KStandardDirs::locateLocal("appdata", "metalinks/") + m_source.fileName()));
    connect(download, SIGNAL(finishedSuccessfully(QUrl,QByteArray)), SLOT(metalinkInit(QUrl,QByteArray)));
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
        KGetMetalink::HandleMetalink::load(m_localMetalinkLocation.toLocalFile(), &m_metalink);
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
        if (KMessageBox::questionYesNo(0, i18n("A newer version of this Metalink might exist, do you want to download it?"),
                                       i18n("Redownload Metalink")) == KMessageBox::Yes) {
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
        dest.setPath(tempDest.adjusted(QUrl::RemoveFilename | QUrl::RemoveScheme).toString() + "/" + (*it).name);

        QList<KGetMetalink::Url> urlList = (*it).resources.urls;
        //sort the urls according to their priority (highest first)
        qSort(urlList.begin(), urlList.end(), qGreater<KGetMetalink::Url>());

        KIO::filesize_t fileSize = (*it).size;
        m_totalSize += fileSize;

        //create a DataSourceFactory for each separate file
        DataSourceFactory *dataFactory = new DataSourceFactory(this, dest, fileSize, segSize);
        dataFactory->setMaxMirrorsUsed(MetalinkSettings::mirrorsPerFile());

#ifdef HAVE_NEPOMUK
        nepomukHandler()->setProperties((*it).properties(), QList<QUrl>() << dest);
#endif //HAVE_NEPOMUK

//TODO compare available file size (<size>) with the sizes of the server while downloading?

        connect(dataFactory, SIGNAL(capabilitiesChanged()), this, SLOT(slotUpdateCapabilities()));
        connect(dataFactory, SIGNAL(dataSourceFactoryChange(Transfer::ChangesFlags)), this, SLOT(slotDataSourceFactoryChange(Transfer::ChangesFlags)));
        connect(dataFactory->verifier(), SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
        connect(dataFactory->signature(), SIGNAL(verified(int)), this, SLOT(slotSignatureVerified()));
        connect(dataFactory, SIGNAL(log(QString,Transfer::LogLevel)), this, SLOT(setLog(QString,Transfer::LogLevel)));

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
                    dataFactory->signature()->setAsciiDetatchedSignature(*it);
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
        KMessageBox::error(0, i18n("Download failed, no working URLs were found."), i18n("Error"));
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
        connect(dialog, SIGNAL(finished(int)), this, SLOT(fileDlgFinished(int)));

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
            //specified number of files is downloaded simultanously
            if (m_currentFiles < MetalinkSettings::simultanousFiles())
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
        KIO::NetAccess::synchronousRun(del, 0);
    }

#ifdef HAVE_NEPOMUK
    nepomukHandler()->deinit();
#endif //HAVE_NEPOMUK
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

        DataSourceFactory *file = new DataSourceFactory(this);
        file->load(&factory);
        connect(file, SIGNAL(capabilitiesChanged()), this, SLOT(slotUpdateCapabilities()));
        connect(file, SIGNAL(dataSourceFactoryChange(Transfer::ChangesFlags)), this, SLOT(slotDataSourceFactoryChange(Transfer::ChangesFlags)));
        m_dataSourceFactory[file->dest()] = file;
        connect(file->verifier(), SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
        connect(file->signature(), SIGNAL(verified(int)), this, SLOT(slotSignatureVerified()));
        connect(file, SIGNAL(log(QString,Transfer::LogLevel)), this, SLOT(setLog(QString,Transfer::LogLevel)));

        //start the DataSourceFactories that were Started when KGet was closed
        if (file->status() == Job::Running) {
            if (m_currentFiles < MetalinkSettings::simultanousFiles()) {
                ++m_currentFiles;
                file->start();
            } else {
                //enough simultanous files already, so increase the number and set file to stop --> that will decrease the number again
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


