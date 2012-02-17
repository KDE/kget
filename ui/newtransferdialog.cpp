/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>
   Copyright (C) 2007 by Javier Goday <jgoday@gmail.com>
   Copyright (C) 2008 - 2009 by Lukas Appelhans <l.appelhans@gmx.de>
   Copyright (C) 2010 by Matthias Fuchs <mat69@gmx.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include "newtransferdialog.h"

#include "core/filedeleter.h"
#include "core/kget.h"
#include "mainwindow.h"
#include "core/mostlocalurl.h"
#include "core/transfertreemodel.h"
#include "core/transfergrouphandler.h"
#include "core/plugin/transferfactory.h"
#include "core/urlchecker.h"
#include "settings.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QtCore/QTimer>

#include <KLocale>
#include <QListWidgetItem>
#include <KColorScheme>
#include <KDebug>
#include <KFileDialog>
#include <KWindowSystem>

K_GLOBAL_STATIC(NewTransferDialogHandler, newTransferDialogHandler)


NewTransferDialog::NewTransferDialog(QWidget *parent)
  : KDialog(parent),
    m_window(0),
    m_existingTransfer(0),
    m_multiple(false),
    m_overWriteSingle(false)
{
    setModal(true);
    setCaption(i18n("New Download"));
    showButtonSeparator(true);

    QWidget *widget = new QWidget(this);
    ui.setupUi(widget);
    setMainWidget(widget);

    enableButtonOk(false);

    //timer to avoid constant checking of the input
    m_timer = new QTimer(this);
    m_timer->setInterval(350);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkInput()));

    const KColorScheme scheme = KColorScheme(QPalette::Active, KColorScheme::View);
    m_existingFileBackground = scheme.background(KColorScheme::NeutralBackground);
    m_normalBackground = scheme.background();


    // properties of the m_destRequester combobox
    ui.destRequester->comboBox()->setDuplicatesEnabled(false);
    ui.destRequester->comboBox()->setUrlDropsEnabled(true);
    ui.destRequester->comboBox()->setEditable(true);
    ui.destRequester->fileDialog()->setKeepLocation(true);

    ui.errorWidget->setCloseButtonVisible(false);

    connect(ui.groupComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setDefaultDestination()));

    connect(ui.urlRequester, SIGNAL(textChanged(QString)), this, SLOT(setDefaultDestination()));
    connect(ui.destRequester, SIGNAL(textChanged(QString)), this, SLOT(inputTimer()));
    connect(ui.urlRequester, SIGNAL(textChanged(QString)), this, SLOT(inputTimer()));
    connect(ui.listWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(inputTimer()));
    connect(this, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
}

NewTransferDialog::~NewTransferDialog()
{
}

void NewTransferDialog::setMultiple(bool useMultiple)
{
    m_multiple = useMultiple;

    const Qt::Alignment alignment = Qt::AlignLeft | (m_multiple ? Qt::AlignTop : Qt::AlignVCenter);
    ui.urlLabel->setAlignment(alignment);
    ui.urlRequester->setVisible(!m_multiple);
    ui.listWidget->setVisible(m_multiple);
    ui.destRequester->setMode(m_multiple ? KFile::Directory : KFile::File);
}

void NewTransferDialog::clear()
{
    ui.urlRequester->clear();
    ui.urlRequester->setFocus();
    ui.listWidget->clear();
    ui.destRequester->comboBox()->clear();
    ui.destRequester->clear();
    m_destination.clear();
    m_sources.clear();
    m_existingTransfer = 0;
    m_overWriteSingle = false;

    //add all destinations
    QStringList list;
    QString downloadPath = KGet::generalDestDir();
    if (!downloadPath.isEmpty()) {
        if (!downloadPath.endsWith('/')) {
            downloadPath.append('/');
        }
        list << downloadPath;
    }
    foreach (TransferGroupHandler *handler, KGet::allTransferGroups()) {
        const QString folder = handler->defaultFolder();
        if (!folder.isEmpty()) {
            list << (folder.endsWith('/') ? folder : folder + '/');
        }
    }

    list.removeDuplicates();
    ui.destRequester->comboBox()->insertItems(0, list);

    //add all transfer groups
    ui.groupComboBox->clear();
    foreach (TransferGroupHandler *group, KGet::allTransferGroups()) {
        ui.groupComboBox->addItem(KIcon(group->iconName()), group->name());
    }
    ui.groupComboBox->setCurrentItem(Settings::lastGroup());
    if (ui.groupComboBox->currentIndex() == -1) {
        ui.groupComboBox->setCurrentIndex(0);
    }

    const bool multipleGroups = KGet::transferGroupNames().count();
    ui.groupComboBox->setVisible(multipleGroups);
    ui.groupLabel->setVisible(multipleGroups);
}

void NewTransferDialog::setSource(const KUrl::List &sources)
{
    if (sources.isEmpty()) {
        return;
    }

    if (sources.count() == 1) {
        KUrl m_srcUrl = sources.first().url();
        ui.urlRequester->clear();
        if (m_srcUrl.isEmpty()) {
            m_srcUrl = KUrl(QApplication::clipboard()->text(QClipboard::Clipboard).trimmed());
        }

        if (UrlChecker::checkSource(m_srcUrl) == UrlChecker::NoError) {
            ui.urlRequester->insert(m_srcUrl.prettyUrl());
        }
    } else {
        foreach (const KUrl &sourceUrl, sources) {
            if (sourceUrl.url() != KUrl(sourceUrl.url()).fileName()) {//TODO simplify, whatfor is this check anyway, shouldn't the sources be checked already and if not add this to UrlChecker
                kDebug(5001) << "Insert" << sourceUrl;
                QListWidgetItem *newItem = new QListWidgetItem(sourceUrl.pathOrUrl(), ui.listWidget);
                newItem->setCheckState(Qt::Checked);
            }
        }
    }

    const QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(sources.first());
    if (!groups.isEmpty()) {
        ui.groupComboBox->setCurrentIndex(ui.groupComboBox->findText(groups.first()->name()));
    }
}

void NewTransferDialog::setDestinationFileName(const QString &filename)
{
    ui.destRequester->setUrl(QString(ui.destRequester->url().path(KUrl::AddTrailingSlash) + filename));
}

void NewTransferDialog::setDestination()
{
    //sets destRequester to either display the defaultFolder of group or the generalDestDir
    QString group = ui.groupComboBox->currentText();
    TransferGroupHandler * current = 0;
    foreach (TransferGroupHandler * handler, KGet::allTransferGroups()) {
        if (handler->name() == group) {
            current = handler;
            break;
        }
    }

    if (current) {
        QString groupFolder = current->defaultFolder();
        if (groupFolder.isEmpty()) {
            groupFolder = KGet::generalDestDir();
        }
        if (!groupFolder.endsWith('/')) {
            groupFolder.append('/');
        }
        ui.destRequester->comboBox()->setCurrentItem(groupFolder, true);
    }
}

void NewTransferDialog::showDialog(KUrl::List list, const QString &suggestedFileName)
{
    //TODO handle the case where for some there are suggested file names --> own file name column in multiple setting
    //the dialog is already in use, adapt it
    if (isVisible()) {
        list << m_sources;
    }
    clear();//Let's clear the old stuff
    m_sources << list;
    UrlChecker::removeDuplicates(m_sources);
    const int size = m_sources.size();
    kDebug(5001) << "SET SOURCES " << m_sources << " MULTIPLE " << (size > 1);
    setMultiple(size > 1);

    if (size) {
        if (size == 1 && !suggestedFileName.isEmpty()) {
            setDestinationFileName(suggestedFileName);
        }

        setSource(m_sources);
    }

    prepareDialog();
}

void NewTransferDialog::setDefaultDestination()
{
    //NOTE if the user enters a file name manually and the changes the group the manually entered file name will be overwritten
    setDestination();

    //set a file name
    if (!m_multiple) {
        const KUrl url(ui.urlRequester->text().trimmed());
        if ((UrlChecker::checkSource(url) == UrlChecker::NoError) &&
            QFileInfo(ui.destRequester->url().toLocalFile()).isDir()) {
            setDestinationFileName(url.fileName());
        }
    }
}

void NewTransferDialog::prepareDialog()
{
    if (m_window) {
        KWindowInfo info = KWindowSystem::windowInfo(m_window->winId(), NET::WMDesktop, NET::WMDesktop);
        KWindowSystem::setCurrentDesktop(info.desktop());
        KWindowSystem::forceActiveWindow(m_window->winId());
    }

    kDebug(5001) << "Show the dialog!";
    show();
}

bool NewTransferDialog::isEmpty()
{
    return (m_multiple ? !ui.listWidget->count() : ui.urlRequester->text().trimmed().isEmpty());
}

void NewTransferDialog::inputTimer()
{
    enableButtonOk(false);
    m_timer->start();
}

void NewTransferDialog::checkInput()
{
    KUrl source = KUrl(ui.urlRequester->text().trimmed());
    const KUrl dest = ui.destRequester->url();

    //check the destination folder
    UrlChecker::UrlError error = UrlChecker::checkFolder(dest);
    const bool folderValid = (error == UrlChecker::NoError);
    bool destinationValid = false;
    QString infoText;
    QString warningText;
    if (!folderValid) {
        if (m_multiple) {
            infoText = UrlChecker::message(KUrl(), UrlChecker::Folder, error);
        } else {
            //might be a destination instead of a folder
            destinationValid = (UrlChecker::checkDestination(dest) == UrlChecker::NoError);
        }
    } else {
        m_destination = dest;
    }

    //check the source
    if (!m_multiple) {
        source = mostLocalUrl(source);
    }
    error = UrlChecker::checkSource(source);
    const bool sourceValid = (error == UrlChecker::NoError);
    if (!m_multiple && !sourceValid) {
        infoText = UrlChecker::message(KUrl(), UrlChecker::Source, error);
    }

    //check if any sources are checked and for existing transfers or destinations
    bool filesChecked = false;
    if (m_multiple && folderValid) {
        KListWidget *list = ui.listWidget;

        //check if some sources have been checked
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item->checkState() == Qt::Checked) {
                filesChecked = true;
                break;
            }
        }
        if (!filesChecked) {
            infoText = i18n("Select at least one source url.");
        }

        //check if there are existing files
        if (filesChecked) {
            bool existingFile = false;
            for (int i = 0; i < list->count(); ++i) {
                QListWidgetItem *item = list->item(i);
                const KUrl source = KUrl(item->text());
                const KUrl destUrl = UrlChecker::destUrl(dest, source);
                if (UrlChecker::wouldOverwrite(source, destUrl)) {
                    item->setBackground(m_existingFileBackground);
                    existingFile = true;
                } else {
                    item->setBackground(m_normalBackground);
                }
            }
            if (existingFile) {
                warningText = i18n("Files that exist already in the current folder have been marked.");//TODO better message
            }
        }
    }

    //single file
    UrlChecker::UrlWarning warning = UrlChecker::NoWarning;
    if (!m_multiple && sourceValid && (folderValid || destinationValid)) {
        m_destination = UrlChecker::destUrl(dest, source);
        //show only one message for existing transfers
        m_existingTransfer = UrlChecker::existingTransfer(source, UrlChecker::Source, &warning);
        if (m_existingTransfer) {
            warningText = UrlChecker::message(KUrl(), UrlChecker::Source, warning);
        } else {
            m_existingTransfer = UrlChecker::existingTransfer(m_destination, UrlChecker::Destination, &warning);
            if (m_existingTransfer) {
                warningText = UrlChecker::message(KUrl(), UrlChecker::Destination, warning);
            }
        }

        if (UrlChecker::wouldOverwrite(KUrl(ui.urlRequester->text().trimmed()), m_destination)) {
            m_overWriteSingle = true;
            if (!warningText.isEmpty()) {
                warningText += '\n';
            }
            warningText += UrlChecker::message(KUrl(), UrlChecker::Destination, UrlChecker::ExistingFile);
        } else {
            m_overWriteSingle = false;
        }
    }

    if (!infoText.isEmpty()) {
        setInformation(infoText);
    } else if (!warningText.isEmpty()) {
        setWarning(warningText);
    } else {
        ui.errorWidget->hide();
    }

    //activate the ok button
    if (m_multiple) {
        enableButtonOk(folderValid && filesChecked);
    } else {
        enableButtonOk((folderValid || destinationValid) && sourceValid);
    }

    kDebug(5001) << source << source.fileName() << dest << dest.fileName();
}

void NewTransferDialog::slotFinished(int resultCode)
{
    if (resultCode == KDialog::Accepted) {
        dialogAccepted();
    }
    clear();
}

void NewTransferDialog::dialogAccepted()
{
    kDebug(5001) << "Dialog accepted.";

    //an existing transfer has been specified and since ok was clicked, it was chosen to be overwritten
    if (m_existingTransfer) {
        kDebug(5001) << "Removing existing transfer:" << m_existingTransfer;
        KGet::delTransfer(m_existingTransfer);
    }

    //set the last directory
    QString dir = m_destination.toLocalFile();
    if (!QFileInfo(dir).isDir()) {
        dir = m_destination.directory();
    }
    Settings::setLastDirectory(dir);
    Settings::self()->writeConfig();

    const QString group = ui.groupComboBox->currentText();

    ///add data to create transfers
    QList<KGet::TransferData> data;
    if (!m_multiple) {
        if (m_overWriteSingle) {
            kDebug(5001) << "Removing existing file:" << m_destination;
            //removes m_destination if it exists, do that here so that it is removed no matter if a transfer could be created or not
            //as the user decided to throw the file away
            FileDeleter::deleteFile(m_destination);
        }

        //sourceUrl is valid, has been checked before
        const KUrl sourceUrl = KUrl(ui.urlRequester->text().trimmed());
        kDebug(5001) << "Downloading" << sourceUrl << "to" << m_destination;
        data << KGet::TransferData(sourceUrl, m_destination, group);
    } else {
        KUrl::List list;
        for (int i = 0; i != ui.listWidget->count(); ++i) {
            QListWidgetItem *item = ui.listWidget->item(i);

            //find selected sources
            if (item->checkState() == Qt::Checked) {
                //both sourceUrl and destUrl are valid, they have been tested in checkInput
                const KUrl sourceUrl = KUrl(item->text().trimmed());
                const KUrl destUrl = UrlChecker::destUrl(m_destination, sourceUrl);
                kDebug(5001) << "Downloading" << sourceUrl << "to" << destUrl;

                //file exists already, remove it
                if (item->background() == m_existingFileBackground) {
                    kDebug(5001) << "Removing existing file:" << destUrl;
                    //removes destUrl if it exists, do that here so that it is removed no matter if a transfer could be created or not
                    //as the user decided to throw the file away
                    FileDeleter::deleteFile(destUrl);
                }

                data << KGet::TransferData(sourceUrl, destUrl, group);
            }
        }
    }

    if (!data.isEmpty()) {
        Settings::setLastGroup(ui.groupComboBox->currentText());
        KGet::createTransfers(data);
    }
}

void NewTransferDialog::setInformation(const QString &information)
{
    ui.errorWidget->setMessageType(KMessageWidget::Information);
    ui.errorWidget->setText(information);
    ui.errorWidget->setVisible(!information.isEmpty());
}

void NewTransferDialog::setWarning(const QString &warning)
{
    ui.errorWidget->setMessageType(KMessageWidget::Warning);
    ui.errorWidget->setText(warning);
    ui.errorWidget->setVisible(!warning.isEmpty());
}



/**
 * NOTE some checks in this class might seem redundant, though target is to display as few dialogs, and then preferable
 * the NewTransferDialog, to the user as possible i.e. if not enough information -- e.g. no destination folder
 * determinable, ...-- is present for a url or a group of urls they won't be added as transfer,
 * instead the NewTransferDialog will be shown
 *
 * This also tries to add as many transfers as possible with one run, to ensure a high speed
 */
NewTransferDialogHandler::NewTransferDialogHandler(QObject *parent)
  : QObject(parent),
    m_nextJobId(0)
{
}

NewTransferDialogHandler::~NewTransferDialogHandler()
{
}


void NewTransferDialogHandler::showNewTransferDialog(const KUrl &url)
{
    showNewTransferDialog(url.isEmpty() ? KUrl::List() : KUrl::List() << url);
}

void NewTransferDialogHandler::showNewTransferDialog(KUrl::List urls)
{
    if (urls.isEmpty()) {
        newTransferDialogHandler->createDialog(urls, QString());
        return;
    }

    QHash<int, UrlData>::iterator itUrls = newTransferDialogHandler->m_urls.insert(newTransferDialogHandler->m_nextJobId, UrlData());
    QString folder;
    QString suggestedFileName;

    ///Only two urls defined, check if second one is a path or a file name
    if (urls.count() == 2) {
        const KUrl lastUrl = urls.last();
        QDir dir(lastUrl.toLocalFile());

        //check if last url is a file path, either absolute or relative
        if (lastUrl.isLocalFile()) {
            if (QDir::isAbsolutePath(lastUrl.toLocalFile())) {
                if (dir.exists()) {
                    //second url is a folder path
                    folder = lastUrl.path(KUrl::AddTrailingSlash);
                } else {
                    //second url is a file path, use this one
                    folder = lastUrl.directory(KUrl::AppendTrailingSlash);
                    suggestedFileName = lastUrl.fileName();
                }
                urls.removeLast();
            } else {
                //second url is just a file name
                suggestedFileName = lastUrl.fileName(KUrl::ObeyTrailingSlash);
                urls.removeLast();
            }
        } else if (!lastUrl.isValid() || (lastUrl.scheme().isEmpty() && lastUrl.directory().isEmpty())) {
            // Sometimes valid filenames are not recognised by KURL::isLocalFile(), they are marked as invalid then
            suggestedFileName = lastUrl.url();
            urls.removeLast();
        }
    }

    ///More than two urls defined, and last is local and will be used as destination directory
    if (urls.count() > 2 && urls.last().isLocalFile()) {
        
    /**
     * FIXME should the code be uncommented again, though then inputing a wrong destination like
     * ~/Downloads/folderNotExisting would result in ~/Downloads/ instead of informing the user
     * and giving them the possibility to improve their mistake
     */
//         if (!QFileInfo(urls.last().toLocalFile()).isDir()) {
//             folder = urls.last().directory(KUrl::AppendTrailingSlash);
//         } else {
            folder = urls.last().path(KUrl::AddTrailingSlash);//checks if that folder is correct happen later
//         }
        urls.removeLast();
    }

    //add a folder or suggestedFileName if they are valid
    if (!folder.isEmpty() && KGet::isValidDestDirectory(folder)) {
        (*itUrls).folder = folder;
    }
    if (!suggestedFileName.isEmpty()) {
        (*itUrls).suggestedFileName = KUrl(suggestedFileName).pathOrUrl();//pathOrUrl to get a non percent encoded url
    }

    newTransferDialogHandler->m_numJobs[newTransferDialogHandler->m_nextJobId] = urls.count();
    foreach (const KUrl &url, urls) {
        //needed to avoid when protocols like the desktop protocol is used, see bko:185283
        KIO::Job *job = mostLocalUrlJob(url);
        job->setProperty("jobId", (newTransferDialogHandler->m_nextJobId));
        connect(job, SIGNAL(result(KJob*)), newTransferDialogHandler, SLOT(slotMostLocalUrlResult(KJob*)));
        job->start();
    }

    ++(newTransferDialogHandler->m_nextJobId);
}

void NewTransferDialogHandler::slotMostLocalUrlResult(KJob *j)
{
    MostLocalUrlJob *job = static_cast<MostLocalUrlJob*>(j);
    const int jobId = job->property("jobId").toInt();

    if (job->error()) {
        kWarning(5001) << "An error happened for" << job->url();
    } else {
        m_urls[jobId].urls << job->mostLocalUrl();
    }
    --m_numJobs[jobId];

    if (m_numJobs[jobId] <= 0) {
        handleUrls(jobId);
    }
}

void NewTransferDialogHandler::handleUrls(const int jobId)
{
    QHash<int, UrlData>::iterator itUrls = m_urls.find(jobId);
    if (itUrls == m_urls.end()) {
        kWarning(5001) << "JobId" << jobId << "was not defined, could not handle urls for it.";
        return;
    }

    KUrl::List urls = (*itUrls).urls;
    UrlChecker::removeDuplicates(urls);

    QString folder = (*itUrls).folder;
    if (!folder.isEmpty() && (UrlChecker::checkFolder(KUrl(folder), true) != UrlChecker::NoError)) {
        folder.clear();
    }

    const QString suggestedFileName = (*itUrls).suggestedFileName;
    KUrl newDest;
    const KUrl folderUrl = KUrl(folder);

    //check if the sources are correct
    UrlChecker check(UrlChecker::Source);
    check.addUrls(urls);
    check.displayErrorMessages();
    check.existingTransfers();
    urls = check.correctUrls();

    QList<KGet::TransferData> data;

    ///Just one file to download, with a specified suggestedFileName, handle if possible
    if (!suggestedFileName.isEmpty() && (urls.count() == 1)) {
        const KUrl sourceUrl = urls.first();
        const QList<TransferGroupHandler*> groups = KGet::groupsFromExceptions(sourceUrl);
        const QString groupName = (groups.isEmpty() ? QString() : groups.first()->name());
        QString defaultFolder;
        if (groups.isEmpty()) {
            defaultFolder = (Settings::askForDestination() ? QString() : KGlobalSettings::downloadPath());
        } else {
            defaultFolder = groups.first()->defaultFolder();
        }

        if (!folder.isEmpty()) {
            const KUrl destUrl = UrlChecker::destUrl(KUrl(folder), sourceUrl, suggestedFileName);
            newDest = check.checkExistingFile(sourceUrl, destUrl);
            if (!newDest.isEmpty()) {
                data << KGet::TransferData(sourceUrl, newDest, groupName);
            }
            urls.removeFirst();
        } else if (((!groups.isEmpty() && !Settings::directoriesAsSuggestion()) || !Settings::askForDestination()) &&
                   (UrlChecker::checkFolder(KUrl(defaultFolder)) == UrlChecker::NoError)) {
            const KUrl destUrl = UrlChecker::destUrl(KUrl(defaultFolder), sourceUrl, suggestedFileName);
            newDest = check.checkExistingFile(sourceUrl, destUrl);
            if (!newDest.isEmpty()) {
                data << KGet::TransferData(sourceUrl, newDest, groupName);
            }
            urls.removeFirst();
        }
    }

    ///A valid folder has been defined, use that for downloading
    if (!folder.isEmpty()) {
        //find the associated groups first, we just need the first matching group though
        const QList<TransferGroupHandler*> groups = KGet::allTransferGroups();
        foreach (TransferGroupHandler *group, groups) {
            if (urls.isEmpty()) {
                break;
            }

            const QString groupName = group->name();
            const QStringList patterns = group->regExp().pattern().split(',');

            //find all urls where a group can be identified
            KUrl::List::iterator it = urls.begin();
            while (it != urls.end()) {
                const KUrl sourceUrl = *it;
                if (KGet::matchesExceptions(sourceUrl, patterns)) {
                    const KUrl destUrl = UrlChecker::destUrl(folderUrl, sourceUrl);
                    newDest = check.checkExistingFile(sourceUrl, destUrl);
                    if (!newDest.isEmpty()) {
                        data << KGet::TransferData(sourceUrl, newDest, groupName);
                    }
                    it = urls.erase(it);
                } else {
                    ++it;
                }
            }
        }

        //there are still some unhandled urls, i.e. for those no group could be found, add them with an empty group
        foreach (const KUrl &sourceUrl, urls) {
            const KUrl destUrl = UrlChecker::destUrl(folderUrl, sourceUrl);
            newDest = check.checkExistingFile(sourceUrl, destUrl);
            if (!newDest.isEmpty()) {
                data << KGet::TransferData(sourceUrl, newDest);
            }
        }

        //all urls have been handled
        urls.clear();
    }

    ///Now handle default folders/groups
    kDebug(5001) << "DIRECTORIES AS SUGGESTION" << Settings::directoriesAsSuggestion();
    if (!Settings::directoriesAsSuggestion() && !urls.isEmpty()) {
        kDebug(5001) << "No, Directories not as suggestion";

        //find the associated groups first, we just need the first matching group though
        const QList<TransferGroupHandler*> groups = KGet::allTransferGroups();
        foreach (TransferGroupHandler *group, groups) {
            if (urls.isEmpty()) {
                break;
            }

            const KUrl folderUrl = KUrl(group->defaultFolder());
            if (UrlChecker::checkFolder(folderUrl) != UrlChecker::NoError) {
                continue;
            }

            const QString groupName = group->name();
            const QStringList patterns = group->regExp().pattern().split(',');

            KUrl::List::iterator it = urls.begin();
            while (it != urls.end()) {
                const KUrl sourceUrl = *it;
                if (KGet::matchesExceptions(sourceUrl, patterns)) {
                    const KUrl destUrl = UrlChecker::destUrl(folderUrl, sourceUrl);
                    newDest = check.checkExistingFile(sourceUrl, destUrl);
                    if (!newDest.isEmpty()) {
                        data << KGet::TransferData(sourceUrl, newDest, groupName);
                    }

                    it = urls.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    ///Download the rest of the urls to KGlobalSettings::downloadPath() if the user is not aksed for a destination
    if (!Settings::askForDestination()) {
        //the download path will be always used
        const QString dir = KGlobalSettings::downloadPath();
        if (!dir.isEmpty()) {
            KUrl::List::iterator it = urls.begin();
            while (it != urls.end()) {
                const KUrl sourceUrl = *it;
                const KUrl destUrl = UrlChecker::destUrl(dir, sourceUrl);
                newDest = check.checkExistingFile(sourceUrl, destUrl);
                if (!newDest.isEmpty()) {
                    data << KGet::TransferData(sourceUrl, newDest);
                }

                it = urls.erase(it);
            }
        }
    }

    ///Now create transfers for the urls that provided enough data
    if (!data.isEmpty()) {
        KGet::createTransfers(data);
    }

    ///Handle custom newtransferdialogs...
    if ((!m_dialog || m_dialog->isEmpty()) && urls.count() == 1) {//FIXME why the m_dialog check? whenever a dialog has been created this would not be shown?
        KUrl url = urls.first();
        QPointer<KDialog> dialog;
        foreach (TransferFactory * factory, KGet::factories()) {
            const QList<TransferGroupHandler*> groups =  KGet::groupsFromExceptions(url);
            dialog = factory->createNewTransferDialog(url, suggestedFileName, !groups.isEmpty() ? groups.first() : 0);
            if (dialog) {
                KWindowInfo info = KWindowSystem::windowInfo(KGet::m_mainWindow->winId(), NET::WMDesktop, NET::WMDesktop);
                KWindowSystem::setCurrentDesktop(info.desktop());
                KWindowSystem::forceActiveWindow(KGet::m_mainWindow->winId());

                dialog->exec();
                delete dialog;
            }
        }
    }

    m_numJobs.remove(jobId);
    m_urls.erase(itUrls);

    ///Display default NewTransferDialog
    if (!urls.isEmpty()) {
        createDialog(urls, suggestedFileName);
    }
}

void NewTransferDialogHandler::createDialog(const KUrl::List &urls, const QString &suggestedFileName)
{
    if (!m_dialog) {
        m_dialog = new NewTransferDialog(KGet::m_mainWindow);
    }

    m_dialog->m_window = KGet::m_mainWindow;
    m_dialog->showDialog(urls, suggestedFileName);
}

#include "newtransferdialog.moc"
