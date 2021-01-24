/***************************************************************************
*   Copyright (C) 2007-2014 Lukas Appelhans <l.appelhans@gmx.de>          *
*   Copyright (C) 2008 Dario Freddi <drf54321@gmail.com>                  *
*   Copyright (C) 2010 Matthias Fuchs <mat69@gmx.net>                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
***************************************************************************/

#include "urlchecker.h"
#include "urlchecker_p.h"
#include "mainwindow.h"
#include "core/filedeleter.h"
#include "core/kget.h"
#include "core/transferhandler.h"
#include "core/transfertreemodel.h"
#include "settings.h"

#include <algorithm>
#include <boost/bind.hpp>

#include "kget_debug.h"

#include <QCheckBox>
#include <QFileInfo>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <KIO/RenameDialog>
#include <KLocalizedString>
#include <KSeparator>
#include <KStandardGuiItem>

ExistingTransferDialog::ExistingTransferDialog(const QString &text, const QString &caption, QWidget *parent)
  : QDialog(parent)
{
    setWindowTitle(caption.isEmpty() ? i18n("Question") : caption);
    setModal(true);

    auto *layout = new QVBoxLayout;
    auto *bottomLayout = new QHBoxLayout;

    auto *label = new QLabel(text, this);
    layout->addWidget(label);
    layout->addWidget(new KSeparator(Qt::Horizontal, this));

    m_applyAll = new QCheckBox(i18n("Appl&y to all"), this);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(m_applyAll);

    auto *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Yes), &QPushButton::clicked, this, &ExistingTransferDialog::slotYesClicked);
    connect(buttonBox->button(QDialogButtonBox::No), &QPushButton::clicked, this, &ExistingTransferDialog::slotNoClicked);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &ExistingTransferDialog::slotCancelClicked);
    bottomLayout->addWidget(buttonBox);
    layout->addLayout(bottomLayout, 0);
    
    setLayout(layout);
}

void ExistingTransferDialog::slotYesClicked()
{
    done(m_applyAll->isChecked() ? YesAll : Yes);
}

void ExistingTransferDialog::slotNoClicked()
{
    done(m_applyAll->isChecked() ? NoAll : No);
}

void ExistingTransferDialog::slotCancelClicked()
{
    done(Cancel);
}


UrlChecker::UrlChecker(UrlType type)
  : m_type(type),
    m_cancel(false),
    m_overwriteAll(false),
    m_autoRenameAll(false),
    m_skipAll(false)
{
}

UrlChecker::~UrlChecker()
{
}

///Static methods following

struct lessThan
{
    bool operator()(const QUrl &lhs, const QUrl &rhs) const
    {
        return lhs.url() < rhs.url();
    }
};

void UrlChecker::removeDuplicates(QList<QUrl> &urls)
{
    std::sort(urls.begin(), urls.end(), lessThan());//sort the urls, to find duplicates fast
    urls.erase(std::unique(urls.begin(), urls.end(),
               boost::bind(&QUrl::matches, _1, _2, QUrl::StripTrailingSlash | QUrl::NormalizePathSegments)), urls.end());
}

UrlChecker::UrlError UrlChecker::checkUrl(const QUrl &url, const UrlChecker::UrlType type, bool showNotification)
{
    switch (type) {
        case Source:
            return checkSource(url, showNotification);
        case Destination:
            return checkDestination(url, showNotification);
        case Folder:
            return checkFolder(url, showNotification);
    }

    return NoError;
}

bool UrlChecker::wouldOverwrite(const QUrl &source, const QUrl &dest)
{
    return (dest.isLocalFile() && QFile::exists(dest.toLocalFile()) && source != dest && !FileDeleter::isFileBeingDeleted(dest));
}

UrlChecker::UrlError UrlChecker::checkSource(const QUrl &src, bool showNotification)
{
    //NOTE hasPath is not used, as this would disallow addresses like https://www.kde.org/ as there is no path
    UrlError error = NoError;
    if (src.isEmpty()) {
        return Empty;
    }
    if ((error == NoError) && !src.isValid()) {
        error = Invalid;
    }
    if ((error == NoError) && src.scheme().isEmpty()){
        error = NoProtocol;
    }
    /*if ((error == NoError) && !src.hasHost()) {//FIXME deactivated to allow file://....metalink etc
        error = NoHost;
    }*/

    if (showNotification && (error != NoError)) {
        qCDebug(KGET_DEBUG) << "Source:" << src << "has error:" << error;
        KGet::showNotification(KGet::m_mainWindow, "error", message(src, Source, error));
    }

    //TODO also check sourceUrl.url() != QUrl(sourceUrl.url()).fileName() as in NewTransferDialog::setSource?

    return error;
}

UrlChecker::UrlError UrlChecker::checkDestination(const QUrl &destination, bool showNotification)
{
    UrlError error = NoError;

    if (destination.isEmpty()) {
        error = Empty;
    }

    if (error == NoError) {
        //not supposed to be a folder
        QFileInfo fileInfo(destination.toLocalFile());
        if (!destination.isValid() || fileInfo.isDir()) {
            error = Invalid;
        }

        qDebug() << "Adjusted destination:" << destination.adjusted(QUrl::RemoveFilename).path();
        if ((error == NoError) && !QFileInfo(destination.adjusted(QUrl::RemoveFilename).path()).isWritable()) {
            error = NotWriteable;
        }
    }
        
    qCDebug(KGET_DEBUG) << "Destination:" << destination << "has error:" << error;

    if (showNotification && (error != NoError)) {
        KGet::showNotification(KGet::m_mainWindow, "error", message(destination, Destination, error));
    }

    return error;
}


UrlChecker::UrlError UrlChecker::checkFolder(const QUrl &folder, bool showNotification)
{
    UrlError error = NoError;

    const QString destDir = folder.toLocalFile();
    if (folder.isEmpty() || destDir.isEmpty()) {
        error = Empty;
    }

    if (error == NoError) {
        //has to be a folder
        QFileInfo fileInfo(destDir);
        if (!folder.isValid() || !fileInfo.isDir()) {
            error = Invalid;
        }

        //has to be writeable
        if ((error == NoError) && !fileInfo.isWritable()) {
            error = NotWriteable;
        }
    }

    if (showNotification && (error != NoError)) {
        qCDebug(KGET_DEBUG) << "Folder:" << folder << "has error:" << error;
        KGet::showNotification(KGet::m_mainWindow, "error", message(folder, Folder, error));
    }

    return error;
}

QUrl UrlChecker::destUrl(const QUrl &destOrFolder, const QUrl &source, const QString &fileName)
{
    QUrl dest = destOrFolder;
    if (QFileInfo(dest.toLocalFile()).isDir()) {
        QString usedFileName = (fileName.isEmpty() ? source.fileName() : fileName);
        if (usedFileName.isEmpty()) {
            usedFileName = QUrl::toPercentEncoding(source.toString(), "/");
        }
        if (!dest.path().endsWith('/')) dest.setPath(dest.path() + '/');
        dest.setPath(dest.adjusted(QUrl::RemoveFilename).path() + usedFileName);
    } else if (!fileName.isEmpty()) {
        dest.setPath(dest.adjusted(QUrl::RemoveFilename).path() + fileName);
    }

    return dest;
}

TransferHandler *UrlChecker::existingTransfer(const QUrl &url, const UrlChecker::UrlType type, UrlChecker::UrlWarning *warning)
{
    UrlWarning temp;
    UrlWarning &warn = (warning ? (*warning) : temp);
    warn = NoWarning;

    switch (type) {
        case Source:
            return existingSource(url, warn);
        case Destination:
            return existingDestination(url, warn);
        default:
            return nullptr;
    }
}

TransferHandler *UrlChecker::existingSource(const QUrl &source, UrlChecker::UrlWarning &warning)
{
    // Check if a transfer with the same url already exists
    Transfer *transfer = KGet::m_transferTreeModel->findTransfer(source);
    if (transfer) {
        if (transfer->status() == Job::Finished) {
            warning = ExistingFinishedTransfer;
        } else {
            warning = ExistingTransfer;
        }
    }

    return (transfer ? transfer->handler() : nullptr);
}

TransferHandler *UrlChecker::existingDestination(const QUrl &url, UrlChecker::UrlWarning &warning)
{
    Transfer *transfer = KGet::m_transferTreeModel->findTransferByDestination(url);
    if (transfer) {
        if (transfer->status() == Job::Finished) {
            warning = ExistingFinishedTransfer;
        } else {
            warning = ExistingTransfer;
        }
    } else if (QFile::exists(url.toString())) {
        warning = ExistingFile;
    }

    return (transfer ? transfer->handler() : nullptr);
}


QString UrlChecker::message(const QUrl &url, const UrlChecker::UrlType type, const UrlChecker::UrlError error)
{
    if (url.isEmpty()) {
        if (type == Folder) {
            switch (error) {
                case Empty:
                    return i18n("No download directory specified.");
                case Invalid:
                    return i18n("Invalid download directory specified.");
                case NotWriteable:
                    return i18n("Download directory is not writeable.");
                default:
                    return QString();
            }
        }
        if (type == Destination) {
            switch (error) {
                case Empty:
                    return i18n("No download destination specified.");
                case Invalid:
                    return i18n("Invalid download destination specified.");
                case NotWriteable:
                    return i18n("Download destination is not writeable.");
                default:
                    return QString();
            }
        }
        if (type == Source) {
            switch (error) {
                case Empty:
                    return i18n("No URL specified.");
                case Invalid:
                    return i18n("Malformed URL.");
                case NoProtocol:
                    return i18n("Malformed URL, protocol missing.");
                case NoHost:
                    return i18n("Malformed URL, host missing.");
                default:
                    return QString();
            }
        }
    } else {
        const QString urlString = url.toString();
        if (type == Folder) {
            switch (error) {
                case Empty:
                    return i18n("No download directory specified.");
                case Invalid:
                    return i18n("Invalid download directory specified:\n%1", urlString);
                case NotWriteable:
                    return i18n("Download directory is not writeable:\n%1", urlString);
                default:
                    return QString();
            }
        }
        if (type == Destination) {
            switch (error) {
                case Empty:
                    return i18n("No download destination specified.");
                case Invalid:
                    return i18n("Invalid download destination specified:\n%1", urlString);
                case NotWriteable:
                    return i18n("Download destination is not writeable:\n%1", urlString);
                default:
                    return QString();
            }
        }
        if (type == Source) {
            switch (error) {
                case Empty:
                    return i18n("No URL specified.");
                case Invalid:
                    return i18n("Malformed URL:\n%1", urlString);
                case NoProtocol:
                    return i18n("Malformed URL, protocol missing:\n%1", urlString);
                case NoHost:
                    return i18n("Malformed URL, host missing:\n%1", urlString);
                default:
                    return QString();
            }
        }
    }

    return QString();
}

QString UrlChecker::message(const QUrl &url, const UrlChecker::UrlType type, const UrlChecker::UrlWarning warning)
{
    if (url.isEmpty()) {
        if (type == Destination) {
            switch (warning) {
                case ExistingFile:
                    return i18n("File already exists. Overwrite it?");
                case ExistingFinishedTransfer:
                    return i18n("You have already downloaded that file from another location.\nDownload and delete the previous one?");
                case ExistingTransfer:
                    return i18n("You are already downloading that file from another location.\nDownload and delete the previous one?");
                default:
                    return QString();
            }
        }
        if (type == Source) {
            switch (warning) {
                case ExistingFile:
                    return i18n("File already exists. Overwrite it?");
                case ExistingFinishedTransfer:
                    return i18n("You have already completed a download from that location. Download it again?");
                case ExistingTransfer:
                    return i18n("You have a download in progress from that location.\nDelete it and download again?");
                default:
                    return QString();
            }
        }
    } else {
        const QString urlString = url.toString();
        if (type == Destination) {
            switch (warning) {
                case ExistingFile:
                    return i18n("File already exists:\n%1\nOverwrite it?", urlString);
                case ExistingFinishedTransfer:
                    return i18n("You have already downloaded that file from another location.\nDownload and delete the previous one?");
                case ExistingTransfer:
                    return i18n("You are already downloading that file from another location.\nDownload and delete the previous one?");
                default:
                    return QString();
            }
        }
        if (type == Source) {
            switch (warning) {
                case ExistingFinishedTransfer:
                    return i18n("You have already completed a download from the location: \n\n%1\n\nDownload it again?", urlString);
                case ExistingTransfer:
                    return i18n("You have a download in progress from the location: \n\n%1\n\nDelete it and download again?", urlString);
                default:
                    return QString();
            }
        }
    }

    return QString();
}

QString UrlChecker::message(const QList<QUrl> &urls, const UrlChecker::UrlType type, const UrlChecker::UrlError error)
{
    QString urlsString;
    if (!urls.isEmpty()) {
        urlsString = urls.first().toString();
        for (int i = 1; i < urls.count(); ++i) {
            urlsString += '\n' + urls[i].toString();
        }
        urlsString = QString("<p style=\"font-size: small;\">\%1</p>").arg(urlsString);
    }

    if (urls.isEmpty()) {
        if ((type == Folder) || (type == Destination)) {
            return message(QUrl(), type, error);
        }
        if (type == Source) {
            switch (error) {
                case Empty:
                    return i18n("No URL specified.");
                case Invalid:
                    return i18n("Malformed URLs.");
                case NoProtocol:
                    return i18n("Malformed URLs, protocol missing.");
                case NoHost:
                    return i18n("Malformed URLs, host missing.");
                default:
                    return QString();
            }
        }
    } else {
        switch (error) {
            case Empty:
                return i18n("No URL specified.");
            case Invalid:
                return i18n("Malformed URLs:\n%1", urlsString);
            case NoProtocol:
                return i18n("Malformed URLs, protocol missing:\n%1", urlsString);
            case NoHost:
                return i18n("Malformed URLs, host missing:\n%1", urlsString);
            case NotWriteable:
                return i18n("Destinations are not writable:\n%1", urlsString);
            default:
                return QString();
        }
    }

    return QString();
}

QString UrlChecker::message(const QList<QUrl> &urls, const UrlChecker::UrlType type, const UrlChecker::UrlWarning warning)
{
    QString urlsString;
    if (!urls.isEmpty()) {
        urlsString = urls.first().toString();
        for (int i = 1; i < urls.count(); ++i) {
            urlsString += '\n' + urls[i].toString();
        }
        urlsString = QString("<p style=\"font-size: small;\">\%1</p>").arg(urlsString);
    }

    if (urls.isEmpty()) {
        if (type == Destination) {
            switch (warning) {
                case ExistingFile:
                    return i18n("Files exist already. Overwrite them?");
                case ExistingFinishedTransfer:
                    return i18n("You have already completed downloads at those destinations. Download them again?");
                case ExistingTransfer:
                    return i18n("You have downloads in progress to these destinations.\nDelete them and download again?");
                default:
                    return QString();
            }
        }
        if (type == Source) {
            switch (warning) {
                case ExistingFinishedTransfer:
                    return i18n("You have already completed downloads from these locations. Download them again?");
                case ExistingTransfer:
                    return i18n("You have downloads in progress from these locations.\nDelete them and download again?");
                default:
                    return QString();
            }
        }
    } else {
        if (type == Destination) {
            switch (warning) {
                case ExistingFile:
                    return i18n("Files exist already:\n%1\nOverwrite them?", urlsString);
                case ExistingFinishedTransfer:
                    return i18n("You have already completed downloads at those destinations: \n\n%1\n\n Download them again?", urlsString);
                case ExistingTransfer:
                    return i18n("You have downloads in progress to these destinations: \n\n%1\n\nDelete them and download again?", urlsString);
                default:
                    return QString();
            }
        }
        if (type == Source) {
            switch (warning) {
                case ExistingFinishedTransfer:
                    return i18n("You have already completed downloads from these locations: \n\n%1\n\nDownload them again?", urlsString);
                case ExistingTransfer:
                    return i18n("You have downloads in progress from these locations: \n\n%1\n\nDelete them and download again?", urlsString);
                default:
                    QString();
            }
        }
    }

    return QString();
}


QList<QUrl> UrlChecker::hasExistingTransferMessages(const QList<QUrl> &urls, const UrlChecker::UrlType type)
{
    UrlWarning warning;
    QHash<UrlWarning, QList<QPair<QUrl, TransferHandler*> > > splitWarnings;
    QList<QUrl> urlsToDownload;

    //collect all errors
    foreach(const QUrl &url, urls) {
        TransferHandler *transfer = existingTransfer(url, type, &warning);
        if (transfer) {
            splitWarnings[warning] << qMakePair(url, transfer);
        } else {
            urlsToDownload << url;
        }
    }

    //First ask about unfinished existing transfers
    QList<QPair<QUrl, TransferHandler*> >::const_iterator it;
    QList<QPair<QUrl, TransferHandler*> >::const_iterator itEnd;
    QList<UrlWarning> orderOfExecution;
    QList<TransferHandler*> toDelete;
    orderOfExecution << ExistingTransfer << ExistingFinishedTransfer;
    for (int i = 0; i < orderOfExecution.count(); ++i) {
        warning = orderOfExecution[i];
        if (splitWarnings.contains(warning)) {
            QList<QPair<QUrl, TransferHandler*> > existing = splitWarnings[warning];
            itEnd = existing.constEnd();
            bool isYesAll = false;
            bool isNoAll = false;
            for (it = existing.constBegin(); it != itEnd; ++it) {
                if (isYesAll) {
                    urlsToDownload << it->first;
                    toDelete << it->second;
                    continue;
                }

                if (isNoAll) {
                    break;
                }

                int result;
                if (Settings::filesOverwrite() || (Settings::filesAutomaticRename() && (warning != ExistingTransfer))) {
                    result = ExistingTransferDialog::ExistingDialogReturn::YesAll;
                } else {
                    result = hasExistingDialog(it->first, type, warning);
                }
                switch (result) {
                    case ExistingTransferDialog::ExistingDialogReturn::YesAll:
                        isYesAll = true;
                        // fallthrough
                    case ExistingTransferDialog::ExistingDialogReturn::Yes:
                        urlsToDownload << it->first;
                        toDelete << it->second;
                        break;
                    case ExistingTransferDialog::ExistingDialogReturn::NoAll:
                        isNoAll = true;
                    case ExistingTransferDialog::ExistingDialogReturn::No:
                        break;
                    case ExistingTransferDialog::ExistingDialogReturn::Cancel:
                    default:
                        removeTransfers(toDelete);
                        return urlsToDownload;
                }
            }
        }
    }

    removeTransfers(toDelete);
    return urlsToDownload;
}

void UrlChecker::removeTransfers(const QList<TransferHandler*> &toRemove)
{
    QList<TransferHandler*> transfers = toRemove;
    transfers.removeAll(nullptr);
    if (!transfers.isEmpty()) {
        KGet::delTransfers(transfers);
    }
}


int UrlChecker::hasExistingDialog(const QUrl &url, const UrlChecker::UrlType type, const UrlChecker::UrlWarning warning)
{
    QWidget *parent = KGet::m_mainWindow;

    //getting the caption
    QString caption;
    if (type == Source) {
        switch (warning) {
            case ExistingFinishedTransfer:
                caption = i18n("Delete it and download again?");
                break;
            case ExistingTransfer:
                caption = i18n("Download it again?");
                break;
            default:
                break;
        }
    } else if (type == Destination) {
        switch (warning) {
            case ExistingFinishedTransfer:
            case ExistingTransfer:
                caption = i18n("File already downloaded. Download anyway?");
                break;
            case ExistingFile:
                caption = i18n("File already exists");
                break;
            default:
                break;
        }
    }

    QScopedPointer<QDialog> dialog(new ExistingTransferDialog(message(url, type, warning), caption, parent));

    return dialog->exec();
}

///Non static methods following

void UrlChecker::clear()
{
    m_correctUrls.clear();
    m_splitErrorUrls.clear();
    m_cancel = false;
    m_overwriteAll = false;
    m_autoRenameAll = false;
    m_skipAll = false;
}

UrlChecker::UrlType UrlChecker::type() const
{
    return m_type;
}

void UrlChecker::setType(UrlChecker::UrlType type)
{
    clear();
    m_type = type;
}

UrlChecker::UrlError UrlChecker::addUrl(const QUrl &url)
{
    const UrlError error = checkUrl(url, m_type);
    if (error == NoError) {
        m_correctUrls << url;
    } else {
        m_splitErrorUrls[error] << url;
    }

    return error;
}

bool UrlChecker::addUrls(const QList<QUrl> &urls)
{
    bool worked = true;
    foreach (const QUrl &url, urls) {
        const UrlError error = addUrl(url);
        if (error != NoError) {
            worked = false;
        }
    }

    return worked;
}

void UrlChecker::existingTransfers()
{
    m_correctUrls = hasExistingTransferMessages(correctUrls(), m_type);
}

QUrl UrlChecker::checkExistingFile(const QUrl &source, const QUrl &destination)
{
    QUrl newDestination = destination;

    //any url is ignored
    if (m_cancel) {
        return QUrl();
    }

    if (Settings::filesOverwrite()) {
        m_overwriteAll = true;
    } else if (Settings::filesAutomaticRename()) {
        m_autoRenameAll = true;
    }

    if (wouldOverwrite(source, destination)) {
        KIO::RenameDialog_Options args = KIO::RenameDialog_MultipleItems | KIO::RenameDialog_Skip | KIO::RenameDialog_Overwrite;
        QScopedPointer<KIO::RenameDialog> dlg(new KIO::RenameDialog(KGet::m_mainWindow, i18n("File already exists"), source,
                                    destination, args));

        ///in the following cases no dialog needs to be shown
        if (m_skipAll) { //only existing are ignored
            return QUrl();
        } else if (m_overwriteAll) {
            FileDeleter::deleteFile(newDestination);
            return newDestination;
        } else if (m_autoRenameAll) {
            newDestination = dlg->autoDestUrl();
            return newDestination;
        }

        ///now show the dialog and look at the result
        const int result = dlg->exec();
        switch (result) {
            case KIO::Result_Overwrite: {
                //delete the file, that way it won't show up in future calls of this method
                FileDeleter::deleteFile(newDestination);
                return newDestination;
            }
            case KIO::Result_OverwriteAll: {

                //delete the file, that way it won't show up in future calls of this method
                FileDeleter::deleteFile(newDestination);
                m_overwriteAll = true;
                return newDestination;
            }
            case KIO::Result_Rename:
                //call it again, as there is no check on the user input
                return checkExistingFile(source, dlg->newDestUrl());
            case KIO::Result_AutoRename:
                newDestination = dlg->autoDestUrl();
                m_autoRenameAll = true;
                return newDestination;
            case KIO::Result_Skip:
                return QUrl();
            case KIO::Result_AutoSkip:
                m_skipAll = true;
                return QUrl();
            case KIO::Result_Cancel:
                m_cancel = true;
                return QUrl();
            default:
                return QUrl();
        }
    }

    return newDestination;
}


QList<QUrl> UrlChecker::correctUrls() const
{
    return m_correctUrls;
}

QList<QUrl> UrlChecker::errorUrls() const
{
    QList<QUrl> urls;

    QHash<UrlChecker::UrlError, QList<QUrl>>::const_iterator it;
    QHash<UrlChecker::UrlError, QList<QUrl>>::const_iterator itEnd = m_splitErrorUrls.constEnd();
    for (it = m_splitErrorUrls.constBegin(); it != itEnd; ++it) {
        urls << (*it);
    }

    return urls;
}

QHash<UrlChecker::UrlError, QList<QUrl>> UrlChecker::splitErrorUrls() const
{
    return m_splitErrorUrls;
}

void UrlChecker::displayErrorMessages()
{
    QHash<UrlChecker::UrlError, QList<QUrl>>::const_iterator it;
    QHash<UrlChecker::UrlError, QList<QUrl>>::const_iterator itEnd = m_splitErrorUrls.constEnd();
    for (it = m_splitErrorUrls.constBegin(); it != itEnd; ++it) {
        QString m;
        if (it->count() > 1) {
            m = message(*it, m_type, it.key());
        } else if (!it->isEmpty()) {
            m = message(it->first(), m_type, it.key());
        }

        if (!m.isEmpty()) {
            KGet::showNotification(KGet::m_mainWindow, "error", m);
        }
    }
}
