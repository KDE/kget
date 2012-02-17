/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfermultisegkio.h"

#include "multisegkiosettings.h"
#include "core/kget.h"
#include "core/transferdatasource.h"
// #include "mirrors.h"
#include "core/filemodel.h"
#include "core/verifier.h"
#include "core/signature.h"

#include <kiconloader.h>
#include <KIO/CopyJob>
#include <KIO/NetAccess>
#include <klocale.h>
#include <KMessageBox>
#include <kdebug.h>

#include <QDomElement>
#include <QFile>

TransferMultiSegKio::TransferMultiSegKio(TransferGroup *parent, TransferFactory *factory,
                         Scheduler *scheduler, const KUrl &source, const KUrl &dest,
                         const QDomElement *e)
  : Transfer(parent, factory, scheduler, source, dest, e),
    m_movingFile(false),
    m_searchStarted(false),
    m_verificationSearch(false),
    m_dataSourceFactory(0),
    m_fileModel(0)
{
}

void TransferMultiSegKio::init()
{
    Transfer::init();

    if (!m_dataSourceFactory) {
         m_dataSourceFactory = new DataSourceFactory(this, m_dest);
        connect(m_dataSourceFactory, SIGNAL(capabilitiesChanged()), this, SLOT(slotUpdateCapabilities()));
        connect(m_dataSourceFactory, SIGNAL(dataSourceFactoryChange(Transfer::ChangesFlags)), this, SLOT(slotDataSourceFactoryChange(Transfer::ChangesFlags)));
        connect(m_dataSourceFactory->verifier(), SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));
        connect(m_dataSourceFactory, SIGNAL(log(QString,Transfer::LogLevel)), this, SLOT(setLog(QString,Transfer::LogLevel)));

        m_dataSourceFactory->addMirror(m_source, MultiSegKioSettings::segments());

        slotUpdateCapabilities();
    }
}

void TransferMultiSegKio::deinit(Transfer::DeleteOptions options)
{
    if (options & Transfer::DeleteFiles)//if the transfer is not finished, we delete the *.part-file
    {
        m_dataSourceFactory->deinit();
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
}

void TransferMultiSegKio::start()
{
    kDebug(5001) << "Start TransferMultiSegKio";
    if (status() == Running) {
        return;
    }

    m_dataSourceFactory->start();

    if (MultiSegKioSettings::useSearchEngines() && !m_searchStarted) {
        m_searchStarted = true;
        QDomDocument doc;
        QDomElement element = doc.createElement("TransferDataSource");
        element.setAttribute("type", "search");
        doc.appendChild(element);

        TransferDataSource *mirrorSearch = KGet::createTransferDataSource(m_source, element, this);
        if (mirrorSearch) {
            connect(mirrorSearch, SIGNAL(data(QList<KUrl>)), this, SLOT(slotSearchUrls(QList<KUrl>)));
            mirrorSearch->start();
        }
    }
}

void TransferMultiSegKio::stop()
{
    kDebug(5001);

    if(status() == Stopped)
        return;

    if (m_dataSourceFactory)
    {
        m_dataSourceFactory->stop();
    }
}

bool TransferMultiSegKio::repair(const KUrl &file)
{
    if (!file.isValid() || (m_dest == file))
    {
        if (m_dataSourceFactory && (m_dataSourceFactory->verifier()->status() == Verifier::NotVerified))
        {
            m_dataSourceFactory->repair();
            return true;
        }
    }

    return false;
}

bool TransferMultiSegKio::setDirectory(const KUrl& newDirectory)
{
    KUrl newDest = newDirectory;
    newDest.addPath(m_dest.fileName());
    return setNewDestination(newDest);
}

bool TransferMultiSegKio::setNewDestination(const KUrl &newDestination)
{
    kDebug(5001) << "New destination: " << newDestination;
    if (newDestination.isValid() && (newDestination != dest()) && m_dataSourceFactory)
    {
        m_movingFile = true;
        stop();
        m_dataSourceFactory->setNewDestination(newDestination);

        m_dest = newDestination;

        if (m_fileModel)
        {
            m_fileModel->setDirectory(directory());
        }

        setTransferChange(Tc_FileName);
        return true;
    }
    return false;
}

void TransferMultiSegKio::load(const QDomElement *element)
{
    kDebug(5001);

    Transfer::load(element);
    m_dataSourceFactory->load(element);
}

void TransferMultiSegKio::save(const QDomElement &element)
{
    kDebug(5001);
    Transfer::save(element);
    m_dataSourceFactory->save(element);
}

void TransferMultiSegKio::slotDataSourceFactoryChange(Transfer::ChangesFlags change)
{
    if (change & Tc_Status) {
        setStatus(m_dataSourceFactory->status());

        if (m_fileModel) {
            QModelIndex statusIndex = m_fileModel->index(m_dest, FileItem::Status);
            m_fileModel->setData(statusIndex, status());
        }
    }
    if (change & Tc_TotalSize) {
        m_totalSize = m_dataSourceFactory->size();
        if (m_fileModel) {
            QModelIndex sizeIndex = m_fileModel->index(m_dest, FileItem::Size);
            m_fileModel->setData(sizeIndex, static_cast<qlonglong>(m_totalSize));
        }
    }
    if (change & Tc_DownloadedSize) {
        KIO::filesize_t processedSize = m_dataSourceFactory->downloadedSize();
        //only start the verification search _after_ data has come in, that way only connections
        //are requested if there is already a successful one
        if ((processedSize != m_downloadedSize) && !m_verificationSearch && MultiSegKioSettings::useSearchVerification()) {
            m_verificationSearch = true;
            QDomDocument doc;
            QDomElement element = doc.createElement("TransferDataSource");
            element.setAttribute("type", "checksumsearch");
            doc.appendChild(element);

            TransferDataSource *checksumSearch = KGet::createTransferDataSource(m_source, element, this);
            if (checksumSearch) {
                connect(checksumSearch, SIGNAL(data(QString,QString)), this, SLOT(slotChecksumFound(QString,QString)));
                checksumSearch->start();
            }
        }
        m_downloadedSize = m_dataSourceFactory->downloadedSize();
    }
    if (change & Tc_Percent) {
        m_percent = m_dataSourceFactory->percent();
    }
    if (change & Tc_DownloadSpeed) {
        kDebug(5001) << "speed:" << m_downloadSpeed;
        m_downloadSpeed = m_dataSourceFactory->currentSpeed();
    }

    setTransferChange(change, true);
}

void TransferMultiSegKio::slotVerified(bool isVerified)
{
    if (m_fileModel) {
        QModelIndex checksumVerified = m_fileModel->index(m_dest, FileItem::ChecksumVerified);
        m_fileModel->setData(checksumVerified, verifier()->status());
    }

    if (!isVerified) {
        QString text = i18n("The download (%1) could not be verified. Do you want to repair it?", m_dest.fileName());

        if (!verifier()->partialChunkLength()) {
            text = i18n("The download (%1) could not be verified. Do you want to redownload it?", m_dest.fileName());
        }
        if (KMessageBox::warningYesNo(0,
                                      text,
                                      i18n("Verification failed.")) == KMessageBox::Yes) {
            repair();
        }
    }
}

void TransferMultiSegKio::slotSearchUrls(const QList<KUrl> &urls)
{
    kDebug(5001) << "Found " << urls.size() << " urls.";

    foreach (const KUrl &url, urls)
    {
        m_dataSourceFactory->addMirror(url, MultiSegKioSettings::segments());
    }
}

void TransferMultiSegKio::slotChecksumFound(QString type, QString checksum)
{
    m_dataSourceFactory->verifier()->addChecksum(type, checksum);
}

QHash<KUrl, QPair<bool, int> > TransferMultiSegKio::availableMirrors(const KUrl &file) const
{
    Q_UNUSED(file)

    return m_dataSourceFactory->mirrors();
}


void TransferMultiSegKio::setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors)
{
    Q_UNUSED(file)

    m_dataSourceFactory->setMirrors(mirrors);
}

Verifier *TransferMultiSegKio::verifier(const KUrl &file)
{
    Q_UNUSED(file)

    return m_dataSourceFactory->verifier();
}

Signature *TransferMultiSegKio::signature(const KUrl &file)
{
    Q_UNUSED(file)

    return m_dataSourceFactory->signature();
}

FileModel *TransferMultiSegKio::fileModel()
{
    if (!m_fileModel)
    {
        m_fileModel = new FileModel(QList<KUrl>() << m_dest, m_dest.upUrl(), this);
        connect(m_fileModel, SIGNAL(rename(KUrl,KUrl)), this, SLOT(slotRename(KUrl,KUrl)));

        QModelIndex statusIndex = m_fileModel->index(m_dest, FileItem::Status);
        m_fileModel->setData(statusIndex, m_dataSourceFactory->status());
        QModelIndex sizeIndex = m_fileModel->index(m_dest, FileItem::Size);
        m_fileModel->setData(sizeIndex, static_cast<qlonglong>(m_dataSourceFactory->size()));
        QModelIndex checksumVerified = m_fileModel->index(m_dest, FileItem::ChecksumVerified);
        m_fileModel->setData(checksumVerified, verifier()->status());
        QModelIndex signatureVerified = m_fileModel->index(m_dest, FileItem::SignatureVerified);
        m_fileModel->setData(signatureVerified, signature()->status());
    }

    return m_fileModel;
}

void TransferMultiSegKio::slotRename(const KUrl &oldUrl, const KUrl &newUrl)
{
    Q_UNUSED(oldUrl)

    if (newUrl.isValid() && (newUrl != dest()) && m_dataSourceFactory)
    {
        m_movingFile = true;
        stop();
        m_dataSourceFactory->setNewDestination(newUrl);

        m_dest = newUrl;

        setTransferChange(Tc_FileName);
    }
}

void TransferMultiSegKio::slotUpdateCapabilities()
{
    setCapabilities(m_dataSourceFactory->capabilities());
}

#include "transfermultisegkio.moc"
