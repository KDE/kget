/* This file is part of the KDE project

   Copyright (C) 2004 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2006 Manolo Valdes <nolis71cu@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "transfermultisegkio.h"
#ifdef HAVE_NEPOMUK
#include "core/nepomukhandler.h"
#endif //HAVE_NEPOMUK

#include "multisegkiosettings.h"
#include "core/kget.h"
#include "core/transferdatasource.h"
// #include "mirrors.h"
#include "core/filemodel.h"

#include <kiconloader.h>
#include <KIO/CopyJob>
#include <KIO/DeleteJob>
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

    if (!m_dataSourceFactory)
    {
        m_dataSourceFactory = new DataSourceFactory(m_dest, 0, 500 * 1024, this);
        connect(m_dataSourceFactory, SIGNAL(speed(ulong)), this, SLOT(slotSpeed(ulong)));
        connect(m_dataSourceFactory, SIGNAL(percent(ulong)), this, SLOT(slotPercent(ulong)));
        connect(m_dataSourceFactory, SIGNAL(processedSize(KIO::filesize_t)), this, SLOT(slotProcessedSize(KIO::filesize_t)));
        connect(m_dataSourceFactory, SIGNAL(statusChanged(Job::Status)), this, SLOT(slotStatus(Job::Status)));
        connect(m_dataSourceFactory, SIGNAL(totalSize(KIO::filesize_t)), this, SLOT(slotTotalSize(KIO::filesize_t)));
        connect(m_dataSourceFactory->verifier(), SIGNAL(verified(bool)), this, SLOT(slotVerified(bool)));

        m_dataSourceFactory->addMirror(m_source, MultiSegKioSettings::segments());
    }
}

void TransferMultiSegKio::start()
{
    kDebug(5001) << "Start TransferMultiSegKio";
    if (status() == Running)
    {
        return;
    }

    m_dataSourceFactory->start();

    if (MultiSegKioSettings::useSearchEngines() && !m_searchStarted)
    {
        m_searchStarted = true;
        QDomDocument doc;
        QDomElement element = doc.createElement("TransferDataSource");
        element.setAttribute("type", "search");
        doc.appendChild(element);

        TransferDataSource * mirrorSearch = KGet::createTransferDataSource(m_source, element);
        if (mirrorSearch)
        {
            connect(mirrorSearch, SIGNAL(data(const QList<KUrl>&)), this, SLOT(slotSearchUrls(const QList<KUrl>&)));
            mirrorSearch->start();
        }
    }

    if (MultiSegKioSettings::useSearchVerification() && !m_verificationSearch)
    {
        m_verificationSearch = true;
        QDomDocument doc;
        QDomElement element = doc.createElement("TransferDataSource");
        element.setAttribute("type", "checksumsearch");
        doc.appendChild(element);

        TransferDataSource *checksumSearch = KGet::createTransferDataSource(m_source, element);
        if (checksumSearch)
        {
            connect(checksumSearch, SIGNAL(data(QString, QString)), this, SLOT(slotChecksumFound(QString, QString)));
            checksumSearch->start();
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

bool TransferMultiSegKio::isResumable() const
{
    return true;
}

bool TransferMultiSegKio::repair(const KUrl &file)
{
    if (!file.isValid() || (m_dest == file))
    {
        if (m_dataSourceFactory && (m_dataSourceFactory->verifier()->status() == Verifier::NotVerified))
        {
            m_dataSourceFactory->repair();
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
    if (/*isResumable() && */newDestination.isValid() && (newDestination != dest()) && m_dataSourceFactory)
    {
        m_movingFile = true;
        stop();
        m_dataSourceFactory->setNewDestination(newDestination);

        m_dest = newDestination;

        if (m_fileModel)
        {
            m_fileModel->setDirectory(directory());
        }

#ifdef HAVE_NEPOMUK
        nepomukHandler()->setNewDestination(m_dest);
#endif //HAVE_NEPOMUK

        setTransferChange(Tc_FileName);

        return true;
    }
    return false;
}

void TransferMultiSegKio::postDeleteEvent()
{
    if (status() != Job::Finished)//if the transfer is not finished, we delete the *.part-file
    {
        m_dataSourceFactory->postDeleteEvent();
    }//TODO: Ask the user if he/she wants to delete the *.part-file? To discuss (boom1992)
#ifdef HAVE_NEPOMUK
    nepomukHandler()->postDeleteEvent();
#endif //HAVE_NEPOMU
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

void TransferMultiSegKio::slotStatus(Job::Status status)
{
    setStatus(status);
    setTransferChange(Tc_Status, true);

    if (m_fileModel)
    {
        QModelIndex statusIndex = m_fileModel->index(m_dest, FileItem::Status);
        m_fileModel->setData(statusIndex, status);
    }
}

void TransferMultiSegKio::slotVerified(bool isVerified)
{
    if (!isVerified && KMessageBox::warningYesNo(0,
                                  i18n("The download (%1) could not be verfied. Do you want to repair it?", m_dest.fileName()),
                                  i18n("Verification failed.")) == KMessageBox::Yes)
    {
        repair();
    }
}

void TransferMultiSegKio::slotPercent(ulong percent)
{
    m_percent = percent;
    setTransferChange(Tc_Percent, true);
}

void TransferMultiSegKio::slotProcessedSize(KIO::filesize_t processedSize)
{
    m_downloadedSize = processedSize;

    setTransferChange(Tc_DownloadedSize, true);
}

void TransferMultiSegKio::slotSpeed(unsigned long bytes_per_second)
{
    kDebug(5001) << "slotSpeed: " << bytes_per_second;

    m_downloadSpeed = bytes_per_second;
    setTransferChange(Tc_DownloadSpeed, true);
}

void TransferMultiSegKio::slotTotalSize(KIO::filesize_t size)
{
    m_totalSize = size;
    setTransferChange(Tc_TotalSize, true);

    if (m_fileModel)
    {
        QModelIndex sizeIndex = m_fileModel->index(m_dest, FileItem::Size);
        m_fileModel->setData(sizeIndex, static_cast<qlonglong>(m_dataSourceFactory->size()));
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
    m_dataSourceFactory->verifier()->model()->addChecksum(type, checksum);
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

FileModel *TransferMultiSegKio::fileModel()
{
    if (!m_fileModel)
    {
        m_fileModel = new FileModel(QList<KUrl>() << m_dest, m_dest.upUrl(), this);
        connect(m_fileModel, SIGNAL(rename(KUrl, KUrl)), this, SLOT(slotRename(KUrl,KUrl)));

        QModelIndex statusIndex = m_fileModel->index(m_dest, FileItem::Status);
        m_fileModel->setData(statusIndex, m_dataSourceFactory->status());
        QModelIndex sizeIndex = m_fileModel->index(m_dest, FileItem::Size);
        m_fileModel->setData(sizeIndex, static_cast<qlonglong>(m_dataSourceFactory->size()));
    }

    return m_fileModel;
}

void TransferMultiSegKio::slotRename(const KUrl &oldUrl, const KUrl &newUrl)
{
    Q_UNUSED(oldUrl);

    if (newUrl.isValid() && (newUrl != dest()) && m_dataSourceFactory)
    {
        m_movingFile = true;
        stop();
        m_dataSourceFactory->setNewDestination(newUrl);

        m_dest = newUrl;
#ifdef HAVE_NEPOMUK
        nepomukHandler()->setNewDestination(m_dest);
#endif //HAVE_NEPOMUK

        setTransferChange(Tc_FileName);
    }
}

#include "transfermultisegkio.moc"
