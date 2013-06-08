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

#include "metalinkhttp.h"
#include "metalinksettings.h"
#include "metalinkxml.h"

#include "core/kget.h"
#include "core/transfergroup.h"
#include "core/download.h"
#include "core/transferdatasource.h"
#include "core/filemodel.h"
#include "core/urlchecker.h"
#include "core/verifier.h"
#include "core/signature.h"
#ifdef HAVE_NEPOMUK
#include "core/nepomukhandler.h"
#include <Nepomuk/Variant>
#endif //HAVE_NEPOMUK

#include <KIconLoader>
#include <KIO/DeleteJob>
#include <KIO/NetAccess>
#include <KIO/RenameDialog>
#include <KLocale>
#include <KMessageBox>
#include <KDebug>
#include <KDialog>
#include <KStandardDirs>

#include <QtCore/QFile>
#include <QtXml/QDomElement>

MetalinkHttp::MetalinkHttp(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         KGetMetalink::MetalinkHttpParser *httpParser,
                         const QDomElement * e)
    : AbstractMetalink(parent,factory,scheduler,source, dest, e) ,
      m_signatureUrl(KUrl("")),
      m_httpparser(httpParser)

{

}

MetalinkHttp::~MetalinkHttp()
{

}

void MetalinkHttp::startMetalink()
{
    if (m_ready) {
        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            //specified number of files is downloaded simultanously
            if (m_currentFiles < MetalinkSettings::simultanousFiles()) {
                const int status = factory->status();

                //only start factories that should be downloaded
                if (factory->doDownload() &&
                    (status != Job::Finished) &&
                    (status != Job::FinishedKeepAlive) &&
                    (status != Job::Running)) {
                    ++m_currentFiles;
                    factory->start();
                }
            }
            else {
                break;
            }
        }
    }
}

QString MetalinkHttp::base64ToHex(const QString& b64)
{
     return QString(QByteArray::fromBase64(b64.toAscii()).toHex());
}

void MetalinkHttp::start()
{
    kDebug() << "metalinkhttp::start";

    if (!m_ready) {
        setLinks();
        setDigests();
        if (metalinkHttpInit()) {
            startMetalink();
        }
    }
    else {
        startMetalink();
    }
}

void MetalinkHttp::stop()
{
    kDebug(5001) << "metalink::Stop";
    if (m_ready && status() != Stopped) {
        m_currentFiles = 0;
        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            factory->stop();
        }
    }
}

void MetalinkHttp::setSignature(KUrl & dest, QByteArray & data, DataSourceFactory* dataFactory)
{
    Q_UNUSED(dest);
    dataFactory->signature()->setSignature(data,Signature::AsciiDetached);

}

void MetalinkHttp::slotSignatureVerified()
{
    if (status() == Job::Finished) {
        //see if some files are NotVerified
        QStringList brokenFiles;
        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            if (m_fileModel) {
                QModelIndex signatureVerified = m_fileModel->index(factory->dest(), FileItem::SignatureVerified);
                m_fileModel->setData(signatureVerified, factory->signature()->status());
            }
            if (factory->doDownload() && (factory->verifier()->status() == Verifier::NotVerified)) {
                brokenFiles.append(factory->dest().pathOrUrl());
            }
        }

        if (brokenFiles.count())
        {
            if (KMessageBox::warningYesNoCancelList(0,
                i18n("The download could not be verified, try to repair it?"),
                     brokenFiles) == KMessageBox::Yes) {
                    if (repair()) {
                    KGet::addTransfer(m_metalinkxmlUrl);
                    //TODO Use a Notification instead. Check kget.h for how to use it.

                }
            }
        }
    }
}

bool MetalinkHttp::metalinkHttpInit()
{
    kDebug() << "m_dest = " << m_dest;
    const KUrl tempDest=  KUrl(m_dest.directory());
    KUrl dest;
    dest = tempDest;
    dest.addPath(m_dest.fileName());
    kDebug() << "dest = " << dest;

    //sort the urls according to their priority (highest first)
    qStableSort(m_linkheaderList);

    DataSourceFactory *dataFactory = new DataSourceFactory(this,dest);
    dataFactory->setMaxMirrorsUsed(MetalinkSettings::mirrorsPerFile());

    connect(dataFactory, SIGNAL(capabilitiesChanged()), this, SLOT(slotUpdateCapabilities()));
    connect(dataFactory, SIGNAL(dataSourceFactoryChange(Transfer::ChangesFlags)), this, SLOT(slotDataSourceFactoryChange(Transfer::ChangesFlags)));
    connect(dataFactory->verifier(), SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
    connect(dataFactory->signature(), SIGNAL(verified(int)), this, SLOT(slotSignatureVerified()));
    connect(dataFactory, SIGNAL(log(QString,Transfer::LogLevel)), this, SLOT(setLog(QString,Transfer::LogLevel)));

    //add the Mirrors Sources

    for(int i = 0; i < m_linkheaderList.size(); ++i) {
        const KUrl url = m_linkheaderList[i].url;
        if (url.isValid()) {
            if (m_linkheaderList[i].m_pref) {
                kDebug() << "found etag in a mirror" ;
                KGetMetalink::MetalinkHttpParser* eTagCher = new KGetMetalink::MetalinkHttpParser(url) ;
                if (eTagCher->getEtag() != m_httpparser->getEtag()) { //There is an ETag mismatch
                    continue ;
                }
            }

            dataFactory->addMirror(url, MetalinkSettings::connectionsPerUrl());
        }
    }

    //no datasource has been created, so remove the datasource factory
    if (dataFactory->mirrors().isEmpty()) {
        kDebug() << "data source factory being deleted" ;
        delete dataFactory;
    }
    else
    {

        QHashIterator<QString, QString> itr(m_DigestList);
        while(itr.hasNext()) {
            itr.next();
            kDebug() << itr.key() << ":" << itr.value() ;
        }

        dataFactory->verifier()->addChecksums(m_DigestList);

        //Add OpenPGP signatures
        if (m_signatureUrl != (KUrl(""))) {
            Download *signat_download = new Download(m_signatureUrl, QString(KStandardDirs::locateLocal("appdata", "metalinks/") + m_source.fileName()));
            connect(signat_download, SIGNAL(finishedSuccessfully(KUrl,QByteArray)), SLOT(setSignature(KUrl,QByteArray)));
        }
        m_dataSourceFactory[dataFactory->dest()] = dataFactory;
    }

    if ((m_dataSourceFactory).size()) {
        m_dest = dest;
    }

    if (!m_dataSourceFactory.size()) {
        //TODO make this via log in the future + do not display the KMessageBox
        kWarning(5001) << "Download of" << m_source << "failed, no working URLs were found.";
        KMessageBox::error(0, i18n("Download failed, no working URLs were found."), i18n("Error"));
        setStatus(Job::Aborted);
        setTransferChange(Tc_Status, true);
        return false;
    }

    m_ready = !m_dataSourceFactory.isEmpty();
    slotUpdateCapabilities();

    return true;

}

void MetalinkHttp::setLinks()
{
    const QMultiMap<QString, QString>* headerInf = m_httpparser->getHeaderInfo();
    QList<QString> linkVals = headerInf->values("link");
    foreach ( QString link, linkVals) {
        KGetMetalink::httpLinkHeader linkheader;
        linkheader.headerBuilder(link);
        if (linkheader.m_reltype == "duplicate") {
            m_linkheaderList.append(linkheader);
        }
        if (linkheader.m_reltype == "application/pgp-signature") {
            m_signatureUrl = linkheader.url; //There will only be one signature
        }
        if (linkheader.m_reltype == "application/metalink4+xml") {
            m_metalinkxmlUrl = linkheader.url ; // There will only be one metalink xml (metainfo URL)
        }
    }
}

void MetalinkHttp::deinit(Transfer::DeleteOptions options)
{
    foreach (DataSourceFactory *factory, m_dataSourceFactory) {
        if (options & Transfer::DeleteFiles) {
            factory->deinit();
        }
    }
}

void MetalinkHttp::setDigests()
{
    QMultiMap<QString, QString>* digestInfo = m_httpparser->getHeaderInfo();
    QList<QString> digestList = digestInfo->values("digest");
    foreach( QString digest, digestList) {
        int eqDelimiter = digest.indexOf('=');
        QString digestType = digest.left(eqDelimiter).trimmed();
        QString digestValue = digest.mid(eqDelimiter + 1).trimmed();
        QString hexDigestValue = base64ToHex(digestValue);
        digestType = adaptDigestType(digestType);
        m_DigestList.insertMulti(digestType,hexDigestValue);
    }
}

QString MetalinkHttp::adaptDigestType(const QString & hashType)
{
    if (hashType == QString("SHA")) {
        return QString("sha");
    }
    else if (hashType == QString("MD5")) {
        return QString("md5");
    }
    else if (hashType == QString("SHA-256")) {
        return QString("sha256");
    }
    else {
        return hashType;
    }
}

#include "metalinkhttp.moc"
