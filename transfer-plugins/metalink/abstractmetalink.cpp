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

#include "abstractmetalink.h"

#include "core/kget.h"
#include "core/transfergroup.h"
#include "core/download.h"
#include "core/transferdatasource.h"
#include "core/filemodel.h"
#include "core/urlchecker.h"
#include "core/verifier.h"
#include "core/signature.h"

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

AbstractMetalink::AbstractMetalink(TransferGroup * parent, TransferFactory * factory,
                         Scheduler * scheduler, const KUrl & source, const KUrl & dest,
                         const QDomElement * e)
    : Transfer(parent, factory, scheduler, source, dest, e),
      m_fileModel(0),
      m_currentFiles(0),
      m_ready(false),
      m_speedCount(0),
      m_tempAverageSpeed(0),
      m_averageSpeed(0)
{
}

AbstractMetalink::~AbstractMetalink()
{
}

void AbstractMetalink::slotDataSourceFactoryChange(Transfer::ChangesFlags change)
{
    if ((change & Tc_Status) | (change & Tc_TotalSize)) {
        DataSourceFactory *factory = qobject_cast<DataSourceFactory*>(sender());
        if (change & Tc_Status) {
            bool changeStatus;
            updateStatus(factory, &changeStatus);
            if (!changeStatus) {
                change &= ~Tc_Status;
            }
        }
        if (change & Tc_TotalSize) {
            recalculateTotalSize(factory);
        }
    }
    if (change & Tc_DownloadedSize) {
        recalculateProcessedSize();
        change |= Tc_Percent;
    }
    if (change & Tc_DownloadSpeed) {
        recalculateSpeed();
    }

    setTransferChange(change, true);
}

void AbstractMetalink::recalculateTotalSize(DataSourceFactory *sender)
{
    m_totalSize = 0;
    foreach (DataSourceFactory *factory, m_dataSourceFactory) {
        if (factory->doDownload()) {
            m_totalSize += factory->size();
        }
    }

    if (m_fileModel) {
        if (sender) {
            QModelIndex sizeIndex = m_fileModel->index(sender->dest(), FileItem::Size);
            m_fileModel->setData(sizeIndex, static_cast<qlonglong>(sender->size()));
        }
    }
}

void AbstractMetalink::recalculateProcessedSize()
{
    m_downloadedSize = 0;
    foreach (DataSourceFactory *factory, m_dataSourceFactory) {
        if (factory->doDownload()) {
            m_downloadedSize += factory->downloadedSize();
        }
    }

    if (m_totalSize)
    {
        m_percent = (m_downloadedSize * 100) / m_totalSize;
    }
    else
    {
        m_percent = 0;
    }
}

void AbstractMetalink::recalculateSpeed()
{
    m_downloadSpeed = 0;
    foreach (DataSourceFactory *factory, m_dataSourceFactory) {
        if (factory->doDownload()) {
            m_downloadSpeed += factory->currentSpeed();
        }
    }

    //calculate the average of the last three speeds
    m_tempAverageSpeed += m_downloadSpeed;
    ++m_speedCount;
    if (m_speedCount == 3) {
        m_averageSpeed = m_tempAverageSpeed / 3;
        m_speedCount = 0;
        m_tempAverageSpeed = 0;
    }
}

int AbstractMetalink::remainingTime() const
{
    if (!m_averageSpeed) {
        m_averageSpeed = m_downloadSpeed;
    }
    return KIO::calculateRemainingSeconds(m_totalSize, m_downloadedSize, m_averageSpeed);
}

void AbstractMetalink::updateStatus(DataSourceFactory *sender, bool *changeStatus)
{
    Job::Status status = (sender ? sender->status() : Job::Stopped);
    *changeStatus = true;
    switch (status)
    {
        case Job::Aborted:
        case Job::Stopped: {
            m_currentFiles = 0;
            foreach (DataSourceFactory *factory, m_dataSourceFactory) {
                //one factory is still running, do not change the status
                if (factory->doDownload() && (factory->status() == Job::Running)) {
                    *changeStatus = false;
                    ++m_currentFiles;
                }
            }

            if (*changeStatus) {
                setStatus(status);
            }
            break;
        }
        case Job::Finished:
            //one file that has been downloaded now is finished//FIXME ignore downloads that were finished in the previous download!!!!
            if (m_currentFiles) {
                --m_currentFiles;
                startMetalink();
            }
            foreach (DataSourceFactory *factory, m_dataSourceFactory) {
                //one factory is not finished, do not change the status
                if (factory->doDownload() && (factory->status() != Job::Finished)) {
                    *changeStatus = false;
                    break;
                }
            }

            if (*changeStatus) {
                setStatus(Job::Finished);
            }
            break;

        default:
            setStatus(status);
            break;
    }

    if (m_fileModel) {
        if (sender) {
            QModelIndex statusIndex = m_fileModel->index(sender->dest(), FileItem::Status);
            m_fileModel->setData(statusIndex, status);
        }
    }
}

void AbstractMetalink::slotVerified(bool isVerified)
{
    Q_UNUSED(isVerified)

    if (status() == Job::Finished)
    {
        //see if some files are NotVerified
        QStringList brokenFiles;
        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            if (m_fileModel) {
                QModelIndex checksumVerified = m_fileModel->index(factory->dest(), FileItem::ChecksumVerified);
                m_fileModel->setData(checksumVerified, factory->verifier()->status());
            }
            if (factory->doDownload() && (factory->verifier()->status() == Verifier::NotVerified)) {
                brokenFiles.append(factory->dest().pathOrUrl());
            }
        }

        if (brokenFiles.count())
        {
            if (KMessageBox::warningYesNoCancelList(0,
                i18n("The download could not be verified, do you want to repair (if repairing does not work the download would be restarted) it?"),
                     brokenFiles) == KMessageBox::Yes) {
                if (repair()) {
                    return;
                }
            }
        }
    }
}

void AbstractMetalink::slotSignatureVerified()
{
    if (status() == Job::Finished)
    {
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
/*
        if (brokenFiles.count())//TODO
        {
            if (KMessageBox::warningYesNoCancelList(0,
                i18n("The download could not be verified, try to repair it?"),
                     brokenFiles) == KMessageBox::Yes)
            {
                if (repair())
                {
                    return;
                }
            }
        }*/
    }
}

bool AbstractMetalink::repair(const KUrl &file)
{
    if (file.isValid()) {
        if (m_dataSourceFactory.contains(file)) {
            DataSourceFactory *broken = m_dataSourceFactory[file];
            if (broken->verifier()->status() == Verifier::NotVerified) {
                broken->repair();
                return true;
            }
        }
    }
    else {
        QList<DataSourceFactory*> broken;
        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            if (factory->doDownload() && (factory->verifier()->status() == Verifier::NotVerified)) {
                broken.append(factory);
            }
        }
        if (broken.count()) {
            foreach (DataSourceFactory *factory, broken) {
                factory->repair();
            }
            return true;
        }
    }

    return false;
}


Verifier *AbstractMetalink::verifier(const KUrl &file)
{
    if (!m_dataSourceFactory.contains(file)) {
        return 0;
    }

    return m_dataSourceFactory[file]->verifier();
}

Signature *AbstractMetalink::signature(const KUrl &file)
{
    if (!m_dataSourceFactory.contains(file)) {
        return 0;
    }

    return m_dataSourceFactory[file]->signature();
}

QList<KUrl> AbstractMetalink::files() const
{
    return m_dataSourceFactory.keys();
}

FileModel *AbstractMetalink::fileModel()
{
    if (!m_fileModel) {
        m_fileModel = new FileModel(files(), directory(), this);
        connect(m_fileModel, SIGNAL(rename(KUrl,KUrl)), this, SLOT(slotRename(KUrl,KUrl)));
        connect(m_fileModel, SIGNAL(checkStateChanged()), this, SLOT(filesSelected()));

        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            const KUrl dest = factory->dest();
            QModelIndex size = m_fileModel->index(dest, FileItem::Size);
            m_fileModel->setData(size, static_cast<qlonglong>(factory->size()));
            QModelIndex status = m_fileModel->index(dest, FileItem::Status);
            m_fileModel->setData(status, factory->status());
            QModelIndex checksumVerified = m_fileModel->index(dest, FileItem::ChecksumVerified);
            m_fileModel->setData(checksumVerified, factory->verifier()->status());
            QModelIndex signatureVerified = m_fileModel->index(dest, FileItem::SignatureVerified);
            m_fileModel->setData(signatureVerified, factory->signature()->status());
            if (!factory->doDownload())
            {
                QModelIndex index = m_fileModel->index(factory->dest(), FileItem::File);
                m_fileModel->setData(index, Qt::Unchecked, Qt::CheckStateRole);
            }
        }
    }

    return m_fileModel;
}

void AbstractMetalink::slotRename(const KUrl &oldUrl, const KUrl &newUrl)
{
    if (!m_dataSourceFactory.contains(oldUrl)) {
        return;
    }

    m_dataSourceFactory[newUrl] = m_dataSourceFactory[oldUrl];
    m_dataSourceFactory.remove(oldUrl);
    m_dataSourceFactory[newUrl]->setNewDestination(newUrl);

    setTransferChange(Tc_FileName);
}

bool AbstractMetalink::setDirectory(const KUrl &new_directory)
{
    if (new_directory == directory()) {
        return false;
    }

    if (m_fileModel) {
        m_fileModel->setDirectory(new_directory);
    }

    const QString oldDirectory = directory().pathOrUrl(KUrl::AddTrailingSlash);
    const QString newDirectory = new_directory.pathOrUrl(KUrl::AddTrailingSlash);
    const QString fileName = m_dest.fileName();
    m_dest = new_directory;
    m_dest.addPath(fileName);

    QHash<KUrl, DataSourceFactory*> newStorage;
    foreach (DataSourceFactory *factory, m_dataSourceFactory) {
        const KUrl oldUrl = factory->dest();
        const KUrl newUrl = KUrl(oldUrl.pathOrUrl().replace(oldDirectory, newDirectory));
        factory->setNewDestination(newUrl);
        newStorage[newUrl] = factory;
    }
    m_dataSourceFactory = newStorage;

    setTransferChange(Tc_FileName);
    return true;
}

QHash<KUrl, QPair<bool, int> > AbstractMetalink::availableMirrors(const KUrl &file) const
{
    QHash<KUrl, QPair<bool, int> > urls;

    if (m_dataSourceFactory.contains(file)) {
        urls = m_dataSourceFactory[file]->mirrors();
    }

    return urls;
}


void AbstractMetalink::setAvailableMirrors(const KUrl &file, const QHash<KUrl, QPair<bool, int> > &mirrors)
{
    if (!m_dataSourceFactory.contains(file)) {
        return;
    }

    m_dataSourceFactory[file]->setMirrors(mirrors);
}

void AbstractMetalink::slotUpdateCapabilities()
{
    Capabilities oldCap = capabilities();
    Capabilities newCap = 0;
    foreach (DataSourceFactory *file, m_dataSourceFactory) {
        if (file->doDownload()) {//FIXME when a download did not start yet it should be moveable!!//FIXME why not working, when only two connections?
            if (newCap) {
                newCap &= file->capabilities();
            } else {
                newCap = file->capabilities();
            }
        }
    }

    if (newCap != oldCap) {
        setCapabilities(newCap);
    }
}

void AbstractMetalink::untickAllFiles()
{
    for (int row = 0; row < fileModel()->rowCount(); ++row) {
        QModelIndex index = fileModel()->index(row, FileItem::File);
        if (index.isValid()) {
            fileModel()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
    }
}

void AbstractMetalink::fileDlgFinished(int result)
{
    //the dialog was not accepted untick every file, this ensures that the user does not
    //press start by accident without first selecting the desired files
    if (result != QDialog::Accepted) {
        untickAllFiles();
    }

    filesSelected();

    //no files selected to download or dialog rejected, stop the download
    if (!m_numFilesSelected || (result != QDialog::Accepted)) {
        setStatus(Job::Stopped);
        setTransferChange(Tc_Status, true);
        return;
    }

    startMetalink();
}

void AbstractMetalink::filesSelected()
{
    bool overwriteAll = false;
    bool autoSkip = false;
    bool cancel = false;
    QModelIndexList files = fileModel()->fileIndexes(FileItem::File);
    m_numFilesSelected = 0;

    //sets the CheckState of the fileModel to the according DataSourceFactories
    //and asks the user if there are existing files already
    foreach (const QModelIndex &index, files)
    {
        const KUrl dest = fileModel()->getUrl(index);
        bool doDownload = index.data(Qt::CheckStateRole).toBool();
        if (m_dataSourceFactory.contains(dest))
        {
            DataSourceFactory *factory = m_dataSourceFactory[dest];
            //ignore finished transfers
            if ((factory->status() == Job::Finished) || (factory->status() == Job::FinishedKeepAlive)) {
                continue;
            }

            //check if the file at dest exists already and ask the user what to do in this case, ignore already running transfers
            if (doDownload && (factory->status() != Job::Running) && QFile::exists(dest.toLocalFile())) {
                //usere has chosen to skip all files that exist already before
                if (autoSkip) {
                    fileModel()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
                    doDownload = false;
                //ask the user, unless he has choosen overwriteAll before
                } else if (!overwriteAll) {
                    KIO::RenameDialog dlg(0, i18n("File already exists"), index.data().toString(), dest, KIO::RenameDialog_Mode(KIO::M_MULTI | KIO::M_OVERWRITE | KIO::M_SKIP));
                    const int result = dlg.exec();

                    if (result == KIO::R_RENAME) {
                        //no reason to use FileModel::rename() since the file does not exist yet, so simply skip it
                        //avoids having to deal with signals
                        const KUrl newDest = dlg.newDestUrl();
                        factory->setDoDownload(doDownload);
                        factory->setNewDestination(newDest);
                        fileModel()->setData(index, newDest.fileName(), FileItem::File);
                        ++m_numFilesSelected;

                        m_dataSourceFactory.remove(dest);
                        m_dataSourceFactory[newDest] = factory;
                        continue;
                    } else if (result == KIO::R_SKIP) {
                        fileModel()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
                        doDownload = false;
                    } else if (result == KIO::R_CANCEL) {
                        cancel = true;
                        break;
                    } else if (result == KIO::R_AUTO_SKIP) {
                        autoSkip = true;
                        fileModel()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
                        doDownload = false;
                    } else if (result == KIO::R_OVERWRITE_ALL) {
                        overwriteAll = true;
                    }
                }
            }

            factory->setDoDownload(doDownload);
            if (doDownload && (factory->status() != Finished) && (factory->status() != FinishedKeepAlive)) {
                ++m_numFilesSelected;
            }
        }
    }

    //the user decided to cancel, so untick all files
    if (cancel) {
        m_numFilesSelected = 0;
        untickAllFiles();
        foreach (DataSourceFactory *factory, m_dataSourceFactory) {
            factory->setDoDownload(false);
        }
    }

    Transfer::ChangesFlags change = (Tc_TotalSize | Tc_DownloadSpeed);
    //some files have been selected that are not finished yet, set them to stop if the transfer is not running (checked in slotStatus)
    if (m_numFilesSelected) {
        change |= Tc_Status;
    }
    slotDataSourceFactoryChange(change);
}

#include "abstractmetalink.moc"

