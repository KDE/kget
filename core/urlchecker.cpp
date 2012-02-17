/***************************************************************************
*   Copyright (C) 2007-2009 Lukas Appelhans <l.appelhans@gmx.de>          *
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

#include <QtCore/QFileInfo>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>

#include <KDialogButtonBox>
#include <KIO/RenameDialog>
#include <KLocale>
#include <KSeparator>
#include <KStandardGuiItem>

ExistingTransferDialog::ExistingTransferDialog(const QString &text, const QString &caption, QWidget *parent)
  : KDialog(parent)
{
    setCaption(caption.isEmpty() ? i18n("Question") : caption);
    setModal(true);
    setButtons(0);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    QHBoxLayout *bottomLayout = new QHBoxLayout;

    QLabel *label = new QLabel(text, this);
    layout->addWidget(label);
    layout->addWidget(new KSeparator(Qt::Horizontal, this));

    m_applyAll = new QCheckBox(i18n("Appl&y to all"), this);
    bottomLayout->addStretch(1);
    bottomLayout->addWidget(m_applyAll);

    KDialogButtonBox *buttonBox = new KDialogButtonBox(this);
    buttonBox->addButton(KStandardGuiItem::yes(), QDialogButtonBox::YesRole, this, SLOT(slotYesClicked()));
    buttonBox->addButton(KStandardGuiItem::no(), QDialogButtonBox::NoRole, this, SLOT(slotNoClicked()));
    buttonBox->addButton(KStandardGuiItem::cancel(), QDialogButtonBox::RejectRole, this, SLOT(slotCancelClicked()));
    bottomLayout->addWidget(buttonBox);
    layout->addLayout(bottomLayout, 0);

    widget->setLayout(layout);
    setMainWidget(widget);
}

void ExistingTransferDialog::slotYesClicked()
{
   done(m_applyAll->isChecked() ? KDialog::User2 : KDialog::Yes);
}

void ExistingTransferDialog::slotNoClicked()
{
   done(m_applyAll->isChecked() ? KDialog::User1 : KDialog::No);
}

void ExistingTransferDialog::slotCancelClicked()
{
    done(QDialog::Rejected);
}


UrlChecker::UrlChecker(UrlType type)
  : m_type(type),
    m_cancle(false),
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
    bool operator()(const KUrl &lhs, const KUrl &rhs) const
    {
        return lhs.url() < rhs.url();
    }
};

void UrlChecker::removeDuplicates(KUrl::List &urls)
{
    std::sort(urls.begin(), urls.end(), lessThan());//sort the urls, to find duplicates fast
    urls.erase(std::unique(urls.begin(), urls.end(),
               boost::bind(&KUrl::equals, _1, _2, KUrl::CompareWithoutTrailingSlash | KUrl::AllowEmptyPath)), urls.end());
}

UrlChecker::UrlError UrlChecker::checkUrl(const KUrl &url, const UrlChecker::UrlType type, bool showNotification)
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

bool UrlChecker::wouldOverwrite(const KUrl &source, const KUrl &dest)
{
    return (dest.isLocalFile() && QFile::exists(dest.toLocalFile()) && source != dest && !FileDeleter::isFileBeingDeleted(dest));
}

UrlChecker::UrlError UrlChecker::checkSource(const KUrl &src, bool showNotification)
{
    //NOTE hasPath is not used, as this would dissallow adresses like http://www.kde.org/ as there is no path
    UrlError error = NoError;
    if (src.isEmpty()) {
        return Empty;
    }
    if ((error == NoError) && !src.isValid()) {
        error = Invalid;
    }
    if ((error == NoError) && src.protocol().isEmpty()){
        error = NoProtocol;
    }
    /*if ((error == NoError) && !src.hasHost()) {//FIXME deactivated to allow file://....metalink etc
        error = NoHost;
    }*/

    if (showNotification && (error != NoError)) {
        kDebug(5001) << "Source:" << src << "has error:" << error;
        KGet::showNotification(KGet::m_mainWindow, "error", message(src, Source, error));
    }

    //TODO also check sourceUrl.url() != KUrl(sourceUrl.url()).fileName() as in NewTransferDialog::setSource?

    return error;
}

UrlChecker::UrlError UrlChecker::checkDestination(const KUrl &destination, bool showNotification)
{
    UrlError error = NoError;

    if (destination.isEmpty()) {
        error = Empty;
    }

    if (error == NoError) {
        //not supposed to be a folder
        QFileInfo fileInfo(destination.pathOrUrl());
        if (!destination.isValid() || fileInfo.isDir()) {
            error = Invalid;
        }

        if ((error == NoError) && !QFileInfo(destination.directory()).isWritable()) {
            error = NotWriteable;
        }
    }

    if (showNotification && (error != NoError)) {
        kDebug(5001) << "Destination:" << destination << "has error:" << error;
        KGet::showNotification(KGet::m_mainWindow, "error", message(destination, Destination, error));
    }

    return error;
}


UrlChecker::UrlError UrlChecker::checkFolder(const KUrl &folder, bool showNotification)
{
    UrlError error = NoError;

    const QString destDir = folder.pathOrUrl();
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
        kDebug(5001) << "Folder:" << folder << "has error:" << error;
        KGet::showNotification(KGet::m_mainWindow, "error", message(folder, Folder, error));
    }

    return error;
}

KUrl UrlChecker::destUrl(const KUrl &destOrFolder, const KUrl &source, const QString &fileName)
{
    KUrl dest = destOrFolder;
    if (QFileInfo(dest.toLocalFile()).isDir()) {
        QString usedFileName = (fileName.isEmpty() ? source.fileName() : fileName);
        if (usedFileName.isEmpty()) {
            usedFileName = KUrl::toPercentEncoding(source.prettyUrl(), "/");
        }
        dest.adjustPath(KUrl::AddTrailingSlash);
        dest.setFileName(usedFileName);
    } else if (!fileName.isEmpty()) {
        dest.setFileName(fileName);
    }

    return dest;
}

TransferHandler *UrlChecker::existingTransfer(const KUrl &url, const UrlChecker::UrlType type, UrlChecker::UrlWarning *warning)
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
            return 0;
    }
}

TransferHandler *UrlChecker::existingSource(const KUrl &source, UrlChecker::UrlWarning &warning)
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

    return (transfer ? transfer->handler() : 0);
}

TransferHandler *UrlChecker::existingDestination(const KUrl &url, UrlChecker::UrlWarning &warning)
{
    Transfer *transfer = KGet::m_transferTreeModel->findTransferByDestination(url);
    if (transfer) {
        if (transfer->status() == Job::Finished) {
            warning = ExistingFinishedTransfer;
        } else {
            warning = ExistingTransfer;
        }
    } else if (QFile::exists(url.pathOrUrl())) {
        warning = ExistingFile;
    }

    return (transfer ? transfer->handler() : 0);
}


QString UrlChecker::message(const KUrl &url, const UrlChecker::UrlType type, const UrlChecker::UrlError error)
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
        const QString urlString = url.prettyUrl();
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

QString UrlChecker::message(const KUrl &url, const UrlChecker::UrlType type, const UrlChecker::UrlWarning warning)
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
        const QString urlString = url.prettyUrl();
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

QString UrlChecker::message(const KUrl::List &urls, const UrlChecker::UrlType type, const UrlChecker::UrlError error)
{
    QString urlsString;
    if (!urls.isEmpty()) {
        urlsString = urls.first().prettyUrl();
        for (int i = 1; i < urls.count(); ++i) {
            urlsString += '\n' + urls[i].prettyUrl();
        }
        urlsString = QString("<p style=\"font-size: small;\">\%1</p>").arg(urlsString);
    }

    if (urls.isEmpty()) {
        if ((type == Folder) || (type == Destination)) {
            return message(KUrl(), type, error);
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

QString UrlChecker::message(const KUrl::List &urls, const UrlChecker::UrlType type, const UrlChecker::UrlWarning warning)
{
    QString urlsString;
    if (!urls.isEmpty()) {
        urlsString = urls.first().prettyUrl();
        for (int i = 1; i < urls.count(); ++i) {
            urlsString += '\n' + urls[i].prettyUrl();
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


KUrl::List UrlChecker::hasExistingTransferMessages(const KUrl::List &urls, const UrlChecker::UrlType type)
{
    UrlWarning warning;
    QHash<UrlWarning, QList<QPair<KUrl, TransferHandler*> > > splitWarnings;
    KUrl::List urlsToDownload;

    //collect all errors
    foreach(const KUrl &url, urls) {
        TransferHandler *transfer = existingTransfer(url, type, &warning);
        if (transfer) {
            splitWarnings[warning] << qMakePair(url, transfer);
        } else {
            urlsToDownload << url;
        }
    }

    //First ask about unfinished existing transfers
    QList<QPair<KUrl, TransferHandler*> >::const_iterator it;
    QList<QPair<KUrl, TransferHandler*> >::const_iterator itEnd;
    QList<UrlWarning> orderOfExecution;
    QList<TransferHandler*> toDelete;
    orderOfExecution << ExistingTransfer << ExistingFinishedTransfer;
    for (int i = 0; i < orderOfExecution.count(); ++i) {
        warning = orderOfExecution[i];
        if (splitWarnings.contains(warning)) {
            QList<QPair<KUrl, TransferHandler*> > existing = splitWarnings[warning];
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
                    result = YesAll;
                } else {
                    result = hasExistingDialog(it->first, type, warning);
                }
                switch (result) {
                    case YesAll:
                        isYesAll = true;
                    case Yes:
                        urlsToDownload << it->first;
                        toDelete << it->second;
                        break;
                    case NoAll:
                        isNoAll = true;
                    case No:
                        break;
                    case Cancel:
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
    transfers.removeAll(0);
    if (!transfers.isEmpty()) {
        KGet::delTransfers(transfers);
    }
}


int UrlChecker::hasExistingDialog(const KUrl &url, const UrlChecker::UrlType type, const UrlChecker::UrlWarning warning)
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

    QScopedPointer<KDialog> dialog(new ExistingTransferDialog(message(url, type, warning), caption, parent));

    const int result = dialog->exec();
    switch (result) {
        case QDialog::Rejected:
            return Cancel;
        case KDialog::Yes:
            return Yes;
        case KDialog::User2:
            return YesAll;
        case KDialog::No:
            return No;
        case KDialog::User1:
            return NoAll;
        default:
            return result;
    }
}

///Non static methods following

void UrlChecker::clear()
{
    m_correctUrls.clear();
    m_splitErrorUrls.clear();
    m_cancle = false;
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

UrlChecker::UrlError UrlChecker::addUrl(const KUrl &url)
{
    const UrlError error = checkUrl(url, m_type);
    if (error == NoError) {
        m_correctUrls << url;
    } else {
        m_splitErrorUrls[error] << url;
    }

    return error;
}

bool UrlChecker::addUrls(const KUrl::List &urls)
{
    bool worked = true;
    foreach (const KUrl &url, urls) {
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

KUrl UrlChecker::checkExistingFile(const KUrl &source, const KUrl &destination)
{
    KUrl newDestination = destination;

    //any url is ignored
    if (m_cancle) {
        return KUrl();
    }

    if (Settings::filesOverwrite()) {
        m_overwriteAll = true;
    } else if (Settings::filesAutomaticRename()) {
        m_autoRenameAll = true;
    }

    if (wouldOverwrite(source, destination)) {
        KIO::RenameDialog_Mode args = static_cast<KIO::RenameDialog_Mode>(KIO::M_MULTI | KIO::M_SKIP | KIO::M_OVERWRITE);
        QScopedPointer<KIO::RenameDialog> dlg(new KIO::RenameDialog(KGet::m_mainWindow, i18n("File already exists"), source,
                                    destination, args));

        ///in the following cases no dialog needs to be shown
        if (m_skipAll) { //only existing are ignored
            return KUrl();
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
            case KIO::R_OVERWRITE: {
                //delete the file, that way it won't show up in future calls of this method
                FileDeleter::deleteFile(newDestination);
                return newDestination;
            }
            case KIO::R_OVERWRITE_ALL: {

                //delete the file, that way it won't show up in future calls of this method
                FileDeleter::deleteFile(newDestination);
                m_overwriteAll = true;
                return newDestination;
            }
            case KIO::R_RENAME:
                //call it again, as there is no check on the user input
                return checkExistingFile(source, dlg->newDestUrl());
            case KIO::R_AUTO_RENAME:
                newDestination = dlg->autoDestUrl();
                m_autoRenameAll = true;
                return newDestination;
            case KIO::R_SKIP:
                return KUrl();
            case KIO::R_AUTO_SKIP:
                m_skipAll = true;
                return KUrl();
            case KIO::R_CANCEL:
                m_cancle = true;
                return KUrl();
            default:
                return KUrl();
        }
    }

    return newDestination;
}


KUrl::List UrlChecker::correctUrls() const
{
    return m_correctUrls;
}

KUrl::List UrlChecker::errorUrls() const
{
    KUrl::List urls;

    QHash<UrlChecker::UrlError, KUrl::List>::const_iterator it;
    QHash<UrlChecker::UrlError, KUrl::List>::const_iterator itEnd = m_splitErrorUrls.constEnd();
    for (it = m_splitErrorUrls.constBegin(); it != itEnd; ++it) {
        urls << (*it);
    }

    return urls;
}

QHash<UrlChecker::UrlError, KUrl::List> UrlChecker::splitErrorUrls() const
{
    return m_splitErrorUrls;
}

void UrlChecker::displayErrorMessages()
{
    QHash<UrlChecker::UrlError, KUrl::List>::const_iterator it;
    QHash<UrlChecker::UrlError, KUrl::List>::const_iterator itEnd = m_splitErrorUrls.constEnd();
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
